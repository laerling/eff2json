#ifndef EFF2JSON_JSON_H
#define EFF2JSON_JSON_H


#include <stdio.h>


struct json_val;

struct json_obj;

struct json_arr;


struct json_val* json_val_new_integer(int v);
struct json_val* json_val_new_string(char* v);
struct json_val* json_val_new_array(struct json_arr* v);
struct json_val* json_val_new_object(struct json_obj* v);
struct json_val* json_val_new_false();
struct json_val* json_val_new_true();
struct json_val* json_val_new_null();

int json_val_equal(struct json_val* a, struct json_val* b);

void json_val_print(struct json_val* v, FILE* f);

void json_val_free(struct json_val* v);


struct json_obj* json_obj_new();

int json_obj_equal(struct json_obj* a, struct json_obj* b);

void json_obj_set(struct json_obj* o, char* key, struct json_val* v);

int json_obj_haskey(struct json_obj* o, char* key);

void json_obj_remove(struct json_obj* o, char* key);

void json_obj_print(struct json_obj* o, FILE* f);

void json_obj_free(struct json_obj* o);


struct json_arr* json_arr_new();

int json_arr_equal(struct json_arr* a, struct json_arr* b);

void json_arr_append(struct json_arr* a, struct json_val* v);

int json_arr_has(struct json_arr* a, struct json_val* v);

void json_arr_remove(struct json_arr* a, struct json_val* v);

void json_arr_print(struct json_arr* a, FILE* f);

void json_arr_free(struct json_arr* a);


#endif
