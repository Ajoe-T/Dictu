#include "json.h"

static Value parseJson(json_value *json) {
    switch (json->type) {
        case json_none:
        case json_null: {
            return NIL_VAL;
        }

        case json_object: {
            ObjDict *dict = initDict();
            for (unsigned int i = 0; i < json->u.object.length; i++) {
                insertDict(dict, json->u.object.values[i].name, parseJson(json->u.object.values[i].value));
            }

            return OBJ_VAL(dict);
        }

        case json_array: {
            ObjList *list = initList();
            for (unsigned int i = 0; i < json->u.array.length; i++) {
                writeValueArray(&list->values, parseJson(json->u.array.values[i]));
            }

            return OBJ_VAL(list);
        }

        case json_integer: {
            return NUMBER_VAL(json->u.integer);
        }

        case json_double: {
            return NUMBER_VAL(json->u.dbl);
        }

        case json_string: {
            return OBJ_VAL(copyString(json->u.string.ptr, json->u.string.length));
        }

        case json_boolean: {
            return BOOL_VAL(json->u.boolean);
        }

        default: {
            return EMPTY_VAL;
        }
    }
}

static Value parse(int argCount, Value *args) {
    if (argCount != 1) {
        runtimeError("parse() takes 1 argument (%d  given)", argCount);
        return EMPTY_VAL;
    }

    if (!IS_STRING(args[0])) {
        runtimeError("parse() argument must be a string.");
        return EMPTY_VAL;
    }

    ObjString *json = AS_STRING(args[0]);
    json_value *json_obj = json_parse(json->chars, json->length);

    if (json_obj == NULL) {
        runtimeError("Invalid JSON passed to parse()");
        return EMPTY_VAL;
    }

    Value val = parseJson(json_obj);

    if (val == EMPTY_VAL) {
        runtimeError("Invalid JSON passed to parse()");
        return EMPTY_VAL;
    }

    json_value_free(json_obj);

    return val;
}

static Value stringify(int argCount, Value *args) {
    if (argCount != 1) {
        runtimeError("parse() takes 1 argument (%d  given)", argCount);
        return EMPTY_VAL;
    }

    Value value = args[0];
    json_value *json = NULL;

    if (IS_NIL(value)) {
        json = json_null_new();
    }

    char *buf = malloc(json_measure(json));
    json_serialize(buf, json);

    ObjString *string = copyString(buf, strlen(buf));
    free(buf);
    return OBJ_VAL(string);
}

void createJSONClass() {
    ObjString *name = copyString("JSON", 4);
    push(OBJ_VAL(name));
    ObjClassNative *klass = newClassNative(name);
    push(OBJ_VAL(klass));

    /**
     * Define Json methods
     */
    defineNativeMethod(klass, "parse", parse);
    defineNativeMethod(klass, "stringify", stringify);

    tableSet(&vm.globals, name, OBJ_VAL(klass));
    pop();
    pop();
}