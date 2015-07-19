#ifndef SN_H_
#define SN_H_

/*
 * serenity database
 *
 * Copyright (c) Dmitry Simonenko
 * BSD License
*/

typedef struct sn sn;

struct sn {
	snconfig conf;
	snlog log;
	snmodules modules;
	snprocs procs;
	snloop loop;
	snhandle *epoll;
	snserver server;
	snstorage storage;
	int active;
};

int sn_init(sn*, char*);
int sn_open(sn*);
int sn_shutdown(sn*);
int sn_start(sn*);
int sn_stop(sn*);

#endif
