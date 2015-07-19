#ifndef SN_CLIENTLIST_H_
#define SN_CLIENTLIST_H_

/*
 * serenity database
 *
 * Copyright (c) Dmitry Simonenko
 * BSD License
*/

typedef struct snclientlist snclientlist;

struct snclientlist {
	snlist list;
	int count;
};

static inline void
sn_clientlist_init(snclientlist *l)
{
	sn_listinit(&l->list);
	l->count = 0;
}

static inline void
sn_clientlist_shutdown(snclientlist *l)
{
	snlist *i, *n;
	sn_listforeach_safe(&l->list, i, n) {
		snclient *c = sncast(i, snclient, link);
		sn_clientclose(c);
		sn_clientfree(c);
	}
}

static inline void
sn_clientlist_add(snclientlist *l, snclient *c)
{
	sn_listappend(&l->list, &c->link);
	l->count++;
}

static inline void
sn_clientlist_delete(snclientlist *l, snclient *c)
{
	sn_listunlink(&c->link);
	l->count--;
}

#endif
