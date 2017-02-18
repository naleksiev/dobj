#include "dobject.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#ifndef NULL
#   define NULL 0
#endif

typedef struct do_str_t do_str_t;
typedef struct do_obj_t do_obj_t;
typedef struct do_arr_t do_arr_t;

typedef struct do_ctx_t {
    do_alloc alloc;
} do_ctx_t;

typedef struct do_value_t {
    do_val_type_t type;
    union {
        bool      b;
        int64_t   i;
        double    f;
        do_str_t* s;
        do_obj_t* o;
        do_arr_t* a;
        void*     p;
    };
} do_value_t;

typedef struct do_str_t {
    int32_t len;
    char    cstr[1];
} do_str_t;

typedef struct do_obj_t {
    const do_type_t* type;
    do_value_t       fields[0];
} do_obj_t;

typedef struct do_arr_t {
    int32_t     capacity;
    int32_t     count;
    do_value_t* data;
} do_arr_t;

struct do_fld_t {
    do_value_t    name;
    do_val_type_t type;
};

struct do_type_t {
    do_value_t name;
    int32_t    count;
    do_fld_t   fields[0];
};

static_assert(sizeof(do_value_t) == sizeof(do_val_t), "do_val_t and do_value_t should have identical size.");

static void* do_allocator(void* ptr, int32_t size) {
    if (ptr) {
        if (size) {
            return realloc(ptr, size);
        } else {
            free(ptr);
            return NULL;
        }
    } else if (size) {
        return malloc(size);
    }
    return NULL;
}

do_ctx_t* do_ctx_create(do_alloc alloc) {
    alloc = alloc ? alloc : &do_allocator;
    do_ctx_t* ctx = (do_ctx_t*)alloc(NULL, sizeof(do_ctx_t));
    ctx->alloc = alloc;
    return ctx;
}

void do_ctx_destroy(do_ctx_t* ctx) {
    ctx->alloc(ctx, 0);
}

do_type_t* do_type_create(do_ctx_t* ctx, const char* name, const do_fld_def_t fields[], int32_t count) {
    do_type_t* type = (do_type_t*)ctx->alloc(NULL, sizeof(do_type_t) + sizeof(do_fld_t) * count);
    do_val_t tn = do_val_str(ctx, name);
    type->name = *(do_value_t*)&tn;
    type->count = count;
    for (int32_t i=0; i<count; i++) {
        do_val_t fn = do_val_str(ctx, fields[i].name);
        type->fields[i].name = *(do_value_t*)&fn;
        type->fields[i].type = fields[i].type;
    }
    return type;
}

void do_type_destroy(do_ctx_t* ctx, do_type_t* type) {
    do_val_destroy(ctx, *(do_val_t*)&type->name);
    for (int32_t i=0; i<type->count; i++)
        do_val_destroy(ctx, *(do_val_t*)&type->fields[i]);
    ctx->alloc(type, 0);
}

do_val_t do_type_name(const do_type_t* type) {
    return *(do_val_t*)&type->name;
}

int32_t do_type_count(const do_type_t* type) {
    return type->count;
}

const do_fld_t* do_type_field(const do_type_t* type, int32_t index) {
    return (index >= 0 && index < type->count) ? &type->fields[index] : NULL;
}

const do_fld_t* do_type_find(const do_type_t* type, const char* name) {
    for (int32_t i=0; i<type->count; i++) {
        if (strcmp(name, type->fields[i].name.s->cstr) == 0)
            return &type->fields[i];
    }
    return NULL;
}

do_val_t do_fld_name(const do_fld_t* field) {
    return *(do_val_t*)&field->name;
}

do_val_type_t do_fld_type(const do_fld_t* field) {
    return field->type;
}

do_val_t do_val_nil(do_ctx_t* ctx) {
    do_value_t v;
    v.type = do_val_type_nil;
    v.i = 0;
    return *(do_val_t*)&v;
}

do_val_t do_val_bool(do_ctx_t* ctx, bool value) {
    do_value_t v;
    v.type = do_val_type_bool;
    v.b = value;
    return *(do_val_t*)&v;
}

do_val_t do_val_int(do_ctx_t* ctx, int64_t value) {
    do_value_t v;
    v.type = do_val_type_int;
    v.i = value;
    return *(do_val_t*)&v;
}

do_val_t do_val_flt(do_ctx_t* ctx, double value) {
    do_value_t v;
    v.type = do_val_type_flt;
    v.f = value;
    return *(do_val_t*)&v;
}

do_val_t do_val_str(do_ctx_t* ctx, const char* string) {
    return do_val_str_len(ctx, string, string ? strlen(string) : 0);
}

