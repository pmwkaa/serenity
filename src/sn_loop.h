#ifndef SN_LOOP_H_
#define SN_LOOP_H_

/*
 * serenity database
 *
 * Copyright (c) Dmitry Simonenko
 * BSD License
*/

typedef struct snhandle snhandle;
typedef struct snloop snloop;

struct snhandle {
	int (*shutdown)(snloop*, snhandle*);
	int (*tick)(snloop*, snhandle*);
	snlist link;
};

struct snloop {
	snlist handles;
	void *arg;
};

int sn_loopinit(snloop*, void*);
int sn_loopfree(snloop*);
int sn_loopregister(snloop*, snhandle*);
int sn_tick(snloop*);

#endif
