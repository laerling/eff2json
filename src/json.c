#define _POSIX_C_SOURCE 1
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdio_ext.h>
#include <string.h>
#include <unistd.h>


// values

// forward declaration needed for json_val
struct json_obj;
struct json_arr;

struct json_val {
    enum type { Integer, String, Array, Object, False, True, Null } type;
    union val {
        int i;
        char* s;
        struct json_arr* a;
        struct json_obj* o;
    } val;
};

#define json_val_new(functionname, typekeyword, typename, typeshorthand) \
    struct json_val* functionname(typekeyword v) { \
        struct json_val* jv = malloc(sizeof(struct json_val)); \
        jv->type = typename; \
        jv->val.typeshorthand = v; \
        return jv; \
    }

#define json_val_new_noarg(functionname, typename) \
    struct json_val* functionname() { \
        struct json_val* jv = malloc(sizeof(struct json_val)); \
        jv->type = typename; \
        return jv; \
    }

json_val_new(json_val_new_integer, int, Integer, i)
json_val_new(json_val_new_string, char*, String, s)
json_val_new(json_val_new_array, struct json_arr*, Array, a)
json_val_new(json_val_new_object, struct json_obj*, Object, o)
json_val_new_noarg(json_val_new_false, False)
json_val_new_noarg(json_val_new_true, True)
json_val_new_noarg(json_val_new_null, Null)


// object

struct json_obj_kv {
    char* key;
    struct json_val* val;
    struct json_obj_kv* next;
};

struct json_obj {
    struct json_obj_kv* first;
};

struct json_obj* json_obj_new() {
    struct json_obj* o = malloc(sizeof(struct json_obj));
    o->first = NULL;
    return o;
}

void json_obj_set(struct json_obj* o, char* key, struct json_val* v) {

    // seek and update
    struct json_obj_kv* last = NULL;
    for(struct json_obj_kv* e = o->first; e != NULL; e = e->next) {

        // found the key, so just update the value and we're done
        if(strcmp(e->key, key) == 0) {
            e->val = v;
            return;
        }

        last = e;
    }

    // no update happened, so we need to append the new key-value pair
    struct json_obj_kv* new = malloc(sizeof(struct json_obj_kv));
    new->key = key;
    new->val = v;
    new->next = NULL; // this is the new end
    // link to last key-value pair (might be the object root)
    if(last == NULL) {
        o->first = new;
    } else {
        last->next = new;
    }
}

int json_obj_haskey(struct json_obj* o, char* key) {
    for(struct json_obj_kv* e = o->first; e != NULL; e = e->next) {
        if(strcmp(e->key, key) == 0)
            return 1;
    }
    return 0;
}

// forward declaration for json_obj_remove
void json_obj_kv_free(struct json_obj_kv* e);

void json_obj_remove(struct json_obj* o, char* key) {
    struct json_obj_kv* last = NULL;
    for(struct json_obj_kv* e = o->first; e != NULL; e = e->next) {
        if(strcmp(e->key, key) == 0) {
            // relink (e->next might be NULL and that's fine)
            if(last == NULL) {
                // this is the first key-value pair in the linked list
                o->first = e->next;
            } else {
                last->next = e->next;
            }
            // json_obj_kv_free frees e->next, but since we linked e->next to
            // last->next, we don't want it to be freed. So set e->next to
            // NULL, which is as if this were the last element of the list.
            e->next = NULL;
            json_obj_kv_free(e);
            return;
        }
        last = e;
    }
}


// array

struct json_arr_elem {
    struct json_val* val;
    struct json_arr_elem* next;
};

struct json_arr {
    struct json_arr_elem* first;
};

struct json_arr* json_arr_new() {
    struct json_arr* a = malloc(sizeof(struct json_arr));
    a->first = NULL;
    return a;
}

void json_arr_append(struct json_arr* a, struct json_val* v) {
    struct json_arr_elem* new = malloc(sizeof(struct json_arr_elem));
    new->val = v;
    new->next = NULL;

    // find place to link
    struct json_arr_elem* e = a->first;
    if(e != NULL) {
        while(e->next != NULL)
            e = e->next;
    }

    // link
    if(e == NULL) {
        a->first = new;
    } else {
        e->next = new;
    }
}

