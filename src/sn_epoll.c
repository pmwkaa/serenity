
/*
 * serenity database
 *
 * Copyright (c) Dmitry Simonenko
 * BSD License
*/

#include <serenity.h>

static int
sn_epoll_shutdown(snloop *l snunused, snhandle *h)
{
	snepoll *e = (snepoll*)h;
	if (e->fd != -1) {
		close(e->fd);
		e->fd = -1;
	}
	if (e->e) {
		sn_free(e->e);
		e->e = NULL;
	}
	sn_free(h);
	return 0;
}

static int
sn_epoll_tick(snloop *l snunused, snhandle *h)
{
	snepoll *e = (snepoll*)h;
	int timeout = 1000; /* 1sec */
	int count = epoll_wait(e->fd, e->e, e->count, timeout);
	if (count <= 0)
		return 0;
	int i = 0;
	while (i < count) {
		struct epoll_event *ev = e->e + i;
		snloopfd *o = ev->data.ptr;
		int events = 0;
		if (ev->events & EPOLLIN) {
			events = SN_LOOPFD_R;
		}
		if (ev->events & EPOLLOUT ||
			ev->events & EPOLLERR ||
			ev->events & EPOLLHUP) {
			events |= SN_LOOPFD_W;
		}
		o->callback(o, events);
		i++;
	}
	return 0;
}

snhandle *sn_epoll(snloop *l)
{
	snepoll *e = sn_malloc(sizeof(snepoll));
	if (snunlikely(e == NULL))
		return NULL;
	e->loop = l;
	e->h.shutdown = sn_epoll_shutdown;
	e->h.tick = sn_epoll_tick;
	sn_listinit(&e->h.link);
	e->count = 0;
	e->size = 1024;
	e->e = sn_malloc(sizeof(struct epoll_event) * e->size);
	if (snunlikely(e->e == NULL)) {
		sn_free(e);
		return NULL;
	}
	e->fd = epoll_create(e->size);
	if (snunlikely(e->fd == -1)) {
		sn_free(e->e);
		sn_free(e);
		return NULL;
	}
	sn_listinit(&e->list);
	return &e->h;
}

int sn_epoll_add(snhandle *h, snloopfd *o, int type)
{
	snepoll *e = (snepoll*)h;
	if ((e->count + 1) > e->size) {
		int size = e->size * 2;
		void *ptr = sn_realloc(e->e, sizeof(struct epoll_event) * size);
		if (snunlikely(ptr == NULL))
			return -1;
		sn_free(e->e);
		e->e = ptr;
		e->size = size;
	}
	struct epoll_event ev;
	ev.events = 0;
	o->type = type;
	if (o->type & SN_LOOPFD_R)
		ev.events |= EPOLLIN;
	if (o->type & SN_LOOPFD_W)
		ev.events |= EPOLLOUT;
	ev.data.ptr = o;
	int rc = epoll_ctl(e->fd, EPOLL_CTL_ADD, o->fd, &ev);
	if (snunlikely(rc == -1))
		return -1;
	e->count++;
	sn_listappend(&e->list, &o->link);
	return 0;
}

int sn_epoll_modify(snhandle *h, snloopfd *o, int type)
{
	snepoll *e = (snepoll*)h;
	struct epoll_event ev;
	ev.events = 0;
	if (type & SN_LOOPFD_R)
		ev.events |= EPOLLIN;
	if (type & SN_LOOPFD_W)
		ev.events |= EPOLLOUT;
	ev.data.ptr = o;
	int rc = epoll_ctl(e->fd, EPOLL_CTL_MOD, o->fd, &ev);
	if (snunlikely(rc == -1))
		return -1;
	o->type = type;
	sn_listunlink(&o->link);
	return 0;
}

int sn_epoll_delete(snhandle *h, snloopfd *o)
{
	snepoll *e = (snepoll*)h;
	struct epoll_event ev;
	ev.events = 0;
	if (o->type & SN_LOOPFD_R)
		ev.events |= EPOLLIN;
	if (o->type & SN_LOOPFD_W)
		ev.events |= EPOLLOUT;
	ev.data.ptr = o;
	int rc = epoll_ctl(e->fd, EPOLL_CTL_DEL, o->fd, &ev);
	if (snunlikely(rc == -1))
		return -1;
	e->count--;
	sn_listunlink(&o->link);
	return 0;
}
