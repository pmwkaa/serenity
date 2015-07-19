#ifndef SN_CONFIG_H_
#define SN_CONFIG_H_

/*
 * serenity database
 *
 * Copyright (c) Dmitry Simonenko
 * BSD License
*/

typedef struct snconfigdb snconfigdb;
typedef struct snconfig snconfig;

typedef enum {
	SN_KEYUNDEF,
	SN_KEYSTRING,
	SN_KEYU32,
	SN_KEYU64
} snconfigkey;

struct snconfigdb {
	int          id;
	char        *path;
	char        *name;
	char        *key;
	snconfigkey  keytype;
	snlist       link;
};

struct snconfig {
	char    *config;
	char    *dir;
	char    *logfile;
	char    *pidfile;
	char    *bind;
	int      port;
	int      databases;
	snlist   scheme;
	int      daemonize;
	uint64_t maxmemory;
};

int sn_configinit(snconfig*);
int sn_configfree(snconfig*);
int sn_configopen(snconfig*, snlog*, snmodules*, char*);
int sn_configshow(snconfig*, snlog*);

snconfigdb*
sn_configscheme_match(snconfig*, int);

#endif