do_val_t do_val_str_len(do_ctx_t* ctx, const char* string, int32_t length) {
    do_value_t v;
    v.type = do_val_type_str;
    if (length > 0) {
        v.s = (do_str_t*)ctx->alloc(NULL, sizeof(do_str_t) + length);
        v.s->len = length;
        strncpy(v.s->cstr, string, length + 1);
    }
    else
        v.s = NULL;
    return *(do_val_t*)&v;
}

do_val_t do_val_obj(do_ctx_t* ctx, const do_type_t* type) {
    do_value_t v;
    v.type = do_val_type_obj;
    v.o = (do_obj_t*)ctx->alloc(NULL, sizeof(do_obj_t) + sizeof(do_value_t) * type->count);
    v.o->type = type;
    do_val_t nil = do_val_nil(ctx);
    for (int32_t i=0; i<type->count; i++) {
        v.o->fields[i] = *(do_value_t*)&nil;
    }
    return *(do_val_t*)&v;
}

do_val_t do_val_arr(do_ctx_t* ctx) {
    do_value_t v;
    v.type = do_val_type_arr;
    v.a = (do_arr_t*)ctx->alloc(NULL, sizeof(do_arr_t));
    return *(do_val_t*)&v;
}

do_val_t do_val_ptr(do_ctx_t* ctx, void* ptr) {
    do_value_t v;
    v.type = do_val_type_ptr;
    v.p = ptr;
    return *(do_val_t*)&v;
}

do_val_type_t do_val_type(do_val_t val) {
    return ((do_value_t*)&val)->type;
}

do_val_t do_val_clone(do_ctx_t* ctx, do_val_t val) {
    do_value_t* pv = (do_value_t*)&val;
    switch (pv->type) {
        case do_val_type_nil:
        case do_val_type_bool:
        case do_val_type_int:
        case do_val_type_flt:
        case do_val_type_ptr:
            return val;
        case do_val_type_str:
            return do_val_str_len(ctx, do_str_cstr(val), do_str_len(val));
        case do_val_type_obj: {
            const do_type_t* type = do_obj_type(val);
            do_val_t clone = do_val_obj(ctx, type);
            for (int32_t i=0; i<do_type_count(type); i++) {
                const do_fld_t* fld = do_type_field(type, i);
                do_obj_set(clone, fld, do_val_clone(ctx, do_obj_get(val, fld)));
            }
            return clone;
        }
        case do_val_type_arr: {
            do_val_t clone = do_val_arr(ctx);
            for (int32_t i=0; i<do_arr_count(val); i++)
                do_arr_set(clone, i, do_val_clone(ctx, do_arr_get(val, i)));
            return clone;
        }
    }
}

void do_val_destroy(do_ctx_t* ctx, do_val_t val) {
    do_value_t* pv = (do_value_t*)&val;
    switch (pv->type) {
        case do_val_type_nil:
        case do_val_type_bool:
        case do_val_type_int:
        case do_val_type_flt:
        case do_val_type_ptr:
            break;
        case do_val_type_str:
            ctx->alloc(pv->s, 0);
            break;
        case do_val_type_obj: {
                const do_type_t* type = do_obj_type(val);
                for (int32_t i=0; i<do_type_count(type); i++)
                    do_val_destroy(ctx, do_obj_get(val, do_type_field(type, i)));
                ctx->alloc(pv->o, 0);
            }
            break;
        case do_val_type_arr: {
                for (int32_t i=0; i<do_arr_count(val); i++)
                    do_val_destroy(ctx, do_arr_get(val, i));
                ctx->alloc(pv->a, 0);
            }
            break;
    }
}

bool do_val_equal(do_val_t a, do_val_t b) {
    do_value_t* pa = (do_value_t*)&a;
    do_value_t* pb = (do_value_t*)&b;
    return pa->type == pb->type && pa->i == pb->i;
}

bool do_bool_get(do_val_t value) {
    do_value_t* pv = (do_value_t*)&value;
    return pv->type == do_val_type_bool ? pv->b : false;
}

int64_t do_int_get(do_val_t value) {
    do_value_t* pv = (do_value_t*)&value;
    return pv->type == do_val_type_int ? pv->i : 0;
}

double do_flt_get(do_val_t value) {
    do_value_t* pv = (do_value_t*)&value;
    return pv->type == do_val_type_flt ? pv->f : 0.0;
}

void* do_ptr_get(do_val_t val) {
    do_value_t* p = (do_value_t*)&val;
    return p->type == do_val_type_ptr ? p->p : NULL;
}

const char* do_str_cstr(do_val_t value) {
    do_value_t* pv = (do_value_t*)&value;
    return pv->type == do_val_type_str ? pv->s->cstr : "";
}