// forward declaration for json_arr_has
int json_val_equal(struct json_val* a, struct json_val* b);

int json_arr_has(struct json_arr* a, struct json_val* v) {
    for(struct json_arr_elem* e = a->first; e != NULL; e = e->next) {
        if(json_val_equal(e->val, v))
            return 1;
    }
    return 0;
}

unsigned int json_arr_length(struct json_arr* a) {
    unsigned int length = 0;
    for(struct json_arr_elem* e=a->first;e!=NULL;e=e->next)
        length++;
    return length;
}

// forward declaration for json_arr_remove
void json_arr_elem_free(struct json_arr_elem* e);

void json_arr_remove(struct json_arr* a, struct json_val* v) {
    struct json_arr_elem* last = NULL;
    for(struct json_arr_elem* e = a->first; e != NULL; e = e->next) {
        if(json_val_equal(e->val, v)) {
            // relink (e->next might be NULL and that's fine)
            if(last == NULL) {
                a->first = e->next;
            } else {
                last->next = e->next;
            }
            json_arr_elem_free(e);
            return;
        }
        last = e;
    }
}


// freeing functions

// forward declaration for json_val_free
void json_obj_free(struct json_obj* o);
void json_arr_free(struct json_arr* a);

void json_val_free(struct json_val* v) {
    // Array and object are the only types that need to be recursively free'd.
    // The respective pointer is set to NULL to prevent access to free'd memory.
    switch(v->type) {
        case(Array):
            json_arr_free(v->val.a);
            v->val.a = NULL;
            break;
        case(Object):
            json_obj_free(v->val.o);
            v->val.o = NULL;
            break;
        case(String):
            // We consider strings (char*) to be owned and free'd by the
            // caller. After all, we don't know whether they're on the stack or
            // the heap or static. However, we still set the pointer to NULL.
            v->val.s = NULL;
            break;
        default:
            break;
    }
    free(v);
}

void json_obj_free(struct json_obj* o) {
    if(o->first != NULL)
        json_obj_kv_free(o->first);
    free(o);
}

void json_obj_kv_free(struct json_obj_kv* e) {
    // Keys are managed by the caller and thus don't have to be freed.
    if(e->next != NULL)
        json_obj_kv_free(e->next);
    json_val_free(e->val);
    free(e);
}

void json_arr_free(struct json_arr* a) {
    if(a->first != NULL)
        json_arr_elem_free(a->first);
    free(a);
}

void json_arr_elem_free(struct json_arr_elem* e) {
    if(e->next != NULL)
        json_arr_elem_free(e->next);
    json_val_free(e->val);
    free(e);
}


// comparison functions

// forward declarations for json_val_equal
int json_obj_equal(struct json_obj* a, struct json_obj* b);
int json_arr_equal(struct json_arr* a, struct json_arr* b);

int json_val_equal(struct json_val* a, struct json_val* b) {
    if(a == b)
        return 1;
    if((a == NULL && b != NULL) || (a != NULL && b == NULL))
        return 0;
    if(a->type != b->type)
        return 0;
    // At this point we already know that the type is the same.
    // For special types that already suffices.
    if(a->type == False || a->type == True || a->type == Null)
        return 1;
    if(a->type == Integer)
        return a->val.i == b->val.i;
    if(a->type == String)
        return strcmp(a->val.s, b->val.s) == 0;
    if(a->type == Array)
        return json_arr_equal(a->val.a, b->val.a);
    if(a->type == Object)
        return json_obj_equal(a->val.o, b->val.o);
    // other cases are unknown
    return 0;
}

int json_obj_equal(struct json_obj* a, struct json_obj* b) {
    if(a == b)
        return 1;
    // Key-value pairs might be in different order, so we have to compare each
    // one from a with each one from b.
    for(struct json_obj_kv* ae = a->first; ae != NULL; ae = ae->next) {
        // look for matching key-value pair in b
        int found = 0;
        for(struct json_obj_kv* be = b->first; be != NULL; be = be->next) {
            if(strcmp(ae->key, be->key) == 0 && json_val_equal(ae->val, be->val)) {
                found = 1;
                break;
            }
        }
        if(!found)
            return 0;
    }
    return 1;
}

