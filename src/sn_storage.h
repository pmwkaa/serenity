#ifndef SN_STORAGE_H_
#define SN_STORAGE_H_

/*
 * serenity database
 *
 * Copyright (c) Dmitry Simonenko
 * BSD License
*/

typedef struct snstorage snstorage;
typedef struct sndb sndb;

struct sndb {
	int   id;
	char *name;
	char *key;
	char *path;
	void *db;
	snconfigkey keytype;
};

struct snstorage {
	void *env;
	sndb *storage;
	int   count;
};

int   sn_storageopen(snstorage*, snconfig*, snlog*);
int   sn_storageclose(snstorage*);
int   sn_storageinfo(snstorage*, snbuf*);
char *sn_storageerror(snstorage*);
int   sn_storagematch(snstorage*, char*, int);

static inline void*
sn_dbof(snstorage *s, int id)
{
	assert(id < s->count);
	return s->storage[id].db;
}

static inline sndb*
sn_databaseof(snstorage *s, int id)
{
	assert(id < s->count);
	return &s->storage[id];
}

#endif
