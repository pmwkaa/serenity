#ifndef SN_LOOPFD_H_
#define SN_LOOPFD_H_

/*
 * serenity database
 *
 * Copyright (c) Dmitry Simonenko
 * BSD License
*/

typedef struct snloopfd snloopfd;

enum {
	SN_LOOPFD_R = 1,
	SN_LOOPFD_W = 2
};

typedef int (*snloopfdf)(snloopfd*, int);

struct snloopfd {
	int type;
	int fd;
	snloopfdf callback;
	void *arg;
	snlist link;
};

static inline void
sn_loopfd_init(snloopfd *f, int fd, snloopfdf callback, void *arg)
{
	f->type = 0;
	f->fd = fd;
	f->callback = callback;
	f->arg = arg;
	sn_listinit(&f->link);
}

#endif