int json_arr_equal(struct json_arr* a, struct json_arr* b) {
    struct json_arr_elem* be = b->first;
    struct json_arr_elem* ae = a->first;
    while(ae != NULL && be != NULL) {
        if(!json_val_equal(ae->val, be->val))
            return 0;
        ae = ae->next;
        be = be->next;
    }

    // do both arrays end in the same place?
    if(ae != NULL || be != NULL)
        return 0;

    // if we didn't return yet the arrays are equal
    return 1;
}


// printing functions

// forward declarations for json_val_printf
void json_obj_print(struct json_obj* o, FILE* f);
void json_arr_print(struct json_arr* a, FILE* f);

void json_val_print(struct json_val* v, FILE* f) {
    switch(v->type) {
        case Integer:
            fprintf(f, "%d", v->val.i);
            break;
        case String:
            // TODO handle escaping and other problems (see RFC)
            fprintf(f, "\"%s\"", v->val.s);
            break;
        case Array:
            json_arr_print(v->val.a, f);
            break;
        case Object:
            json_obj_print(v->val.o, f);
            break;
        case False:
            fprintf(f, "false");
            break;
        case True:
            fprintf(f, "true");
            break;
        case Null:
            fprintf(f, "null");
            break;
    }
}

void json_obj_print(struct json_obj* o, FILE* f) {
    fprintf(f, "{");
    int first_elem = 1;
    struct json_obj_kv* e = o->first;
    while(e != NULL) {
        if(first_elem) {
            first_elem = 0;
        } else {
            fprintf(f, ",");
        }
        // TODO handle escaping and other problems (see RFC)
        fprintf(f, "\"%s\":", e->key);
        json_val_print(e->val, f);
        e = e->next;
    }
    fprintf(f, "}");
}

void json_arr_print(struct json_arr* a, FILE* f) {
    fprintf(f, "[");
    int first_elem = 1;
    struct json_arr_elem* e = a->first;
    while(e != NULL) {
        if(first_elem) {
            first_elem = 0;
        } else {
            fprintf(f, ",");
        }
        json_val_print(e->val, f);
        e = e->next;
    }
    fprintf(f, "]");
}


// unit tests

#define fail(msg) { \
    fprintf(stderr, "Error: "); \
    fprintf(stderr, msg); \
    fprintf(stderr, "\n"); \
    return 1; \
}

int test_json_val_new_integer() {
    int x = 53;

    struct json_val* v = json_val_new_integer(x);

    // check type
    if(v->type != Integer)
        fail("Value type is not Integer")

    // check that dereferenced val is the one we put in
    if(v->val.i != x)
        fail("Value content is wrong")

    return 0;
}

int test_json_val_new_string() {
    char* x = "A funny string";

    struct json_val* v = json_val_new_string(x);

    // check type
    if(v->type != String)
        fail("Value type is not String")

    // check that the value is the one we put in
    if(v->val.s != x)
        fail("Value content pointer is wrong")

    return 0;
}

int test_json_val_new_array() {
    struct json_arr* x = json_arr_new();

    struct json_val* v = json_val_new_array(x);

    // check type
    if(v->type != Array)
        fail("Value type is not Array")

    // check that the value is the one we put in
    if(v->val.a != x)
        fail("Value content is wrong")

    return 0;
}

int test_json_val_new_object() {
    struct json_obj* x = json_obj_new();

    struct json_val* v = json_val_new_object(x);

    // check type
    if(v->type != Object)
        fail("Value type is not Object")

    // check that the value is the one we put in
    if(v->val.o != x)
        fail("Value content is wrong")

    return 0;
}

int test_json_val_new_false() {
    struct json_val* v = json_val_new_false();

    // check type
    if(v->type != False)
        fail("Value type is not False")

    return 0;
}

int test_json_val_new_true() {
    struct json_val* v = json_val_new_true();

    // check type
    if(v->type != True)
        fail("Value type is not True")

    return 0;
}

int test_json_val_new_null() {
    struct json_val* v = json_val_new_null();

    // check type
    if(v->type != Null)
        fail("Value type is not Null")

    return 0;
}

