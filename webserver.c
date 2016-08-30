#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/sendfile.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define BUFSIZE 8096
#define ERROR      42
#define LOG        44
#define FORBIDDEN 403
#define NOTFOUND  404

#ifndef SIGCLD
#   define SIGCLD SIGCHLD
#endif

#include "json.h"
#include "util.h"

struct {
	char* ext;
	char* filetype;
} extensions [] = {
	{"gif", "image/gif" },
	{"jpg", "image/jpg" },
	{"jpeg", "image/jpeg"},
	{"png", "image/png" },
	{"ico", "image/ico" },
	{"zip", "image/zip" },
	{"gz",  "image/gz"  },
	{"tar", "image/tar" },
	{"htm", "text/html" },
	{"html", "text/html" },
	{"js", "text/javascript" },
	{"css","text/css" },
	{0,0}
};

void slog(char* msg){
	int fd = open("../log.txt", O_CREAT | O_WRONLY | O_APPEND, 0644);
	write(fd, msg, strlen(msg));
	close(fd);
}

/*
void log_print(const char* msg, ...){
	va_list va;
	FILE* fd;
	va_start(va, msg);
	if((fd = fopen("../log.txt", "a")) != 0){
		
		vfprintf(fd, msg, va);
		fclose(fd);
	}
	va_end(va);
}
*/

void logger(int type, char *s1, char *s2, int socket_fd){
  int fd ;
  char logbuffer[BUFSIZE*2];

  switch (type) {
  case ERROR: (void)sprintf(logbuffer,"ERROR: %s:%s Errno=%d exiting pid=%d",s1, s2, errno,getpid());
    break;
  case FORBIDDEN:
    (void)write(socket_fd, "HTTP/1.1 403 Forbidden\nContent-Length: 185\nConnection: close\nContent-Type: text/html\n\n<html><head>\n<title>403 Forbidden</title>\n</head><body>\n<h1>Forbidden</h1>\nThe requested URL, file type or operation is not allowed on this simple static file webserver.\n</body></html>\n",271);
    (void)sprintf(logbuffer,"FORBIDDEN: %s:%s",s1, s2);
    break;
  case NOTFOUND:
    (void)write(socket_fd, "HTTP/1.1 404 Not Found\nContent-Length: 136\nConnection: close\nContent-Type: text/html\n\n<html><head>\n<title>404 Not Found</title>\n</head><body>\n<h1>Not Found</h1>\nThe requested URL was not found on this server.\n</body></html>\n",224);
    (void)sprintf(logbuffer,"NOT FOUND: %s:%s",s1, s2);
    break;
  case LOG: (void)sprintf(logbuffer," INFO: %s:%s:%d",s1, s2,socket_fd); break;
  }
  /* No checks here, nothing can be done with a failure anyway */
  if((fd = open("../log.txt", O_CREAT| O_WRONLY | O_APPEND,0644)) >= 0) {
    (void)write(fd,logbuffer,strlen(logbuffer));
    (void)write(fd,"\n",1);
    (void)close(fd);
  }
	printf(s1);
	printf(s2);
  if(type == ERROR || type == NOTFOUND || type == FORBIDDEN) exit(3);
}

int handle_http_get(int fd, char* buffer, char* response,  int bufferlen){
	static char route[256];
	long len;
	int i, mime = -1, file_fd;

	for(i = 5; i < bufferlen; i++){
		if(buffer[i] == '.' && buffer[i + 1] == '.'){
			logger(FORBIDDEN, "error", "cannot use parent dir", fd);
		}
		if(buffer[i] == ' '){
			route[i - 5] = 0;
			file_fd = strlen(route);
			break;
		}else{
			route[i - 5] = buffer[i];
		}
	}

	for(i = 0; extensions[i].ext != 0; i++){
		len = strlen(extensions[i].ext);
		if(!strncmp(&route[file_fd - len], extensions[i].ext, len)){
			mime = i;
			break;
		}
	}
	if(mime == -1){
		logger(FORBIDDEN, "file extension not supported", "", fd);
		return -1;
	}
	if((file_fd = open(route, O_RDONLY)) == -1){
		logger(NOTFOUND, "failed to open file",&buffer[5],fd);
		return -1;
	}
	logger(LOG, "SEND", route, fd);
	len = (long)lseek(file_fd, (off_t)0, SEEK_END);
	(void)lseek(file_fd, (off_t)0, SEEK_SET);
	(void)sprintf(response,"HTTP/1.1 200 OK\nServer: bwackwat/23.0\nContent-Length: %ld\nConnection: keep-alive\nContent-Type: %s\n\n", len, extensions[i].filetype);
	logger(LOG, "Header", response, fd);
	(void)write(fd, response, strlen(response));
		/* SENDFILE OR READ/WRITE IMPLEMENTAITONS */
	if(sendfile(fd, file_fd, 0, len) == -1){
		logger(ERROR, "Error", "sendfile", 0);
		return -1;
	}
	
	/*
	while((ret = read(file_fd, buffer, BUFSIZE)) > 0){
		(void)write(fd, buffer, ret);
	}
	*/
	return 0;
}

