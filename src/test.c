#ifndef EFF2JSON_TEST
#error Not in testing mode
#endif

#include <stdio.h>
#include "test.h"

#define test(function) \
    printf("Running test_%s", #function); \
    if(test_ ## function()) { \
        printf("\tFAILED"); \
        failed++; \
    } \
    printf("\n");

int main() {
    unsigned int failed = 0;

    test(json_val_new_integer)
    test(json_val_new_string)
    test(json_val_new_array)
    test(json_val_new_object)
    test(json_val_new_false)
    test(json_val_new_true)
    test(json_val_new_null)
    test(json_val_equal)
    test(json_val_print)

    test(json_obj_new)
    test(json_obj_equal)
    test(json_obj_set)
    test(json_obj_haskey)
    test(json_obj_remove)
    test(json_obj_print)

    test(json_arr_new)
    test(json_arr_equal)
    test(json_arr_append)
    test(json_arr_has)
    test(json_arr_remove)
    test(json_arr_print)

    return failed;
}