int test_json_val_equal() {
    // json_val_equal must compare contents, not pointers, so make two values
    // with the same content but at different memory locations.
    char s1[6] = "hello";
    char s2[6] = "hello";

    // check that we really have two different memory locations
    if(s1 == s2)
        fail("Trying to create two strings with same content but in different memory locations failed")

    struct json_val* v1 = json_val_new_string(s1);
    struct json_val* v2 = json_val_new_string(s2);

    // check that we have two different value structs
    if(v1 == v2)
        fail("Two value structs created after one another point to same memory");

    // check that we have two different pointers to the strings
    if(v1->val.s == v2->val.s)
        fail("Two strings in different memory locations, but same pointer in two different value structs");

    // check that the two value structs count as equal
    if(!(json_val_equal(v1, v2)))
        fail("Two value structs with same content are not equal");

    // check unequality
    s2[0]++;
    if(json_val_equal(v1, v2))
        fail("Two value structs with different content are equal");

    return 0;
}

int test_json_val_print() {

    // make pipe
    int pipe_fd[2];
    if(pipe(pipe_fd) == -1)
        fail("Can't create pipe from integer pair");
    int fd_r = pipe_fd[0];
    int fd_w = pipe_fd[1];

    // make streams from pipe FDs
    FILE* stream_r = fdopen(fd_r, "r");
    if(stream_r == NULL)
        fail("Cannot make stream from pipe read FD");
    FILE* stream_w = fdopen(fd_w, "w");
    if(stream_w == NULL)
        fail("Cannot make stream from pipe write FD");

    // integer

    // make and print JSON integer value
    int integer = 53;
    char expected_integer[] = "53";
    struct json_val* vi = json_val_new_integer(integer);
    json_val_print(vi, stream_w);
    fflush(stream_w);

    // read and compare
    int integer_len = strlen(expected_integer);
    char actual_integer[integer_len];
    fread(actual_integer, 1, integer_len, stream_r);
    if(strncmp(expected_integer, actual_integer, strlen(expected_integer)))
        fail("Got unexpected output from json_val_print for integer type");

    // discard streams for next test
    __fpurge(stream_w);
    __fpurge(stream_r);

    // string

    // make and print JSON string value
    char string[] = "foo";
    struct json_val* vs = json_val_new_string(string);
    json_val_print(vs, stream_w);
    fflush(stream_w);

    // read and compare
    char expected_string[] = "\"foo\"";
    int string_len = strlen(expected_string);
    char actual_string[string_len];
    fread(actual_string, 1, string_len, stream_r);
    if(strncmp(expected_string, actual_string, strlen(expected_string)))
        fail("Got unexpected output from json_val_print for string type");

    // discard streams for next test
    __fpurge(stream_w);
    __fpurge(stream_r);

    // test printing of empty array/object to check that json_print_val calls
    // json_print_arr/json_print_obj

    // array

    // make and print empty JSON array value
    // non-empty JSON arrays are tested in test_json_arr_print
    struct json_arr* a = json_arr_new();
    struct json_val* va = json_val_new_array(a);
    json_val_print(va, stream_w);
    fflush(stream_w);

    // read and compare
    char expected_arr[] = "[]";
    int array_len = strlen(expected_arr);
    char actual_array[array_len];
    fread(actual_array, 1, array_len, stream_r);
    if(strncmp(expected_arr, actual_array, strlen(expected_arr)))
        fail("Got unexpected output from json_val_print for array type");

    // object

    // make and print empty JSON object value
    // non-empty JSON object are tested in test_json_obj_print
    struct json_obj* o = json_obj_new();
    struct json_val* vo = json_val_new_object(o);
    json_val_print(vo, stream_w);
    fflush(stream_w);

    // read and compare
    char expected_obj[] = "{}";
    int object_len = strlen(expected_obj);
    char actual_object[object_len];
    fread(actual_object, 1, object_len, stream_r);
    if(strncmp(expected_obj, actual_object, strlen(expected_obj)))
        fail("Got unexpected output from json_val_print for object type");

    // false

    // make and print JSON false value
    struct json_val* vf = json_val_new_false();
    json_val_print(vf, stream_w);
    fflush(stream_w);

    // read and compare
    char expected_false[] = "false";
    int false_len = strlen(expected_false);
    char actual_false[false_len];
    fread(actual_false, 1, false_len, stream_r);
    if(strncmp(expected_false, actual_false, strlen(expected_false)))
        fail("Got unexpected output from json_val_print for false type");

    // true

    // make and print JSON true value
    struct json_val* vt = json_val_new_true();
    json_val_print(vt, stream_w);
    fflush(stream_w);

    // read and compare
    char expected_true[] = "true";
    int true_len = strlen(expected_true);
    char actual_true[true_len];
    fread(actual_true, 1, true_len, stream_r);
    if(strncmp(expected_true, actual_true, strlen(expected_true)))
        fail("Got unexpected output from json_val_print for true type");

    // null

    // make and print JSON null value
    struct json_val* vn = json_val_new_null();
    json_val_print(vn, stream_w);
    fflush(stream_w);

    // read and compare
    char expected_null[] = "null";
    int null_len = strlen(expected_null);
    char actual_null[null_len];
    fread(actual_null, 1, null_len, stream_r);
    if(strncmp(expected_null, actual_null, strlen(expected_null)))
        fail("Got unexpected output from json_val_print for null type");

    return 0;
}

