
/*
 * serenity database
 *
 * Copyright (c) Dmitry Simonenko
 * BSD License
*/

#include <serenity.h>

static inline char*
sn_keyof(sndb *db, char *key, int keysize, int *rsize)
{
	if (snlikely(db->keytype == SN_KEYSTRING)) {
		*rsize = keysize;
		return key;
	}
	static uint32_t u32;
	static uint64_t u64;
	switch (db->keytype) {
	case SN_KEYU32:
		u32 = sn_decode_number(key, key + keysize);
		*rsize = sizeof(uint32_t);
		return (char*)&u32;
	case SN_KEYU64:
		u64 = sn_decode_number(key, key + keysize);
		*rsize = sizeof(uint64_t);
		return (char*)&u64;
	default: break;
	}
	assert(0);
	return NULL;
}

int sn_execute(sn *s, snclient *c, snrequest *r)
{
	char *error;
	void *o;
	sndb *db = sn_databaseof(&s->storage, c->db);
	void *database = db->db;
	void *dest = database;
	if (c->transaction)
		dest = c->transaction;

	int keysize;
	char *key;
	int valuesize;
	char *value;

	int rc;
	switch (r->cmd) {
	case SN_SET: {
		key = sn_keyof(db, r->key, r->keysize, &keysize);
		o = sp_object(database);
		sp_setstring(o, "key", key, keysize);
		sp_setstring(o, "value", r->value, r->valuesize);
		rc = sp_set(dest, o);
		if (snunlikely(rc == -1)) {
			error = sn_storageerror(s->storage.env);
			sn_encode_errorf(&c->result, "STORAGE ERROR %s", error);
			free(error);
			return -1;
		}
		return sn_encode_stringf(&c->result, "OK");
	}
	case SN_DEL: {
		key = sn_keyof(db, r->key, r->keysize, &keysize);
		void *o = sp_object(database);
		sp_setstring(o, "key", key, keysize);
		rc = sp_delete(dest, o);
		if (snunlikely(rc == -1)) {
			error = sn_storageerror(s->storage.env);
			sn_encode_errorf(&c->result, "STORAGE ERROR %s", error);
			free(error);
			return -1;
		}
		return sn_encode_stringf(&c->result, "OK");
	}
	case SN_GET: {
		key = sn_keyof(db, r->key, r->keysize, &keysize);
		o = sp_object(database);
		sp_setstring(o, "key", key, keysize);
		o = sp_get(dest, o);
		if (o == NULL)
			return sn_encode_nil(&c->result);
		value = sp_getstring(o, "value", &valuesize);
		rc = sn_encode_bin(&c->result, value, valuesize);
		sp_destroy(o);
		return rc;
	}
	case SN_EXISTS: {
		key = sn_keyof(db, r->key, r->keysize, &keysize);
		o = sp_object(database);
		sp_setstring(o, "key", key, keysize);
		o = sp_get(dest, o);
		if (o == NULL)
			return sn_encode_int(&c->result, 0);
		sp_destroy(o);
		return sn_encode_int(&c->result, 1);
	}
	case SN_CURSOR: {
		sncursor *cur = sn_cursornew(&c->cursors);
		if (snunlikely(cur == NULL))
			return sn_encode_errorf(&c->result, "ERROR out-of-memory");
		o = sp_object(database);
		sp_setstring(o, "order", r->key, r->keysize);
		if (r->value) {
			key = sn_keyof(db, r->value, r->valuesize, &keysize);
			sp_setstring(o, "key", key, keysize);
		}
		cur->c = sp_cursor(database, o);
		if (snunlikely(cur->c == NULL)) {
			sn_cursorfree(&c->cursors, cur);
			error = sn_storageerror(s->storage.env);
			sn_encode_errorf(&c->result, "ERROR failed to create cursor %s", error);
			free(error);
			return -1;
		}
		sn_cursorattach(&c->cursors, cur);
		return sn_encode_int(&c->result, cur->id);
	}
	case SN_NEXT: {
		int id = sn_decode_number(r->key, r->key + r->keysize);
		sncursor *cur = sn_cursormatch(&c->cursors, id);
		if (snunlikely(cur == NULL))
			return sn_encode_errorf(&c->result, "ERROR failed to match cursor '%d'", id);
		void *o = sp_get(cur->c, NULL);
		if (o == NULL) {
			sn_cursorfree(&c->cursors, cur);
			return sn_encode_nil(&c->result);
		}
		rc = sn_encode_array(&c->result, 2);
		int keysize;
		int valuesize;
		char *key = sp_getstring(o, "key", &keysize);
		char *value = sp_getstring(o, "value", &valuesize);
		char keysz[32];
		if (db->keytype == SN_KEYU32) {
			keysize = snprintf(keysz, sizeof(keysz), "%"PRIu32, *(uint32_t*)key);
			rc = sn_encode_bin(&c->result, keysz, keysize);
		} else
		if (db->keytype == SN_KEYU64) {
			keysize = snprintf(keysz, sizeof(keysz), "%"PRIu64, *(uint64_t*)key);
			rc = sn_encode_bin(&c->result, keysz, keysize);
		} else {
			rc = sn_encode_bin(&c->result, key, keysize);
		}
		rc = sn_encode_bin(&c->result, value, valuesize);
		sp_destroy(o);
		return 0;
	}
	case SN_CLOSE: {
		int id = sn_decode_number(r->key, r->key + r->keysize);
		sncursor *cur = sn_cursormatch(&c->cursors, id);
		if (snunlikely(cur == NULL))
			return sn_encode_errorf(&c->result, "ERROR failed to match cursor '%d'", id);
		sn_cursorfree(&c->cursors, cur);
		return sn_encode_stringf(&c->result, "OK");
	}
	case SN_SELECT: {
		if (snunlikely(r->keysize == 0))
			return sn_encode_errorf(&c->result, "ERROR bad select argument");
		int id = 0;
		if (! isdigit(r->key[0])) {
			id = sn_storagematch(&s->storage, r->key, r->keysize);
			if (snunlikely(id == -1)) {
				return sn_encode_errorf(&c->result, "ERROR database '%.*s' not found",
				                        r->keysize, r->key);
			}
		} else {
			id = sn_decode_number(r->key, r->key + r->keysize);
		}
		if (snunlikely(id >= s->storage.count))
			return sn_encode_errorf(&c->result, "ERROR database does not exists (databases %d)",
			                        s->storage.count);
		c->db = id;
		return sn_encode_stringf(&c->result, "OK");
	}
	case SN_BEGIN: {
		if (c->transaction)
			return sn_encode_errorf(&c->result, "ERROR transaction is in progress");
		c->transaction = sp_begin(s->storage.env);
		if (c->transaction == NULL) {
			error = sn_storageerror(s->storage.env);
			sn_encode_errorf(&c->result, "STORAGE ERROR %s", error);
			free(error);
			return -1;
		}
		return sn_encode_stringf(&c->result, "OK");
	}
	case SN_COMMIT: {
		if (c->transaction == NULL)
			return sn_encode_errorf(&c->result, "ERROR not in transaction");
		rc = sp_commit(c->transaction);
		switch (rc) {
		case -1:
			error = sn_storageerror(s->storage.env);
			sn_encode_errorf(&c->result, "STORAGE ERROR %s", error);
			free(error);
			c->transaction = NULL;
			return -1;
		case  2:
			sp_destroy(c->transaction);
		case  1:
			c->transaction = NULL;
			return sn_encode_errorf(&c->result, "ERROR rollback by conflict");
		}
		c->transaction = NULL;
		return sn_encode_stringf(&c->result, "OK");
	}
	case SN_ROLLBACK: {
		if (c->transaction == NULL)
			return sn_encode_errorf(&c->result, "ERROR not in transaction");
		rc = sp_destroy(c->transaction);
		switch (rc) {
		case -1:
			error = sn_storageerror(s->storage.env);
			sn_encode_errorf(&c->result, "STORAGE ERROR %s", error);
			free(error);
			c->transaction = NULL;
			return -1;
		}
		c->transaction = NULL;
		return sn_encode_stringf(&c->result, "OK");
	}
	case SN_CALL: {
		snproc *proc = sn_procmatch(&s->procs, r->key, r->keysize);
		if (proc == NULL) {
			sn_encode_errorf(&c->result, "unknown procedure '%.*s'",
			                 r->keysize, r->key);
			return -1;
		}
		return proc->f(c, r->value, r->valuesize);
	}
	case SN_PING:
		return sn_encode_stringf(&c->result, "PONG");
	case SN_INFO: {
		snbuf info;
		sn_bufinit(&info);
		char desc[] = "# Storage info\n";
		sn_bufadd(&info, desc, sizeof(desc) - 1);
		sn_storageinfo(&s->storage, &info);
		rc = sn_encode_bin(&c->result, info.s, sn_bufused(&info));
		sn_buffree(&info);
		return rc;
	}
	case SN_SHUTDOWN:
		sn_stop(s);
		return sn_encode_stringf(&c->result, "OK");
	}
	return sn_encode_errorf(&c->result, "ERROR bad request");
}
