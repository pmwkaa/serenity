#ifndef SN_MODULE_H_
#define SN_MODULE_H_

/*
 * serenity database
 *
 * Copyright (c) Dmitry Simonenko
 * BSD License
*/

typedef struct snmodules snmodules;
typedef struct snmodule snmodule;

typedef int (*snmodulef)(int);

struct snmodule {
	char *path;
	void *h;
	snmodulef ctl;
	snlist link;
};

struct snmodules {
	snlist list;
	int count;
};

int sn_moduleinit(snmodules*);
int sn_moduleshutdown(snmodules*);
int sn_moduleadd(snmodules*, char*);
int sn_moduleopen(snmodules*, snlog*);

#endif