int test_json_obj_new() {
    struct json_obj* o = json_obj_new();

    // check that the object is empty
    if(o->first != NULL) {
        fail("Object is not empty");
        return 1;
    }

    return 0;
}

int test_json_obj_equal() {
    // json_obj_equal must compare contents, not pointers, so make two objects
    // with the same content but at different memory locations.
    struct json_obj* o1 = json_obj_new();
    struct json_obj* o2 = json_obj_new();
    struct json_val* v1i = json_val_new_integer(15);
    struct json_val* v2i = json_val_new_integer(15);
    struct json_val* v1s = json_val_new_string("Some funky string");
    struct json_val* v2s = json_val_new_string("Some funky string");
    struct json_arr* v1av = json_arr_new();
    struct json_val* v1a = json_val_new_array(v1av);
    struct json_arr* v2av = json_arr_new();
    struct json_val* v2a = json_val_new_array(v2av);
    struct json_obj* v1ov = json_obj_new();
    struct json_val* v1o = json_val_new_object(v1ov);
    struct json_obj* v2ov = json_obj_new();
    struct json_val* v2o = json_val_new_object(v2ov);
    struct json_val* v1t = json_val_new_true();
    struct json_val* v2t = json_val_new_true();
    struct json_val* v1f = json_val_new_false();
    struct json_val* v2f = json_val_new_false();
    struct json_val* v1n = json_val_new_null();
    struct json_val* v2n = json_val_new_null();

    // Add key-value pairs in different order, to make sure that json_obj_equal
    // compares them irrespectable of order.

    json_obj_set(o1, "integer", v1i);
    json_obj_set(o1, "object", v1o);
    json_obj_set(o1, "true", v1t);
    json_obj_set(o1, "string", v1s);
    json_obj_set(o1, "array", v1a);
    json_obj_set(o1, "false", v1f);
    json_obj_set(o1, "null", v1n);

    json_obj_set(o2, "string", v2s);
    json_obj_set(o2, "integer", v2i);
    json_obj_set(o2, "true", v2t);
    json_obj_set(o2, "false", v2f);
    json_obj_set(o2, "null", v2n);
    json_obj_set(o2, "array", v2a);
    json_obj_set(o2, "object", v2o);

    // compare objects
    if(json_obj_equal(o1, o2))
        return 0;

    return 1;
}

int test_json_obj_set() {

    // prepare object
    struct json_obj* o = json_obj_new();
    char* k = "A funny key";
    struct json_val* v = json_val_new_null();
    json_obj_set(o, k, v);

    // check that the object has at least one key-value pair
    if(o->first == NULL) {
        fail("Object does not have a key-value pair");
        return 1;
    }

    // check that the object has exactly one key-value pair
    if(o->first->next != NULL) {
        fail("Object has more than one key-value pair");
        return 1;
    }

    // check the key-value pair
    struct json_obj_kv* e = o->first;

    // check that the key is the one we put in
    if(e->key != k) {
        fail("The key is different than the one we put in");
        return 1;
    }

    // check that the value is the one we put in
    if(e->val != v) {
        fail("The value is different than the one we put in");
        return 1;
    }

    // add another key-value pair
    char* k2 = "Another funny key";
    struct json_val* v2 = json_val_new_null();
    json_obj_set(o, k2, v2);

    // check that a key-value pair has been appended to the already existing one
    if(e->next == NULL) {
        fail("After setting a second key on object, no key-value pair has been appended to the first one");
        return 1;
    }

    // check the second key-value pair
    struct json_obj_kv* e2 = e->next;

    // check that the second key is the one we put in
    if(e2->key != k2)
        fail("Second key is different than the one we put in");

    // check that the second value is the one we put in
    if(e2->val != v2)
        fail("Second value is different than the one we put in");

    return 0;
}

