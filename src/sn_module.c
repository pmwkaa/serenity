
/*
 * serenity database
 *
 * Copyright (c) Dmitry Simonenko
 * BSD License
*/

#include <serenity.h>

int sn_moduleinit(snmodules *m)
{
	sn_listinit(&m->list);
	m->count = 0;
	return 0;
}

int sn_moduleshutdown(snmodules *m)
{
	snlist *i, *n;
	sn_listforeach_safe(&m->list, i, n) {
		snmodule *p = sncast(i, snmodule, link);
		if (p->ctl)
			p->ctl(1);
		if (p->path)
			sn_free(p->path);
		if (p->h)
			dlclose(p->h);
		sn_free(p);
	}
	return 0;
}

int sn_moduleadd(snmodules *m, char *path)
{
	snmodule *n = sn_malloc(sizeof(snmodule));
	if (snunlikely(n == NULL))
		return -1;
	n->path = sn_strdup(path);
	if (snunlikely(n->path == NULL)) {
		sn_free(n);
		return -1;
	}
	n->h   = NULL;
	n->ctl = NULL;
	sn_listinit(&n->link);
	sn_listappend(&m->list, &n->link);
	m->count++;
	return 0;
}

int sn_moduleopen(snmodules *m, snlog *l)
{
	snlist *i, *n;
	sn_listforeach_safe(&m->list, i, n) {
		snmodule *p = sncast(i, snmodule, link);
		p->h = dlopen(p->path, RTLD_NOW);
		if (p->h == NULL) {
			char *error = dlerror();
			sn_log(l, "error: failed to open module '%s': %s", p->path, error);
			return -1;
		}
		p->ctl = (snmodulef)(uintptr_t)dlsym(p->h, "severity_module");
		if (p->ctl == NULL) {
			sn_log(l, "error: bad severity module '%s'", p->path);
			dlclose(p->h);
			p->h = NULL;
			return -1;
		}
		int rc = p->ctl(0);
		if (rc == -1) {
			char *error = dlerror();
			sn_log(l, "error: failed to init module '%s': %s", p->path, error);
			dlclose(p->h);
			p->h = NULL;
			return -1;
		}
		sn_log(l, "loading module '%s'", p->path);
	}
	if (m->count)
		sn_log(l, "");
	return 0;
}
