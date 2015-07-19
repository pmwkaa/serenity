#ifndef SN_BUF_H_
#define SN_BUF_H_

/*
 * serenity database
 *
 * Copyright (c) Dmitry Simonenko
 * BSD License
*/

typedef struct snbuf snbuf;

struct snbuf {
	char *reserve;
	char *s, *p, *e;
};

static inline void
sn_bufinit(snbuf *b)
{
	b->reserve = NULL;
	b->s = NULL;
	b->p = NULL;
	b->e = NULL;
}

static inline void
sn_bufinit_reserve(snbuf *b, void *buf, int size)
{
	b->reserve = buf;
	b->s = buf;
	b->p = b->s; 
	b->e = b->s + size;
}

static inline void
sn_buffree(snbuf *b)
{
	if (snunlikely(b->s == NULL))
		return;
	if (snunlikely(b->s != b->reserve))
		sn_free(b->s);
	b->s = NULL;
	b->p = NULL;
	b->e = NULL;
}

static inline void
sn_bufreset(snbuf *b) {
	b->p = b->s;
}

static inline int
sn_bufsize(snbuf *b) {
	return b->e - b->s;
}

static inline int
sn_bufused(snbuf *b) {
	return b->p - b->s;
}

static inline int
sn_bufunused(snbuf *b) {
	return b->e - b->p;
}

static inline int
sn_bufensure(snbuf *b, int size)
{
	if (snlikely(b->e - b->p >= size))
		return 0;
	int sz = sn_bufsize(b) * 2;
	int actual = sn_bufused(b) + size;
	if (snunlikely(actual > sz))
		sz = actual;
	char *p;
	if (snunlikely(b->s == b->reserve)) {
		p = sn_malloc(sz);
		if (snunlikely(p == NULL))
			return -1;
		memcpy(p, b->s, sn_bufused(b));
	} else {
		p = sn_realloc(b->s, sz);
		if (snunlikely(p == NULL))
			return -1;
	}
	b->p = p + (b->p - b->s);
	b->e = p + sz;
	b->s = p;
	assert((b->e - b->p) >= size);
	return 0;
}

static inline int
sn_buftruncate(snbuf *b, int size)
{
	assert(size <= (b->p - b->s));
	char *p = b->reserve;
	if (b->s != b->reserve) {
		p = sn_realloc(b->s, size);
		if (snunlikely(p == NULL))
			return -1;
	}
	b->p = p + (b->p - b->s);
	b->e = p + size;
	b->s = p;
	return 0;
}

static inline void
sn_bufadvance(snbuf *b, int size)
{
	b->p += size;
}

static inline int
sn_bufadd(snbuf *b, void *buf, int size)
{
	int rc = sn_bufensure(b, size);
	if (snunlikely(rc == -1))
		return -1;
	memcpy(b->p, buf, size);
	sn_bufadvance(b, size);
	return 0;
}

static inline int
sn_bufin(snbuf *b, void *v) {
	assert(b->s != NULL);
	return (char*)v >= b->s && (char*)v < b->p;
}

static inline void*
sn_bufat(snbuf *b, int size, int i) {
	return b->s + size * i;
}

static inline void
sn_bufset(snbuf *b, int size, int i, char *buf, int bufsize)
{
	assert(b->s + (size * i + bufsize) <= b->p);
	memcpy(b->s + size * i, buf, bufsize);
}

static inline void
sn_bufmove(snbuf *b, char *p)
{
	assert(p <= b->p);
	int to_move = b->p - p;
	int to_add  = p - b->s;
	memmove(b->s, p, to_move);
	b->p -= to_add;
}

#endif
