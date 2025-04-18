/****************************************************************************************
//
//    CTX - Basic memory management library
//
//    Project Author: Alex Macafee
//    Project Repo: https://github.com/lxmcf/ctx
//
//    DESCRIPTION:
//        CTX is a simple single header library that implements a simple memory
//        management system.
//
//    LIMITATIONS:
//        Does not have dynamic sized contexts, if you run out of room in a context you
//        will need to create a new context (This may be implemented in the future).
//
//    QUICK NOTES:
//        * This is a very basic library, if you are looking for something more robust I
//          would recommend looking at https://github.com/tsoding/arena
//        * If using the temporary context with a main loop (EG. Game), you will need to
//          call context_tclear () at the end of every frame.
//        * When you use a context, all library code and logs will refer to it as
//          a "static context"
//
//    CONFIGURATION:
//        #define CTX_IMPLEMENTATION or #define CTX_IMPL
//            Creates the function function definition's for the library, you can use
//            either define.
//
//            If not defined; the library will be in header mode and can be included in
//            any amount of files for use. Only ONE file can hold the implementation.
//
//        #define CTX_MALLOC(x)
//            If you do not wish to use the standard std malloc; you can define your
//            own (Will use calloc by default).
//
//        #define CTX_FREE(x)
//            Similar to the above, if using a custom memory allocation function, you can
//            provide a custom free function.
//
//        #define CTX_NO_SIZE_HELPERS
//            Disables the helper macros for easy data size allocations.
//
//        #define CTX_NO_TEMP
//            Disables the built in temp context and its functions.
//
//        #define CTX_NO_STR
//            Disables the string helper functions.
//
//        #define CTX_TEMP_SIZE X
//            Defines the size of the built in temporary context, will default to 1MB.
//
//        #define CTX_LOG(...)
//            If you do not wish to use printf, you can use this to use a custom logger.
//
//    CHANGELOG:
//        1.0.0 (2025-03-01) - Initial release.
//        1.1.0 (2025-03-29) - Added forget functions and string helpers with config.
//
//    LICENSE:
//        Copyright (c) 2025 Alex Macafee
//
//        This software is provided ‘as-is’, without any express or implied
//        warranty. In no event will the authors be held liable for any damages
//        arising from the use of this software.
//
//        Permission is granted to anyone to use this software for any purpose,
//        including commercial applications, and to alter it and redistribute it
//        freely, subject to the following restrictions:
//
//        1. The origin of this software must not be misrepresented; you must not
//        claim that you wrote the original software. If you use this software
//        in a product, an acknowledgment in the product documentation would be
//        appreciated but is not required.
//
//        2. Altered source versions must be plainly marked as such, and must not be
//        misrepresented as being the original software.
//
//        3. This notice may not be removed or altered from any source
//        distribution.
//
****************************************************************************************/

#ifndef CTX_H
#define CTX_H

#define CTX_VERSION_MAJOR 1
#define CTX_VERSION_MINOR 1
#define CTX_VERSION_PATCH 0
#define CTX_VERSION       "1.1.0"

#ifdef _WIN32
#if defined(CTX_BUILD_SHARED)
#define CTX_API __declspec (dllexport)
#elif defined(CTX_USE_SHARED)
#define CTX_API __declspec (dllimport)
#endif
#endif // _WIN32

// -----------------------------------------------------------------------------
// CONFIG
// -----------------------------------------------------------------------------
#ifndef CTX_MALLOC
#include <stdlib.h>
#define CTX_MALLOC(x) calloc (1, x)
#endif

#ifndef CTX_FREE
#include <stdlib.h>
#define CTX_FREE(x) free (x)
#endif

#ifndef CTX_LOG
#include <stdio.h>
#define CTX_LOG(...) printf (__VA_ARGS__)
#endif

#if !defined(CTX_NO_TEMP) && !defined(CTX_TEMP_SIZE)
#define CTX_TEMP_SIZE 1 MB
#endif

#ifndef CTX_API
#define CTX_API
#endif

#ifndef NULL
#define NULL ((void*)0)
#endif

#ifndef CTX_NO_SIZE_HELPERS
#define KB *1024
#define MB *(1024 * 1024)
#define GB *(1024 * 1024 * 1024)
#endif

// -----------------------------------------------------------------------------
// TYPES DEFINITION
// -----------------------------------------------------------------------------
#include <stddef.h>

#ifndef CTX_NO_STR
#include <stdarg.h>
#include <string.h>
#endif

typedef struct Context {
    void* buffer;
    size_t location;
    size_t last_location;
    size_t size;
} Context;

