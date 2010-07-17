/*
 * Copyright (C) 1993-2003 Yukihiro Matsumoto
 * Copyright (C) 2000 Network Applied Communication Laboratory, Inc.
 * Copyright (C) 2000 Information-technology Promotion Agency, Japan
 * Copyright (C) 2010 Tim Felgentreff
 *
 * This file is part of jruby-cext.
 *
 * This code is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License version 3 only, as
 * published by the Free Software Foundation.
 *
 * This code is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * version 3 for more details.
 *
 * You should have received a copy of the GNU General Public License
 * version 3 along with this work.  If not, see <http://www.gnu.org/licenses/>.
 */


#include "jruby.h"
#include "ruby.h"
#include "JLocalEnv.h"
#include "Handle.h"

using namespace jruby;

static VALUE
convert_type(VALUE val, const char* type_name, const char* method, int raise)
{
    ID m = rb_intern(method);
    if (!rb_respond_to(val, m)) {
        if (raise) {
            rb_raise(rb_eTypeError, "can't convert %s into %s",
                    NIL_P(val) ? "nil" :
                    val == Qtrue ? "true" :
                    val == Qfalse ? "false" :
                    rb_obj_classname(val),
                    type_name);
        } else {
            return Qnil;
        }
    }
    return rb_funcall(val, m, 0);
}

extern "C" VALUE 
rb_obj_freeze(VALUE obj)
{    
    return callMethodA(obj, "freeze", 0, NULL);
}

extern "C" char*
rb_obj_classname(VALUE obj)
{
    return rb_class2name(rb_class_of(obj));
}

extern "C" int
rb_respond_to(VALUE obj, ID id)
{
    JLocalEnv env;
    return env->CallBooleanMethod(valueToObject(env, obj),
            IRubyObject_respondsTo_method, idToString(env, id)) != JNI_FALSE;
}

extern "C" VALUE
rb_convert_type(VALUE val, int type, const char* type_name, const char* method)
{
    VALUE v;

    if (TYPE(val) == type) {
        return val;
    }

    v = convert_type(val, type_name, method, Qtrue);

    if (TYPE(v) != type) {
        rb_raise(rb_eTypeError, "%s#%s should return %s",
            rb_obj_classname(val), method, type_name);
    }

    return v;
}

extern "C" VALUE
rb_iv_get(VALUE obj, const char* name) {
    JLocalEnv env;

    char var_name[strlen(name) + 1];
    (name[0] != '@') ? strcpy(var_name, "@")[0] : var_name[0] = '\0';
    strcat(var_name, name);

    jmethodID mid = getMethodID(env, RubyBasicObject_class, "getInstanceVariable",
            "(Ljava/lang/String;)Lorg/jruby/runtime/builtin/IRubyObject;");
    jobject retval = env->CallObjectMethod(valueToObject(env, obj), mid, env->NewStringUTF(var_name));
    checkExceptions(env);
    return objectToValue(env, retval);
}

extern "C" VALUE
rb_iv_set(VALUE obj, const char* name, VALUE value) {
    JLocalEnv env;

    char var_name[strlen(name) + 1];
    (name[0] != '@') ? strcpy(var_name, "@")[0] : var_name[0] = '\0';
    strcat(var_name, name);

    jmethodID mid = getMethodID(env, RubyBasicObject_class, "setInstanceVariable",
            "(Ljava/lang/String;Lorg/jruby/runtime/builtin/IRubyObject;)Lorg/jruby/runtime/builtin/IRubyObject;");
    jobject retval = env->CallObjectMethod(valueToObject(env, obj), mid, env->NewStringUTF(var_name),
            valueToObject(env, value));
    checkExceptions(env);
    return objectToValue(env, retval);
}

extern "C" VALUE
rb_ivar_get(VALUE obj, ID ivar_name) {
    return rb_iv_get(obj, rb_id2name(ivar_name));
}

extern "C" VALUE
rb_ivar_set(VALUE obj, ID ivar_name, VALUE value) {
    return rb_iv_set(obj, rb_id2name(ivar_name), value);
}

extern "C" void
rb_obj_call_init(VALUE recv, int arg_count, VALUE* args) {
    callMethodANonConst(recv, "initialize", arg_count, args);
}

extern "C" VALUE
rb_obj_is_kind_of(VALUE obj, VALUE module) {
    return callMethod(obj, "kind_of?", 1, module);
}