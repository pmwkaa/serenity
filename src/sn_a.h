#ifndef SN_A_H_
#define SN_A_H_

/*
 * serenity database
 *
 * Copyright (c) Dmitry Simonenko
 * BSD License
*/

static inline void*
sn_malloc(int size) {
	return malloc(size);
}

static inline void*
sn_realloc(void *ptr, int size) {
	return realloc(ptr, size);
}

static inline void
sn_free(void *ptr) {
	free(ptr);
}

static inline char*
sn_strdup(char *str) {
	int sz = strlen(str) + 1;
	char *s = sn_malloc(sz);
	if (snunlikely(s == NULL))
		return NULL;
	memcpy(s, str, sz);
	return s;
}

static inline char*
sn_memdup(void *ptr, size_t size) {
	char *s = sn_malloc(size);
	if (snunlikely(s == NULL))
		return NULL;
	memcpy(s, ptr, size);
	return s;
}

#endif
