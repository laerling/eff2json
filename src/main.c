// This preprocessor directive prevents this file from defining the main
// function when we're in testing mode. The alternative is to leave out this
// file from compilation when testing. That's how Makefile does it right now:
// $(filter-out src/main.c, $(wildcard src/*))
#ifndef EFF2JSON_TEST


#include <stdio.h>
#include "json.h"

struct json_obj* json_demo() {
    struct json_val* vs = json_val_new_string("bar");
    struct json_val* vi = json_val_new_integer(42);
    struct json_val* vn = json_val_new_null();
    struct json_val* vf = json_val_new_false();
    struct json_val* vt = json_val_new_true();
    struct json_obj* voo = json_obj_new();
    json_obj_set(voo, "foo", json_val_new_null());
    struct json_val* vo = json_val_new_object(voo);
    struct json_arr* vll = json_arr_new();
    json_arr_append(vll, json_val_new_null());
    struct json_val* vl = json_val_new_array(vll);
    struct json_obj* o = json_obj_new();
    json_obj_set(o, "string", vs);
    json_obj_set(o, "integer", vi);
    json_obj_set(o, "null", vn);
    json_obj_set(o, "false", vf);
    json_obj_set(o, "true", vt);
    json_obj_set(o, "object", vo);
    json_obj_set(o, "array", vl);

    json_obj_print(o, stdout);
    printf("\n");
    return o;
}

char* eff2json(const char* filename) {

    // detect file format
    // TODO

    // TEMP: JSON demo
    struct json_obj* o = json_demo();
    json_obj_print(o, stdout);
    json_obj_free(o);
    printf("\n");
}

int main(int argc, char* argv[]) {

    // check arguments
    if(argc<=1) {
        fprintf(stderr, "Expected one argument\n");
        return 1;
    }

    // make global JSON object
    struct json_obj* top_json = json_obj_new();

    // check every file
    for(int i=1;i<argc;i++) {
        char* file_json = eff2json(argv[i]);
        // TODO add to global JSON object
    }

    // print global JSON object
    // TODO

    return 0;
}


#endif
