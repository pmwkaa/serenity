
/*
 * serenity database
 *
 * Copyright (c) Dmitry Simonenko
 * BSD License
*/

#include <serenity.h>

void sn_cursorlist_init(sncursorlist *l)
{
	l->seq = 0;
	l->count = 0;
	sn_listinit(&l->list);
}

void sn_cursorlist_free(sncursorlist *l)
{
	snlist *i, *n;
	sn_listforeach_safe(&l->list, i, n) {
		sncursor *c = sncast(i, sncursor, link);
		sn_cursorfree(l, c);
	}
}

void sn_cursorfree(sncursorlist *l, sncursor *c)
{
	if (c->c)
		sp_destroy(c->c);
	sn_listunlink(&c->link);
	l->count--;
	sn_free(c);
}

void sn_cursorattach(sncursorlist *l, sncursor *c)
{
	sn_listappend(&l->list, &c->link);
	l->count++;
}

sncursor *sn_cursornew(sncursorlist *l)
{
	sncursor *c = sn_malloc(sizeof(sncursor));
	if (snunlikely(c == NULL))
		return NULL;
	c->id = ++l->seq;
	c->c  = NULL;
	sn_listinit(&c->link);
	return c;
}

sncursor *sn_cursormatch(sncursorlist *l, int id)
{
	snlist *i;
	sn_listforeach(&l->list, i) {
		sncursor *c = sncast(i, sncursor, link);
		if (c->id == id)
			return c;
	}
	return NULL;
}
