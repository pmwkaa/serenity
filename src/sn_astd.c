
/*
 * serenity database
 *
 * Copyright (c) Dmitry Simonenko
 * BSD License
*/

#include <serenity.h>

static inline int
sn_stdaopen(sna *a snunused, va_list args snunused) {
	return 0;
}

static inline int
sn_stdaclose(sna *a snunused) {
	return 0;
}

static inline void*
sn_stdamalloc(sna *a snunused, int size) {
	return malloc(size);
}

static inline void*
sn_stdarealloc(sna *a snunused, void *ptr, int size) {
	return realloc(ptr,  size);
}

static inline void
sn_stdafree(sna *a snunused, void *ptr) {
	assert(ptr != NULL);
	free(ptr);
}

snaif sn_stda =
{
	.open    = sn_stdaopen,
	.close   = sn_stdaclose,
	.malloc  = sn_stdamalloc,
	.realloc = sn_stdarealloc,
	.free    = sn_stdafree 
};
