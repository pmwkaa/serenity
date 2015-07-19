
/*
 * serenity database
 *
 * Copyright (c) Dmitry Simonenko
 * BSD License
*/

#include <serenity.h>

static void
sn_storagerecover_cb(char *trace, void *arg)
{
	snlog *l = arg;
	sn_log(l, trace);
}

static int
sn_storagescheme(snstorage *s, snconfig *c, snlog *l)
{
	if (c->databases == 0) {
		sn_log(l, "no databases to use");
		return -1;
	}
	int size = c->databases * sizeof(sndb);
	s->count = c->databases;
	s->storage = sn_malloc(size);
	if (snunlikely(s->storage == NULL))
		return -1;
	memset(s->storage, 0, size);
	int i = 0;
	while (i < c->databases) {
		sndb *db = &s->storage[i];
		snconfigdb *scheme = sn_configscheme_match(c, i);
		char  namesz[32];
		char *name = NULL;
		char *path = NULL;
		char *key  = NULL;
		if (scheme) {
			name = scheme->name;
			path = scheme->path;
			key  = scheme->key;
		}
		if (name == NULL) {
			snprintf(namesz, sizeof(namesz), "db_%d", i);
			name = namesz;
		}
		if (key == NULL) {
			key = "string";
		}
		db->id   = i;
		db->db   = NULL;
		db->name = sn_strdup(name);
		if (snunlikely(db->name == NULL))
			return -1;
		db->key = sn_strdup(key);
		if (snunlikely(db->key == NULL))
			return -1;
		if (scheme)
			db->keytype = scheme->keytype;
		else
			db->keytype = SN_KEYSTRING;
		db->path = NULL;
		if (path) {
			db->path = sn_strdup(path);
			if (snunlikely(db->path == NULL))
				return -1;
		}
		i++;
	}
	return 0;
}

static void
sn_storagescheme_print(snstorage *s, snlog *l)
{
	int i = 0;
	while (i < s->count) {
		sndb *db = &s->storage[i];
		char desc[1024];
		int len = snprintf(desc, sizeof(desc), "db %d name %s primary_key %s",
		                   db->id, db->name, db->key);
		if (db->path) {
			len += snprintf(desc + len, sizeof(desc) - len, " path %s",
			                db->path);
		}
		sn_log(l, "%s", desc);
		i++;
	}
	sn_log(l, "");
}

int sn_storageopen(snstorage *s, snconfig *c, snlog *l)
{
	int rc;
	rc = sn_storagescheme(s, c, l);
	if (snunlikely(rc == -1))
		return -1;
	sn_storagescheme_print(s, l);
	s->env = sp_env();
	if (s->env == NULL)
		return -1;
	sp_setstring(s->env, "sophia.path", c->dir, 0);
	sp_setint(s->env, "memory.limit", c->maxmemory);
	sp_setstring(s->env, "scheduler.on_recover",
	             (void*)(uintptr_t)sn_storagerecover_cb, 0);
	sp_setstring(s->env, "scheduler.on_recover_arg", l, 0);
	int i = 0;
	while (i < s->count) {
		sndb *db = &s->storage[i];
		sp_setstring(s->env, "db", db->name, 0);
		char path[128];
		snprintf(path, sizeof(path), "db.%s.mmap", db->name);
		sp_setint(s->env, path, 1);
		if (db->path) {
			snprintf(path, sizeof(path), "db.%s.path", db->name);
			sp_setstring(s->env, path, db->path, 0);
		}
		if (db->key) {
			snprintf(path, sizeof(path), "db.%s.index.key", db->name);
			sp_setstring(s->env, path, db->key, 0);
		}
		i++;
	}
	rc = sp_open(s->env);
	if (snunlikely(rc == -1)) {
		char *error = sn_storageerror(s);
		if (error) {
			sn_log(l, "storage error: %s", error);
			free(error);
		}
		return -1;
	}
	i = 0;
	while (i < s->count) {
		sndb *db = &s->storage[i];
		char path[128];
		snprintf(path, sizeof(path), "db.%s", db->name);
		db->db = sp_getobject(s->env, path);
		if (snunlikely(db->db == NULL)) {
			sn_log(l, "failed to setupt '%s' database", db->name);
			return -1;
		}
		i++;
	}
	return 0;
}

int sn_storageclose(snstorage *s)
{
	if (s->env == NULL)
		return 0;
	int rcret = sp_destroy(s->env);
	int i = 0;
	while (i < s->count) {
		sndb *db = &s->storage[i];
		if (db->name)
			sn_free(db->name);
		if (db->path)
			sn_free(db->path);
		if (db->key)
			sn_free(db->key);
		i++;
	}
	if (s->storage)
		sn_free(s->storage);
	return rcret;
}

char *sn_storageerror(snstorage *s)
{
	assert(s->env != NULL );
	return sp_getstring(s->env, "sophia.error", NULL);
}

int sn_storageinfo(snstorage *s, snbuf *info)
{
	int rc = 0;
	assert(s->env != NULL );
	char buf[1024];
	void *c = sp_cursor(s->env, NULL);
	void *o;
	while ((o = sp_get(c, NULL)))
	{
		int keysize = 0;
		int valuesize = 0;
		char *key = sp_getstring(o, "key", &keysize);
		char *value = sp_getstring(o, "value", &valuesize);
		int len = snprintf(buf, sizeof(buf), "%s = %s\n",
				key, (value) ? value : "");
		rc = sn_bufadd(info, buf, len);
		if (snunlikely(rc == -1))
			break;
		sp_destroy(o);
	}
	sp_destroy(c);
	return rc;
}

int sn_storagematch(snstorage *s, char *name, int size)
{
	int i = 0;
	while (i < s->count) {
		sndb *db = &s->storage[i];
		if (strncasecmp(db->name, name, size) == 0)
			return db->id;
		i++;
	}
	return -1;
}
