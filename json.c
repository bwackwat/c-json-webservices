#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define STRMAX 2048

#include "json.h"

int stringify_json_object(struct json_object* object, char* output){
	struct json_object* current = object;
	int i = 0;

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

int parse_json_object(struct json_exception* ex, struct json_object* object, char* json, int jsonlen){
	int i, str_index;
	struct json_object* current = object;
	enum json_state state = NONE;

	current->next = 0;

	for(i = 0; i < jsonlen; i++){
		switch(json[i]){
			case 0:
				if(state == NONE){
					return 0;
				}else{
					strcpy(ex->message, "Incomplete JSON.");
					ex->location = i - 1;
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
					continue;
				}else if(state == GET_KEY){
					state = GOT_KEY;
					current->key[str_index] = 0;
					current->keylen = str_index;
					break;
				}else if(state == GOT_KEY){
					state = GET_VALUE;
					str_index = 0;
					continue;
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
				if(state == GET_KEY ||
				state == GET_VALUE){
					break;
				}else if(state != GOT_VALUE){
					strcpy(ex->message, "Invalid character D.");
					ex->location = i;
					return -1;
				}else{
					state = NONE;
					break;
				}
			case ' ':
				break;
			default:
				if(state != GET_KEY &&
				state != GET_VALUE){
					sprintf(ex->message, "Invalid character E. %c", json[i]);
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
	str_index = jsonlen - 1;
	while(json[str_index] == ' '){
		str_index--;
	}
	if(json[str_index] != '}'){
		strcpy(ex->message, "Objects must end with '}'.");
		ex->location = str_index;
		return -1;
	}

	return 0;
}

char* get_value(struct json_object* object, char* key){
	struct json_object* current = object;
	while(current != 0){
		if(strcmp(current->key, key) == 0){
			return current->value;
		}
		current = current->next;
	}
	return 0;
}