int test_json_obj_haskey() {

    // prepare object
    struct json_obj* o = json_obj_new();
    char* k = "A funny key";
    struct json_val* v = json_val_new_null();
    json_obj_set(o, k, v);

    // check that key is set in the object
    if(!json_obj_haskey(o, k))
        fail("Object does not have key that was added");

    return 0;
}

int test_json_obj_remove() {

    // prepare object
    struct json_obj* o = json_obj_new();
    char* k1 = "A funny key";
    char* k2 = "Another funny key";
    char* k3 = "Yet another funny key";
    struct json_val* v1 = json_val_new_null();
    struct json_val* v2 = json_val_new_null();
    struct json_val* v3 = json_val_new_null();
    json_obj_set(o, k1, v1);
    json_obj_set(o, k2, v2);
    json_obj_set(o, k3, v3);

    // remove second key
    json_obj_remove(o, k2);

    // Check that the other keys are still there.
    // That is, that the pointers have been correctly rewritten.
    if(!json_obj_haskey(o, k1))
        fail("Object somehow lost its first key-value pair: First element not reachable");
    if(!json_obj_haskey(o, k3))
        fail("Object key-value pair list pointers were not correctly rewritten: Formerly third element not reachable");

    return 0;
}

int test_json_obj_print() {

    // make pipe
    int pipe_fd[2];
    if(pipe(pipe_fd) == -1)
        fail("Can't create pipe from integer pair");
    int fd_r = pipe_fd[0];
    int fd_w = pipe_fd[1];

    // make streams from pipe FDs
    FILE* stream_r = fdopen(fd_r, "r");
    if(stream_r == NULL)
        fail("Cannot make stream from pipe read FD");
    FILE* stream_w = fdopen(fd_w, "w");
    if(stream_w == NULL)
        fail("Cannot make stream from pipe write FD");

    // empty json object was already tested in test_json_val_print

    // make and print JSON object
    // adding several values tests correct formatting with comma
    struct json_val* vn = json_val_new_null();
    struct json_val* vs = json_val_new_string("a string");
    struct json_obj* o = json_obj_new();
    json_obj_set(o, "one", vn);
    json_obj_set(o, "two", vs);
    json_obj_print(o, stream_w);
    fflush(stream_w);

    // read and compare
    char expected[] = "{\"one\":null,\"two\":\"a string\"}";
    char expected_len = strlen(expected);
    char actual[expected_len];
    fread(actual, 1, expected_len, stream_r);
    if(strncmp(expected, actual, expected_len))
        fail("Got unexpected output from json_obj_print");

    return 0;
}

int test_json_arr_new() {
    struct json_arr* a = json_arr_new();

    // check that the array is empty
    if(a->first != NULL) {
        fail("Array is not empty");
        return 1;
    }

    return 0;
}