#ifdef __cplusplus
extern "C" {
#endif

CTX_API Context new_context (size_t size);
CTX_API void* context_alloc (Context* context, size_t size);
CTX_API size_t context_forget (Context* context);

#ifndef CTX_NO_STR
CTX_API char* context_alloc_cstring (Context* context, const char* str);
CTX_API char* context_alloc_cstringf (Context* context, const char* fmt, ...);
#endif

CTX_API void context_clear (Context* context);
CTX_API void context_free (Context* context);

#ifndef CTX_NO_TEMP
CTX_API void* context_talloc (size_t size);
CTX_API size_t context_tforget (void);

#ifndef CTX_NO_STR
CTX_API char* context_talloc_cstring (const char* str);
CTX_API char* context_talloc_cstringf (const char* fmt, ...);
#endif

CTX_API void context_tclear (void);
CTX_API void context_tfree (void);
#endif // CTX_NO_TEMP

#ifdef __cplusplus
}
#endif

// -----------------------------------------------------------------------------
// function IMPLEMENTATION
// -----------------------------------------------------------------------------
#define CTX_IMPL
#if defined(CTX_IMPL) || defined(CTX_IMPLEMENTATION)

#ifndef CTX_NO_TEMP
static Context global_temp_context = {NULL, 0, 0, 0};
#endif

#ifdef __cplusplus
extern "C" {
#endif

Context new_context (size_t size) {
    Context ctx = {
        .buffer        = CTX_MALLOC (size),
        .location      = 0,
        .last_location = 0,
        .size          = size,
    };

    return ctx;
}

void* context_alloc (Context* context, size_t size) {
    if (context->location + size > context->size) {
        CTX_LOG ("[ERROR]: Static context unable to allocate %zu bytes!\n", size);
        return NULL;
    }

    context->last_location = context->location;

    char* buffer_start = (void*)context->buffer;
    void* chunk        = &buffer_start[context->location];

    context->location += size;
    return chunk;
}

size_t context_forget (Context* context) {
    if (context->last_location > context->location) {
        CTX_LOG ("[ERROR]: Cannot forget last allocation!\n");
        return 0;
    }

    size_t reverted   = context->location - context->last_location;
    context->location = context->last_location;

    return reverted;
}

#ifndef CTX_NO_STR
char* context_alloc_cstring (Context* context, const char* str) {
    size_t string_length = strlen (str);

    char* chunk = context_alloc (context, string_length + 1);
    if (chunk == NULL) {
        return NULL;
    }

    strcpy (chunk, str);

    return chunk;
}

char* context_alloc_cstringf (Context* context, const char* fmt, ...) {
    va_list args;
    va_start (args, fmt);

    va_list args_copy;
    va_copy (args_copy, args);
    size_t string_length = vsnprintf (NULL, 0, fmt, args_copy) + 1;
    va_end (args_copy);

    char* buffer = context_alloc (context, string_length);
    if (buffer == NULL) {
        va_end (args);
        return NULL;
    }

    vsnprintf (buffer, string_length, fmt, args);
    va_end (args);

    return NULL;
}
#endif

void context_clear (Context* context) {
    context->location      = 0;
    context->last_location = 0;
}

void context_free (Context* context) {
    context->last_location = 0;
    context->location      = 0;
    context->size          = 0;

    CTX_FREE (context->buffer);
}

#ifndef CTX_NO_TEMP
void* context_talloc (size_t size) {
    if (global_temp_context.size == 0) {
        global_temp_context = new_context (CTX_TEMP_SIZE);
    }

    return context_alloc (&global_temp_context, size);
}

size_t context_tforget (void) {
    return context_forget (&global_temp_context);
}

#ifndef CTX_NO_STR
char* context_talloc_cstring (const char* str) {
    if (global_temp_context.size == 0) {
        global_temp_context = new_context (CTX_TEMP_SIZE);
    }

    return context_alloc_cstring (&global_temp_context, str);
}

char* context_talloc_cstringf (const char* fmt, ...) {
    if (global_temp_context.size == 0) {
        global_temp_context = new_context (CTX_TEMP_SIZE);
    }

    va_list args;
    va_start (args, fmt);

    va_list args_copy;
    va_copy (args_copy, args);
    size_t string_length = vsnprintf (NULL, 0, fmt, args_copy) + 1;
    va_end (args_copy);

    char* buffer = context_alloc (&global_temp_context, string_length);
    if (buffer == NULL) {
        va_end (args);
        return NULL;
    }

    vsnprintf (buffer, string_length, fmt, args);
    va_end (args);

    return buffer;
}
#endif

void context_tclear (void) {
    context_clear (&global_temp_context);
}

void context_tfree (void) {
    context_free (&global_temp_context);
}
#endif // CTX_NO_TEMP

#ifdef __cplusplus
}
#endif

#endif

#endif // CTX_H
