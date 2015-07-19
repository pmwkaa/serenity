
/*
 * serenity database
 *
 * Copyright (c) Dmitry Simonenko
 * BSD License
*/

#include <serenity.h>

int sn_loopinit(snloop *l, void *arg)
{
	sn_listinit(&l->handles);
	l->arg = arg;
	return 0;
}

int sn_loopfree(snloop *l)
{
	int rc = 0;
	int rcret = 0;
	snlist *i, *n;
	sn_listforeach_safe(&l->handles, i, n) {
		snhandle *h = sncast(i, snhandle, link);
		rc = h->shutdown(l, h);
		if (snunlikely(rc == -1))
			rcret = -1;
	}
	return rcret;
}

int sn_loopregister(snloop *l, snhandle *h)
{
	sn_listappend(&l->handles, &h->link);
	return 0;
}

int sn_tick(snloop *l)
{
	snlist *i, *n;
	sn_listforeach_safe(&l->handles, i, n) {
		snhandle *h = sncast(i, snhandle, link);
		int rc = h->tick(l, h);
		if (snunlikely(rc == -1))
			return -1;
	}
	return 0;
}
