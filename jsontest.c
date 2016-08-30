#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define STRMAX 2048

#include "json.h"

void test(char* json){
	struct json_exception ex;
	struct json_object object;
	char* route;
	char result[256];
	printf("\nTesting: %s\n", json);

	if(parse_json_object(&ex, &object, json, strlen(json)) == -1){
		printf("Error: %s on char %d(%c).\n", ex.message, ex.location, json[ex.location]);
		return;
	}else{
		printf("JSON successfully parsed.\n");
	}

	if((route = get_value(&object, "route")) == 0){
		printf("No 'route' key/value pair.\n");
	}else{
		printf("'route':'%s'\n", route);
	}

	stringify_json_object(&object, result);
	printf("Stringified JSON object: %s\n", result);
}

int main(){
	test(" { \" {}:,apples\" : \"+     5\" , \"   oranges\" : \"+4\"  ,\"route\":\"\\index\\winner.txt\"    }      ");
	test("{\"token\":\"sgs5fgh56");
	test("{\"token\":\"sgs5fgh56\",\"username\":\"'bwackwat','blogdata':'my post data yay'\"}");
	test("{\"route\":\"/login\",\"username\":\"'bwackwat','blogdata':'my post data yay'\"}");
	test("{\"one\":\"one\",\"two\":\"two\",\"three\":\"three\",\"four\":\"four\",\"five\":\"five\"}");

	printf("\nDone!\n");

	return 0;
}
