// This preprocessor directive prevents this file from defining the main
// function when we're in testing mode. The alternative is to leave out this
// file from compilation when testing. That's how Makefile does it right now:
// $(filter-out src/main.c, $(wildcard src/*))
#ifndef EFF2JSON_TEST


#include <stdio.h>
#include "json.h"

struct json_obj* eff2json(const char* filename) {
    struct json_obj* jo_file = json_obj_new();

    // detect file format
    // TODO

    return jo_file;
}

int main(int argc, char* argv[]) {

    // check arguments
    if(argc<=1) {
        fprintf(stderr, "Expected one argument\n");
        return 1;
    }

    // make top JSON object
    struct json_obj* jo_top = json_obj_new();

    // make object with files
    struct json_obj* jo_files = json_obj_new();
    for(int i=1;i<argc;i++) {
        char* filename = argv[i];
        struct json_obj* jo_file = eff2json(filename);
        struct json_val* jv_file = json_val_new_object(jo_file);
        json_obj_set(jo_files, filename, jv_file);
    }

    // add object with files to top JSON object
    struct json_val* jv_files = json_val_new_object(jo_files);
    json_obj_set(jo_top, "files", jv_files);

    // print top JSON object
    json_obj_print(jo_top, stdout);
    printf("\n");
}


#endif
