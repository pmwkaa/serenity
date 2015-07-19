#ifndef SN_MACRO_H_
#define SN_MACRO_H_

/*
 * sophia database
 * sphia.org
 *
 * Copyright (c) Dmitry Simonenko
 * BSD License
*/

#if __GNUC__ >= 4 && __GNUC_MINOR__ >= 3
#  define snhot __attribute__((hot))
#else
#  define snhot
#endif

#define snpacked __attribute__((packed))
#define snunused __attribute__((unused))

#define sncast(N, T, F) ((T*)((char*)(N) - __builtin_offsetof(T, F)))

#define snlikely(EXPR)   __builtin_expect(!! (EXPR), 1)
#define snunlikely(EXPR) __builtin_expect(!! (EXPR), 0)

#endif