int handle_raw_json(int fd, char* request, char* response, int requestlen){
	struct json_exception ex;
	struct json_object object;
	char* route;

        if(parse_json_object(&ex, &object, request, requestlen) == -1){
		sprintf(response, "{\"error\":\"%s at location %d(%c)\"}", ex.message, ex.location, request[ex.location]);
        }else if((route = get_value(&object, "route")) == 0){
		sprintf(response, "{\"error\":\"You must provide a 'route' key/value pair.\"}");
	}else{
		sprintf(response, "{\"result\":\"You hit the route %s!\"}", route);
	}

	if(write(fd, response, strlen(response)) == -1){
		log_print("Write error!\n");
		return -1;
	}
	return 0;
}

void handle_connection(int fd, int hit){
	static char request[BUFSIZE + 1];
	static char response[BUFSIZE + 1];
	int len, transaction;
	
	for(transaction = 1;; transaction++){
		if((len = read(fd, request, BUFSIZE)) <= 0){
			log_print("Connection #%d, done!\n", hit);
			return;
		}
		request[len] = 0;

		log_print("Connection #%d transaction #%d -> %s\n", hit, transaction, request);

		if(strncmp(request, "GET ", 4) == 0){
			if(handle_http_get(fd, request, response, len) == -1){
				return;
			}
		}else{
			if(handle_raw_json(fd, request, response, len) == -1){
				return;
			}
		}

		log_print("Connection #%d transaction #%d <- %s\n", hit, transaction, response);
	}
}

int main(int argc, char **argv){
	int i, port, pid, listenfd, socketfd, hit;
	socklen_t length;
	static struct sockaddr_in cli_addr;
	static struct sockaddr_in serv_addr;

	if(argc != 3){
		printf("usage: %s <port> <directory>\n", argv[0]);
		return 0;
	}

	port = atoi(argv[1]);
	if(port < 0 || port > 60000){
		printf("Invalid port.\n");
		return 1;
	}

	if(chdir(argv[2]) == -1){
		printf("Couldn't change directory (chdir) to %s!\n", argv[2]);
		return 1;
	}

	/* Become deamon + unstopable and no zombies children (= no wait()) */
	if(fork() != 0)
		return 0;

	signal(SIGCLD, SIG_IGN); /* ignore child death */
	signal(SIGHUP, SIG_IGN); /* ignore terminal hangups */

	for(i = 0; i < 32; i++)
		close(i); /* Closes open files...? */

	setpgid(0, 0); /* break away from process group */
	
	if((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
		log_print("System call to socket failed!\n");
		return 1;
	}

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port = htons(port);

	if(bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0){
		log_print("System call to bind failed!\n");
		return 1;
	}

	if(listen(listenfd, 64) < 0){
		log_print("System call to listen failed!\n");
		return 1;
	}

	for(hit = 1;; hit++){
		length = sizeof(cli_addr);

		if((socketfd = accept(listenfd, (struct sockaddr*)&cli_addr, &length)) < 0){
			log_print("System call to accept failed!\n");
			return 1;
		}

		if((pid = fork()) < 0){
			log_print("System call to fork failed!\n");
			return 1;
		}else{
			if(pid == 0){
				close(listenfd);
				log_print("New connection #%d\n", hit);
				handle_connection(socketfd, hit);
				close(socketfd);
				break;
			}else{
				close(socketfd);
			}
		}
	}
	return 0;
}