int test_json_arr_equal() {
    // json_arr_equal must compare contents, not pointers, so make two arrays
    // with the same content but at different memory locations.
    struct json_arr* a1 = json_arr_new();
    struct json_arr* a2 = json_arr_new();
    struct json_val* v1i = json_val_new_integer(15);
    struct json_val* v2i = json_val_new_integer(15);
    struct json_val* v1s = json_val_new_string("Some funky string");
    struct json_val* v2s = json_val_new_string("Some funky string");
    struct json_arr* v1av = json_arr_new();
    struct json_val* v1a = json_val_new_array(v1av);
    struct json_arr* v2av = json_arr_new();
    struct json_val* v2a = json_val_new_array(v2av);
    struct json_obj* v1ov = json_obj_new();
    struct json_val* v1o = json_val_new_object(v1ov);
    struct json_obj* v2ov = json_obj_new();
    struct json_val* v2o = json_val_new_object(v2ov);
    struct json_val* v1t = json_val_new_true();
    struct json_val* v2t = json_val_new_true();
    struct json_val* v1f = json_val_new_false();
    struct json_val* v2f = json_val_new_false();
    struct json_val* v1n = json_val_new_null();
    struct json_val* v2n = json_val_new_null();

    json_arr_append(a1, v1i);
    json_arr_append(a1, v1o);
    json_arr_append(a1, v1t);
    json_arr_append(a1, v1s);
    json_arr_append(a1, v1a);
    json_arr_append(a1, v1f);
    json_arr_append(a1, v1n);

    json_arr_append(a2, v2i);
    json_arr_append(a2, v2o);
    json_arr_append(a2, v2t);
    json_arr_append(a2, v2s);
    json_arr_append(a2, v2a);
    json_arr_append(a2, v2f);
    json_arr_append(a2, v2n);

    // compare arrays
    if(json_arr_equal(a1, a2))
        return 0;

    return 1;
}

int test_json_arr_append() {
    struct json_arr* a = json_arr_new();

    // check that the array is empty
    if(a->first != NULL)
        fail("Freshly created array has first element");

    // append value
    struct json_val* v = json_val_new_null();
    json_arr_append(a, v);

    // check that the array contains one element
    if(a->first == NULL)
        fail("Array is empty after appending element");

    // check that the array contains not more than one element
    if(a->first->next != NULL)
        fail("Array contains more than one element after adding just one");

    // check that the array element is the one we put in
    if(a->first->val != v)
        fail("The element in the array is not the one we put in");

    return 0;
}

int test_json_arr_has() {
    struct json_arr* a = json_arr_new();
    struct json_val* v = json_val_new_null();

    // test negative outcome
    if(json_arr_has(a, v))
        fail("json_arr_has returns non-zero for empty array");

    // append value
    // json_arr_append has been tested above, so we know that it works
    json_arr_append(a, v);

    // test positive outcome
    if(!json_arr_has(a, v))
        fail("json_arr_has returns zero for non-empty array");

    return 0;
}

int test_json_arr_length() {
    struct json_arr* a = json_arr_new();
    struct json_val* v = json_val_new_null();

    // 0 elements
    if(json_arr_length(a) != 0)
        return 1;

    // add first element
    json_arr_append(a, v);

    // 1 elements
    if(json_arr_length(a) != 1)
        return 1;

    // add first element
    json_arr_append(a, v);

    // 2 elements
    if(json_arr_length(a) != 2)
        return 1;

    return 0;
}

int test_json_arr_remove() {
    struct json_arr* a = json_arr_new();

    // append value
    // json_arr_append has been tested above, so we know that it works
    struct json_val* v = json_val_new_null();
    json_arr_append(a, v);

    // check that array has value
    if(!json_arr_has(a, v))
        fail("Array empty after value has been appended");

    // remove value
    json_arr_remove(a, v);

    // check that array is empty
    if(json_arr_has(a, v))
        fail("Value not removed from array");

    return 0;
}

int test_json_arr_print() {

    // make pipe
    int pipe_fd[2];
    if(pipe(pipe_fd) == -1)
        fail("Can't create pipe from integer pair");
    int fd_r = pipe_fd[0];
    int fd_w = pipe_fd[1];

    // make streams from pipe FDs
    FILE* stream_r = fdopen(fd_r, "r");
    if(stream_r == NULL)
        fail("Cannot make stream from pipe read FD");
    FILE* stream_w = fdopen(fd_w, "w");
    if(stream_w == NULL)
        fail("Cannot make stream from pipe write FD");

    // make and print JSON array
    // adding several values tests correct formatting with comma
    struct json_val* vn = json_val_new_null();
    struct json_val* vs = json_val_new_string("a string");
    struct json_arr* a = json_arr_new();
    json_arr_append(a, vn);
    json_arr_append(a, vs);
    json_arr_print(a, stream_w);
    fflush(stream_w);

    // read and compare
    char expected[] = "[null,\"a string\"]";
    char expected_len = strlen(expected);
    char actual[expected_len];
    fread(actual, 1, expected_len, stream_r);
    if(strncmp(expected, actual, expected_len))
        fail("Got unexpected output from json_arr_print");

    return 0;
}