int32_t do_str_len(do_val_t value) {
    do_value_t* pv = (do_value_t*)&value;
    return pv->type == do_val_type_str ? pv->s->len : 0;
}

const do_type_t* do_obj_type(do_val_t obj) {
    do_value_t* po = (do_value_t*)&obj;
    return po->type == do_val_type_obj ? po->o->type : NULL;
}

do_val_t do_obj_get(do_val_t obj, const do_fld_t* field) {
    do_value_t* po = (do_value_t*)&obj;
    if (po->type != do_val_type_obj)
        return do_val_nil(NULL);
    int32_t idx = (int32_t)(field - po->o->type->fields);
    if (idx < 0 || idx >= po->o->type->count)
        return do_val_nil(NULL);
    return *(do_val_t*)&po->o->fields[idx];
}

void do_obj_set(do_val_t obj, const do_fld_t* field, do_val_t value) {
    do_value_t* po = (do_value_t*)&obj;
    if (po->type != do_val_type_obj)
        return;
    int32_t idx = (int32_t)(field - po->o->type->fields);
    if (idx < 0 || idx >= po->o->type->count)
        return;
    if (field->type != do_val_type_any && field->type != do_val_type(value))
        return;
    po->o->fields[idx] = *(do_value_t*)&value;
}

int32_t do_arr_count(do_val_t arr) {
    do_value_t* pa = (do_value_t*)&arr;
    return pa->type == do_val_type_arr ? pa->a->count : 0;
}

do_val_t do_arr_get(do_val_t arr, int32_t index) {
    do_value_t* pa = (do_value_t*)&arr;
    if (pa->type != do_val_type_arr)
        return do_val_nil(NULL);
    if (index < 0 || index >= pa->a->count)
        return do_val_nil(NULL);
    return *(do_val_t*)&pa->a->data[index];
}

void do_arr_set(do_val_t arr, int32_t index, do_val_t value) {
    do_value_t* pa = (do_value_t*)&arr;
    if (pa->type != do_val_type_arr)
        return;
    if (index < 0 || index >= pa->a->count)
        return;
    pa->a->data[index] = *(do_value_t*)&value;
}

void do_arr_add(do_ctx_t* ctx, do_val_t arr, do_val_t value) {
    do_arr_insert(ctx, arr, do_arr_count(arr), value);
}

void do_arr_insert(do_ctx_t* ctx, do_val_t arr, int32_t index, do_val_t value) {
    do_value_t* pa = (do_value_t*)&arr;
    if (pa->type != do_val_type_arr)
        return;
    do_arr_t* a = pa->a;
    if (a->capacity - a->count >= 0) {
        int32_t cap = a->count ? a->count * 2 : 4;
        do_value_t* data = (do_value_t*)ctx->alloc(NULL, sizeof(do_value_t) * cap);
        memcpy(data, a->data, sizeof(do_value_t) * index);
        data[index] = *(do_value_t*)&value;
        memcpy(&data[index+1], &a->data[index], sizeof(do_value_t) * (a->count - index));
        if (a->data)
            ctx->alloc(a->data, 0);
        a->data = data;
        a->capacity = cap;
        a->count++;
        return;
    }
    for (int32_t i=a->count; i>index; i--)
        a->data[i] = a->data[i-1];
    a->data[index] = *(do_value_t*)&value;
    a->count++;
}

void do_arr_remove(do_ctx_t* ctx, do_val_t arr, do_val_t value) {
    do_value_t* pa = (do_value_t*)&arr;
    if (pa->type != do_val_type_arr)
        return;
    do_arr_t* a = pa->a;
    for (int32_t i=0; i<a->count; i++) {
        if (do_val_equal(*(do_val_t*)&a->data[i], value)) {
            do_arr_remove_at(ctx, arr, i);
            break;
        }
    }
}

void do_arr_remove_at(do_ctx_t* ctx, do_val_t arr, int32_t index) {
    do_value_t* pa = (do_value_t*)&arr;
    if (pa->type != do_val_type_arr)
        return;
    do_arr_t* a = pa->a;
    if (a->count < a->capacity / 4) {
        int32_t cap = a->capacity / 4;
        do_value_t* data = (do_value_t*)ctx->alloc(NULL, sizeof(do_value_t) * cap);
        memcpy(data, a->data, sizeof(do_value_t) * index);
        memcpy(&data[index], &a->data[index+1], sizeof(do_value_t) * (a->count - index + 1));
        ctx->alloc(a->data, 0);
        a->data = data;
        a->capacity = cap;
        a->count--;
        return;
    }
    for (int32_t i=index; i<a->count; i++)
        a->data[i] = a->data[i+1];
    a->count--;
}

