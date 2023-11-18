// Declaration of unit test functions.
// They are defined in the compilation unit that contains the unit under test.
//
// Unit test functions must return 0 for success.

int test_json_val_new_integer();
int test_json_val_new_string();
int test_json_val_new_array();
int test_json_val_new_object();
int test_json_val_new_false();
int test_json_val_new_true();
int test_json_val_new_null();
int test_json_val_equal();
int test_json_val_print();

int test_json_obj_new();
int test_json_obj_equal();
int test_json_obj_set();
int test_json_obj_haskey();
int test_json_obj_remove();
int test_json_obj_print();

int test_json_arr_new();
int test_json_arr_equal();
int test_json_arr_append();
int test_json_arr_has();
int test_json_arr_remove();
int test_json_arr_print();
