#ifndef SN_SERVER_H_
#define SN_SERVER_H_

/*
 * serenity database
 *
 * Copyright (c) Dmitry Simonenko
 * BSD License
*/

typedef struct snserver snserver;

typedef int (*sn_onconnectf)(snserver*, snclient*);
typedef int (*sn_onreadf)(snserver*, snclient*);
typedef int (*sn_onwritef)(snserver*, snclient*);

struct snserver {
	snloopfd server;
	int fd;
	snloop *loop;
	snhandle *epoll;
	snclientlist cl;
	sn_onconnectf on_connect;
	sn_onreadf on_read;
	sn_onwritef on_write;
	snlog *log;
	void *ptr;
};

int sn_serverinit(snserver*, snlog*, snloop*, snhandle*,
                  sn_onconnectf,
                  sn_onreadf, sn_onwritef,
                  void*);
int sn_serveropen(snserver*, char*, int);
int sn_servershutdown(snserver*);
int sn_serverdelete(snserver*, snclient*);

#endif
