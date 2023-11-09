#include <stddef.h> // needed for null
#include <stdlib.h> // malloc
#include <stdio.h> // FILE
#include <string.h> // strcmp


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

struct json_obj_elem {
    char* key;
    struct json_val* val;
    struct json_obj_elem* next;
};

struct json_obj {
    struct json_obj_elem* first;
};

struct json_obj* json_obj_new() {
    struct json_obj* o = malloc(sizeof(struct json_obj));
    o->first = NULL;
    return o;
}

void json_obj_set(struct json_obj* o, char* key, struct json_val* v) {

    // seek and update
    struct json_obj_elem* last = NULL;
    for(struct json_obj_elem* e = o->first; e != NULL; e = e->next) {

        // found the key, so just update the value and we're done
        if(strcmp(e->key, key) == 0) {
            e->val = v;
            return;
        }

        last = e;
    }

    // no update happened, so we need to append the new key-value pair
    struct json_obj_elem* new = malloc(sizeof(struct json_obj_elem));
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
    for(struct json_obj_elem* e = o->first; e != NULL; e = e->next) {
        if(strcmp(e->key, key) == 0)
            return 1;
    }
    return 0;
}

// forward declaration for json_obj_remove
void json_obj_elem_free(struct json_obj_elem* e);

void json_obj_remove(struct json_obj* o, char* key) {
    struct json_obj_elem* last = NULL;
    for(struct json_obj_elem* e = o->first; e != NULL; e = e->next) {
        if(strcmp(e->key, key) == 0) {
            // relink (e->next might be NULL and that's fine)
            if(last == NULL) {
                // this is the first element of the key-value linked list
                o->first = e->next;
            } else {
                last->next = e->next;
            }
            json_obj_elem_free(e);
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
        json_obj_elem_free(o->first);
    free(o);
}

void json_obj_elem_free(struct json_obj_elem* e) {
    // Keys are managed by the caller and thus don't have to be freed.
    if(e->next != NULL)
        json_obj_elem_free(e->next);
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
    if(a->type == Array)
        return json_obj_equal(a->val.o, b->val.o);
    // other cases are unknown
    return 0;
}

int json_obj_equal(struct json_obj* a, struct json_obj* b) {
    if(a == b)
        return 1;
    // Key-value pairs might be in different order, so we have to compare each
    // one from a with each one from b.
    for(struct json_obj_elem* ae = a->first; ae != NULL; ae = ae->next) {
        // look for matching key-value pair in b
        int found = 0;
        for(struct json_obj_elem* be = b->first; be != NULL; be = be->next) {
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
    for(struct json_arr_elem* ae = a->first; ae != NULL; ae = ae->next) {
        if(json_val_equal(ae->val, be->val) != 0)
            return 0;
        be = be->next;
    }

    // we're at the end of a, but is it also the end of b?
    if(be != NULL)
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
    struct json_obj_elem* e = o->first;
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
