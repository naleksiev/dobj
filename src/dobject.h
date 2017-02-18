#ifndef DATA_STRUCTURE_H
#define DATA_STRUCTURE_H

#include <stdint.h>
#include <stdbool.h>

typedef struct do_ctx_t  do_ctx_t;
typedef struct do_fld_t  do_fld_t;
typedef struct do_type_t do_type_t;

typedef enum do_val_type_t {
    do_val_type_nil,
    do_val_type_bool,
    do_val_type_int,
    do_val_type_flt,
    do_val_type_str,
    do_val_type_obj,
    do_val_type_arr,
    do_val_type_ptr,
    do_val_type_any
} do_val_type_t;

typedef struct do_val_t {
    void* data[2];
} do_val_t;

typedef struct do_fld_def_t {
    const char*   name;
    do_val_type_t type;
} do_fld_def_t;

typedef void* (*do_alloc)(void* ptr, int32_t size);

do_ctx_t*        do_ctx_create(do_alloc alloc);
void             do_ctx_destroy(do_ctx_t* ctx);

do_type_t*       do_type_create(do_ctx_t* ctx, const char* name, const do_fld_def_t fields[], int32_t count);
void             do_type_destroy(do_ctx_t* ctx, do_type_t* type);
do_val_t         do_type_name(const do_type_t* type);
int32_t          do_type_count(const do_type_t* type);
const do_fld_t*  do_type_field(const do_type_t* type, int32_t index);
const do_fld_t*  do_type_find(const do_type_t* type, const char* name);

do_val_t         do_fld_name(const do_fld_t* field);
do_val_type_t    do_fld_type(const do_fld_t* field);

do_val_t         do_val_nil(do_ctx_t* ctx);
do_val_t         do_val_bool(do_ctx_t* ctx, bool value);
do_val_t         do_val_int(do_ctx_t* ctx, int64_t value);
do_val_t         do_val_flt(do_ctx_t* ctx, double value);
do_val_t         do_val_str(do_ctx_t* ctx, const char* string);
do_val_t         do_val_str_len(do_ctx_t* ctx, const char* string, int32_t length);
do_val_t         do_val_obj(do_ctx_t* ctx, const do_type_t* type);
do_val_t         do_val_arr(do_ctx_t* ctx);
do_val_t         do_val_ptr(do_ctx_t* ctx, void* ptr);

do_val_type_t    do_val_type(do_val_t val);
do_val_t         do_val_clone(do_ctx_t* ctx, do_val_t val);
void             do_val_destroy(do_ctx_t* ctx, do_val_t val);
bool             do_val_equal(do_val_t a, do_val_t b);

bool             do_bool_get(do_val_t value);
int64_t          do_int_get(do_val_t value);
double           do_flt_get(do_val_t value);
void*            do_ptr_get(do_val_t val);

const char*      do_str_cstr(do_val_t value);
int32_t          do_str_len(do_val_t value);

const do_type_t* do_obj_type(do_val_t obj);
do_val_t         do_obj_get(do_val_t obj, const do_fld_t* field);
void             do_obj_set(do_val_t obj, const do_fld_t* field, do_val_t value);

int32_t          do_arr_count(do_val_t arr);
do_val_t         do_arr_get(do_val_t arr, int32_t index);
void             do_arr_set(do_val_t arr, int32_t index, do_val_t value);
void             do_arr_add(do_ctx_t* ctx, do_val_t arr, do_val_t value);
void             do_arr_insert(do_ctx_t* ctx, do_val_t arr, int32_t index, do_val_t value);
void             do_arr_remove(do_ctx_t* ctx, do_val_t arr, do_val_t value);
void             do_arr_remove_at(do_ctx_t* ctx, do_val_t arr, int32_t index);

#endif

