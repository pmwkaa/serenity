#ifndef SN_PROC_H_
#define SN_PROC_H_

/*
 * serenity database
 *
 * Copyright (c) Dmitry Simonenko
 * BSD License
*/

typedef struct snprocs snprocs;
typedef struct snproc snproc;

typedef int (*snprocf)(snclient*, char*, int);

struct snproc {
	char *name;
	snprocf f;
	snlist link;
};

struct snprocs {
	snlist list;
	int count;
};

int sn_procinit(snprocs*);
int sn_procfree(snprocs*);
int sn_procadd(snprocs*, char*, snprocf);
snproc *sn_procmatch(snprocs*, char*, int);

#endif
