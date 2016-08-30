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

int stringify_json_object(struct json_exception* ex, struct json_object* object, char* output){
	struct json_object* current = object;
	int keylen, valuelen, i = 0;

	output[i++] = '{';

	while(current != 0){
		output[i++] = '"';
		strncpy(output + i, current->key, current->keylen);
		i += current->keylen;
		output[i++] = '"';
		output[i++] = ':';
		output[i++] = '"';
		strncpy(output + i, current->value, current->valuelen);
		i += current->valuelen;
		output[i++] = '"';
	
		current = current->next;
		if(current != 0){
			output[i++] = ',';
		}
	}

	output[i++] = '}';
	output[i++] = 0;

	return 0;	
}

int parse_json_object(struct json_exception* ex, struct json_object* object, char* json){
	int i, str_index, json_len = strlen(json);
	struct json_object* current = object;
	enum json_state state = NONE;

	current->next = 0;

	for(i = 0; i < json_len; i++){
		printf("%c", json[i]);
		switch(json[i]){
			case 0:
				if(state == NONE){
					return 0;
				}else{
					strcpy(ex->message, "Incomplete JSON.");
					ex->location = i;
					return -1;
				}
			case '{':
				if(state == NONE){
					state = IN_OBJECT;
				}else if(state == IN_OBJECT ||
				state == GOT_KEY ||
				state == GOT_VALUE){
					strcpy(ex->message, "Invalid character A.");
					ex->location = i;
					return -1;
				}
				break;
			case '"':
				if(state == IN_OBJECT){
					state = GET_KEY;
					str_index = 0;
					break;
				}else if(state == GET_KEY){
					state = GOT_KEY;
					current->key[str_index] = 0;
					current->keylen = str_index;
					break;
				}else if(state == GOT_KEY){
					state = GET_VALUE;
					str_index = 0;
					break;
				}else if(state == GET_VALUE){
					state = GOT_VALUE;
					current->value[str_index] = 0;
					current->valuelen = str_index;
					break;
				}else if(state == GOT_VALUE){
					strcpy(ex->message, "Invalid character B.");
					ex->location = i;
					return -1;
				}else{
					strcpy(ex->message, "What the heck happened?");
					ex->location = i;
					return -1;
				}
			case ':':
				if(state == IN_OBJECT ||
				state == GOT_VALUE){
					strcpy(ex->message, "Invalid character C.");
					ex->location = i;
					return -1;
				}
				break;
			case ',':
				if(state == GOT_VALUE){
					current->next = (struct json_object*)malloc(sizeof(struct json_object));
					current = current->next;
					current->next = 0;
					state = IN_OBJECT;
				}
				break;
			case '}':
				if(state != GOT_VALUE){
					strcpy(ex->message, "Invalid character D.");
					ex->location = i;
					return -1;
				}
				break;
			case ' ':
				break;
			default:
				if(state != GET_KEY &&
				state != GET_VALUE){
					strcpy(ex->message, "Invalid character E.");
					ex->location = i;
					return -1;
				}
		}
		if(state == GET_KEY){
			current->key[str_index] = json[i];
			str_index++;
		}else if(state == GET_VALUE){
			current->value[str_index] = json[i];
			str_index++;
		}
	}
}

int main(){
	struct json_exception ex;
	struct json_object object;

	char* myjson = " { \" apples\" : \"+     5\" , \"   oranges\" : \"+4\"      }      ";
	char result_json[256];

	if(parse_json_object(&ex, &object, myjson) == -1){
		printf("\nError: %s on char %d(%c).\n", ex.message, ex.location, myjson[ex.location]);
	}else{
		printf("\nJSON successfully parsed.\n");
	}

	stringify_json_object(&ex, &object, result_json);
	printf("Stringified JSON object:\n%s\n", result_json);

	return 0;
}
