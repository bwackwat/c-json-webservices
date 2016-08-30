#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define STRMAX 2048

struct json_exception {
	int location;
	char message[256];
};

struct json_object {
	char key[256];
	int keylen;
	char value[256];
	int valuelen;
	struct json_object* next;
};

enum json_state {
	NONE,
	IN_OBJECT,
	GET_KEY,
	GOT_KEY,
	GET_VALUE,
	GOT_VALUE
};

int stringify_json_object(struct json_object* object, char* output);
int parse_json_object(struct json_exception* ex, struct json_object* object, char* json, int jsonlen);
char* get_value(struct json_object* object, char* key);
