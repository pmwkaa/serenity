#ifndef SN_EPOLL_H_
#define SN_EPOLL_H_

/*
 * serenity database
 *
 * Copyright (c) Dmitry Simonenko
 * BSD License
*/

typedef struct snepoll snepoll;

struct snepoll {
	snhandle h;
	struct epoll_event *e;
	int size;
	int count;
	int fd;
	snlist list;
	snloop *loop;
};

snhandle *sn_epoll(snloop*);
int sn_epoll_add(snhandle*, snloopfd*, int);
int sn_epoll_modify(snhandle*, snloopfd*, int);
int sn_epoll_delete(snhandle*, snloopfd*);

#endif
