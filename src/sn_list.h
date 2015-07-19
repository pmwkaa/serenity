#ifndef SN_LIST_H_
#define SN_LIST_H_

/*
 * serenity database
 *
 * Copyright (c) Dmitry Simonenko
 * BSD License
*/

typedef struct snlist snlist;

struct snlist {
	snlist *next, *prev;
};

static inline void
sn_listinit(snlist *h) {
	h->next = h->prev = h;
}

static inline void
sn_listappend(snlist *h, snlist *n) {
	n->next = h;
	n->prev = h->prev;
	n->prev->next = n;
	n->next->prev = n;
}

static inline void
sn_listunlink(snlist *n) {
	n->prev->next = n->next;
	n->next->prev = n->prev;
}

static inline void
sn_listpush(snlist *h, snlist *n) {
	n->next = h->next;
	n->prev = h;
	n->prev->next = n;
	n->next->prev = n;
}

static inline snlist*
sn_listpop(snlist *h) {
	register snlist *pop = h->next;
	sn_listunlink(pop);
	return pop;
}

static inline int
sn_listempty(snlist *l) {
	return l->next == l && l->prev == l;
}

static inline void
sn_listmerge(snlist *a, snlist *b) {
	if (snunlikely(sn_listempty(b)))
		return;
	register snlist *first = b->next;
	register snlist *last = b->prev;
	first->prev = a->prev;
	a->prev->next = first;
	last->next = a;
	a->prev = last;
}

static inline void
sn_listreplace(snlist *o, snlist *n) {
	n->next = o->next;
	n->next->prev = n;
	n->prev = o->prev;
	n->prev->next = n;
}

#define sn_listlast(H, N) ((H) == (N))

#define sn_listforeach(H, I) \
	for (I = (H)->next; I != H; I = (I)->next)

#define sn_listforeach_continue(H, I) \
	for (; I != H; I = (I)->next)

#define sn_listforeach_safe(H, I, N) \
	for (I = (H)->next; I != H && (N = I->next); I = N)

#define sn_listforeach_reverse(H, I) \
	for (I = (H)->prev; I != H; I = (I)->prev)

#endif
