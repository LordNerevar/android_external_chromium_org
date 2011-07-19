/*
 * Mesa 3-D graphics library
 * Version:  7.1
 *
 * Copyright (C) 1999-2008  Brian Paul   All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * BRIAN PAUL BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
 * AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */


/**
 * \mainpage Mesa GL API Module
 *
 * \section GLAPIIntroduction Introduction
 *
 * The Mesa GL API module is responsible for dispatching all the
 * gl*() functions.  All GL functions are dispatched by jumping through
 * the current dispatch table (basically a struct full of function
 * pointers.)
 *
 * A per-thread current dispatch table and per-thread current context
 * pointer are managed by this module too.
 *
 * This module is intended to be non-Mesa-specific so it can be used
 * with the X/DRI libGL also.
 */


#ifndef _GLAPI_H
#define _GLAPI_H


/* opengl.dll does not export _glapi_* */
#if defined(_WIN32)
#define _GLAPI_NO_EXPORTS
#endif

#ifdef _GLAPI_NO_EXPORTS
#  define _GLAPI_EXPORT
#else /* _GLAPI_NO_EXPORTS */
#  ifdef _WIN32
#    ifdef _GLAPI_DLL_EXPORTS
#      define _GLAPI_EXPORT __declspec(dllexport)
#    else
#      define _GLAPI_EXPORT __declspec(dllimport)
#    endif
#  elif defined(__GNUC__) || (defined(__SUNPRO_C) && (__SUNPRO_C >= 0x590))
#    define _GLAPI_EXPORT __attribute__((visibility("default")))
#  else
#    define _GLAPI_EXPORT
#  endif
#endif /* _GLAPI_NO_EXPORTS */


/* Is this needed?  It is incomplete anyway. */
#ifdef USE_MGL_NAMESPACE
#define _glapi_set_dispatch _mglapi_set_dispatch
#define _glapi_get_dispatch _mglapi_get_dispatch
#define _glapi_set_context _mglapi_set_context
#define _glapi_get_context _mglapi_get_context
#define _glapi_Dispatch _mglapi_Dispatch
#define _glapi_Context _mglapi_Context
#endif

#include "glapi/glthread.h"

typedef void (*_glapi_proc)(void);
struct _glapi_table;


#if defined (GLX_USE_TLS)

_GLAPI_EXPORT extern __thread struct _glapi_table * _glapi_tls_Dispatch
    __attribute__((tls_model("initial-exec")));

_GLAPI_EXPORT extern __thread void * _glapi_tls_Context
    __attribute__((tls_model("initial-exec")));

_GLAPI_EXPORT extern const struct _glapi_table *_glapi_Dispatch;
_GLAPI_EXPORT extern const void *_glapi_Context;

# define GET_DISPATCH() _glapi_tls_Dispatch
# define GET_CURRENT_CONTEXT(C)  GLcontext *C = (GLcontext *) _glapi_tls_Context

#else

_GLAPI_EXPORT extern struct _glapi_table *_glapi_Dispatch;
_GLAPI_EXPORT extern void *_glapi_Context;

# ifdef THREADS

#  define GET_DISPATCH() \
     (likely(_glapi_Dispatch) ? _glapi_Dispatch : _glapi_get_dispatch())

#  define GET_CURRENT_CONTEXT(C)  GLcontext *C = (GLcontext *) \
     (likely(_glapi_Context) ? _glapi_Context : _glapi_get_context())

# else

#  define GET_DISPATCH() _glapi_Dispatch
#  define GET_CURRENT_CONTEXT(C)  GLcontext *C = (GLcontext *) _glapi_Context

# endif

#endif /* defined (GLX_USE_TLS) */


void
_glapi_destroy_multithread(void);


_GLAPI_EXPORT void
_glapi_check_multithread(void);


_GLAPI_EXPORT void
_glapi_set_context(void *context);


_GLAPI_EXPORT void *
_glapi_get_context(void);


_GLAPI_EXPORT void
_glapi_set_dispatch(struct _glapi_table *dispatch);


_GLAPI_EXPORT struct _glapi_table *
_glapi_get_dispatch(void);


_GLAPI_EXPORT unsigned int
_glapi_get_dispatch_table_size(void);


_GLAPI_EXPORT int
_glapi_add_dispatch( const char * const * function_names,
		     const char * parameter_signature );

_GLAPI_EXPORT int
_glapi_get_proc_offset(const char *funcName);


_GLAPI_EXPORT _glapi_proc
_glapi_get_proc_address(const char *funcName);


_GLAPI_EXPORT const char *
_glapi_get_proc_name(unsigned int offset);


_GLAPI_EXPORT unsigned long
_glthread_GetID(void);


/*
 * These stubs are kept so that the old DRI drivers still load.
 */
_GLAPI_EXPORT void
_glapi_noop_enable_warnings(unsigned char enable);


_GLAPI_EXPORT void
_glapi_set_warning_func(_glapi_proc func);


#endif /* _GLAPI_H */
