
/*
 * serenity database
 *
 * Copyright (c) Dmitry Simonenko
 * BSD License
*/

#include <serenity.h>

int sn_procinit(snprocs *l)
{
	sn_listinit(&l->list);
	l->count = 0;
	return 0;
}

int sn_procfree(snprocs *l)
{
	snlist *i, *n;
	sn_listforeach_safe(&l->list, i, n) {
		snproc *p = sncast(i, snproc, link);
		sn_free(p->name);
		sn_free(p);
	}
	return 0;
}

int sn_procadd(snprocs *l, char *name, snprocf f)
{
	int len = strlen(name);
	snproc *p = sn_procmatch(l, name, len);
	if (p) {
		return -1;
	}
	p = sn_malloc(sizeof(snproc));
	if (p == NULL)
		return -1;
	p->name = sn_strdup(name);
	if (snunlikely(p->name == NULL)) {
		sn_free(p);
		return -1;
	}
	p->f = f;
	sn_listinit(&p->link);
	sn_listappend(&l->list, &p->link);
	l->count++;
	return 0;
}

snproc*
sn_procmatch(snprocs *l, char *name, int size)
{
	snlist *i;
	sn_listforeach(&l->list, i) {
		snproc *p = sncast(i, snproc, link);
		if (strncasecmp(p->name, name, size) == 0)
			return p;
	}
	return NULL;
}
