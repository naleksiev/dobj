#include "../src/dobj.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#define COUNT_OF(x)     (sizeof(x) / sizeof(x[0]))
int32_t allocs = 0;

void* alloc(void* ptr, int32_t size) {
    if (ptr) {
        if (size) {
            return realloc(ptr, size);
        } else {
            allocs--;
            free(ptr);
            return NULL;
        }
    } else if (size) {
        allocs++;
        return malloc(size);
    }
    return NULL;
}

void dump(do_val_t val, int32_t indent) {
    int32_t i;
    int32_t j;

    for (i=0; i<indent; i++)
        printf("  ");
    switch (do_val_type(val)) {
        case do_val_type_nil:
            printf("nil");
            break;
        case do_val_type_bool:
            printf("%s", do_bool_get(val) ? "true" : "false");
            break;
        case do_val_type_int:
            printf("%lld", (long long int)do_int_get(val));
            break;
        case do_val_type_flt:
            printf("%g", do_flt_get(val));
            break;
        case do_val_type_str:
            printf("\"%s\"", do_str_cstr(val));
            break;
        case do_val_type_obj: {
            printf("{\n");
            const do_type_t* type = do_obj_type(val);
            for (i=0; i<do_type_count(type); i++) {
                for (j=0; j<=abs(indent); j++)
                    printf("  ");
                printf("%s : ", do_str_cstr(do_fld_name(do_type_field(type, i))));
                dump(do_obj_get(val, do_type_field(type, i)), -(abs(indent) + 1));
                if (i < do_type_count(type) - 1)
                    printf(",\n");
                else
                    printf("\n");
            }
            for (i=0; i<abs(indent); i++)
                printf("  ");
            printf("}");
            break;
        }
        case do_val_type_arr: {
            printf("[\n");
            for (i=0; i<do_arr_count(val); i++) {
                dump(do_arr_get(val, i), abs(indent) + 1);
                if (i < do_arr_count(val) - 1)
                    printf(",\n");
                else
                    printf("\n");
            }
            for (i=0; i<abs(indent); i++)
                printf("  ");
            printf("]");
            break;
        }
        case do_val_type_ptr:
            printf("0x%p", do_ptr_get(val));
            break;
        case do_val_type_any:
            break;
    }
    if (indent == 0)
        printf("\n");
}

int main() {
    do_ctx_t* ctx = do_ctx_create(&alloc);
    do_fld_def_t fields[] = {
        { "A", do_val_type_flt },
        { "B", do_val_type_obj },
        { "C", do_val_type_arr }
    };
    do_type_t* type = do_type_create(ctx, "TestObj", fields, COUNT_OF(fields));

    do_val_t obj = do_val_obj(ctx, type);
    do_obj_set(obj, do_type_find(type, "A"), do_val_flt(ctx, 1.2f));
    do_obj_set(obj, do_type_find(type, "B"), do_val_obj(ctx, type));
    do_obj_set(obj, do_type_find(type, "C"), do_val_arr(ctx));

    do_val_t arr = do_obj_get(obj, do_type_find(type, "C"));
    do_arr_add(ctx, arr, do_val_flt(ctx, 3.4f));
    do_arr_add(ctx, arr, do_val_nil(ctx));
    do_arr_add(ctx, arr, do_val_str(ctx, "Hello"));
    dump(obj, 0);

    do_val_destroy(ctx, obj);
    do_type_destroy(ctx, type);
    do_ctx_destroy(ctx);

    printf("allocations: %d\n", allocs);
    assert(allocs == 0);

    return 0;
}

