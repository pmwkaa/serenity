
/*
 * serenity database
 *
 * Copyright (c) Dmitry Simonenko
 * BSD License
*/

#include <serenity.h>

static inline int
sn_request_str(char **p, char *end, char **result)
{
	int c = sn_decode_next(p, end);
	switch (c) {
	case SN_PERROR: return -1;
	case SN_PINCOMPLETE: return 0;
	case SN_STRING:
		c = sn_decode_string(p, end, result);
		break;
	case SN_BIN:
		c = sn_decode_bin(p, end, result);
		break;
	default:
		return SN_PERROR;
	}
	switch (c) {
	case SN_PERROR: return -1;
	case SN_PINCOMPLETE: return 0;
	}
	if (snunlikely(c == 0))
		return -1;
	return c;
}

static inline int
sn_request_set(char **start, char *end, char *p, int argc,
               snrequest **request)
{
	if (snunlikely(argc != 3))
		return -1;
	/* key */
	char *key;
	int c;
	c = sn_request_str(&p, end, &key);
	if (snunlikely(c <= 0))
		return c;
	int keysize = c;
	/* value */
	char *value;
	c = sn_request_str(&p, end, &value);
	if (snunlikely(c <= 0))
		return c;
	int valuesize = c;
	/* create request */
	snrequest *r = sn_malloc(sizeof(snrequest) + keysize + valuesize);
	if (snunlikely(r == NULL))
		return -1;
	r->cmd = SN_SET;
	r->key = (char*)r + sizeof(snrequest);
	r->keysize = keysize;
	r->value = r->key + r->keysize;
	r->valuesize = valuesize;
	memcpy(r->key, key, keysize);
	memcpy(r->value, value, valuesize);
	*request = r;
	/* move readahead */
	*start = p;
	return 1;
}

static inline int
sn_request_del(char **start, char *end, char *p, int argc,
               snrequest **request)
{
	if (snunlikely(argc != 2))
		return -1;
	/* key */
	char *key;
	int c;
	c = sn_request_str(&p, end, &key);
	if (snunlikely(c <= 0))
		return c;
	int keysize = c;
	/* create request */
	snrequest *r = sn_malloc(sizeof(snrequest) + keysize);
	if (snunlikely(r == NULL))
		return -1;
	r->cmd = SN_DEL;
	r->key = (char*)r + sizeof(snrequest);
	r->keysize = keysize;
	r->value = NULL;
	r->valuesize = 0;
	memcpy(r->key, key, keysize);
	*request = r;
	/* move readahead */
	*start = p;
	return 1;
}

static inline int
sn_request_get(char **start, char *end, char *p, int argc,
               snrequest **request)
{
	if (snunlikely(argc != 2))
		return -1;
	/* key */
	char *key;
	int c;
	c = sn_request_str(&p, end, &key);
	if (snunlikely(c <= 0))
		return c;
	int keysize = c;
	/* create request */
	snrequest *r = sn_malloc(sizeof(snrequest) + keysize);
	if (snunlikely(r == NULL))
		return -1;
	r->cmd = SN_GET;
	r->key = (char*)r + sizeof(snrequest);
	r->keysize = keysize;
	r->value = NULL;
	r->valuesize = 0;
	memcpy(r->key, key, keysize);
	*request = r;
	/* move readahead */
	*start = p;
	return 1;
}

static inline int
sn_request_exists(char **start, char *end, char *p, int argc,
                  snrequest **request)
{
	if (snunlikely(argc != 2))
		return -1;
	/* key */
	char *key;
	int c;
	c = sn_request_str(&p, end, &key);
	if (snunlikely(c <= 0))
		return c;
	int keysize = c;
	/* create request */
	snrequest *r = sn_malloc(sizeof(snrequest) + keysize);
	if (snunlikely(r == NULL))
		return -1;
	r->cmd = SN_EXISTS;
	r->key = (char*)r + sizeof(snrequest);
	r->keysize = keysize;
	r->value = NULL;
	r->valuesize = 0;
	memcpy(r->key, key, keysize);
	*request = r;
	/* move readahead */
	*start = p;
	return 1;
}

static inline int
sn_request_cursor(char **start, char *end, char *p, int argc,
                  snrequest **request)
{
	if (snunlikely(argc < 2))
		return -1;
	/* key */
	char *key;
	int c;
	c = sn_request_str(&p, end, &key);
	if (snunlikely(c <= 0))
		return c;
	int keysize = c;

	/* value */
	char *value = NULL;
	int valuesize = 0;

	/* options */
	int n = argc - 2;
	while (n > 0) {
		char *opt;
		c = sn_request_str(&p, end, &opt);
		if (snunlikely(c <= 0))
			return c;
		int optsize = c;
		if (strncasecmp(opt, "pos", optsize) == 0) {
			n--;
			if (snunlikely(n == 0))
				return -1;
			valuesize = sn_request_str(&p, end, &value);
			if (snunlikely(valuesize <= 0))
				return valuesize;
			n--;
		} else {
			return -1;
		}
	}
	/* create request */
	snrequest *r = sn_malloc(sizeof(snrequest) + keysize + valuesize);
	if (snunlikely(r == NULL))
		return -1;
	r->cmd = SN_CURSOR;
	r->key = (char*)r + sizeof(snrequest);
	r->keysize = keysize;
	r->valuesize = valuesize;
	if (r->valuesize > 0)
		r->value = r->key + r->keysize;
	else
		r->value = NULL;
	memcpy(r->key, key, keysize);
	memcpy(r->value, value, valuesize);
	*request = r;
	/* move readahead */
	*start = p;
	return 1;
}

static inline int
sn_request_close(char **start, char *end, char *p, int argc,
                  snrequest **request)
{
	if (snunlikely(argc != 2))
		return -1;
	/* key */
	char *key;
	int c;
	c = sn_request_str(&p, end, &key);
	if (snunlikely(c <= 0))
		return c;
	int keysize = c;
	/* create request */
	snrequest *r = sn_malloc(sizeof(snrequest) + keysize);
	if (snunlikely(r == NULL))
		return -1;
	r->cmd = SN_CLOSE;
	r->key = (char*)r + sizeof(snrequest);
	r->keysize = keysize;
	r->value = NULL;
	r->valuesize = 0;
	memcpy(r->key, key, keysize);
	*request = r;
	/* move readahead */
	*start = p;
	return 1;
}

static inline int
sn_request_next(char **start, char *end, char *p, int argc,
                snrequest **request)
{
	if (snunlikely(argc < 2 || argc > 3))
		return -1;
	/* key */
	char *key;
	int c;
	c = sn_request_str(&p, end, &key);
	if (snunlikely(c <= 0))
		return c;
	int keysize = c;
	/* value */
	char *value = NULL;
	int valuesize = 0;
	if (argc == 3) {
		c = sn_request_str(&p, end, &value);
		if (snunlikely(c <= 0))
			return c;
		valuesize = c;
	}
	/* create request */
	snrequest *r = sn_malloc(sizeof(snrequest) + keysize + valuesize);
	if (snunlikely(r == NULL))
		return -1;
	r->cmd = SN_NEXT;
	r->key = (char*)r + sizeof(snrequest);
	r->keysize = keysize;
	r->valuesize = valuesize;
	if (value)
		r->value = r->key + r->keysize;
	else
		r->value = NULL;
	memcpy(r->key, key, keysize);
	memcpy(r->value, value, valuesize);
	*request = r;
	/* move readahead */
	*start = p;
	return 1;
}

static inline int
sn_request_select(char **start, char *end, char *p, int argc,
                  snrequest **request)
{
	if (snunlikely(argc != 2))
		return -1;
	/* key */
	char *key;
	int c;
	c = sn_request_str(&p, end, &key);
	if (snunlikely(c <= 0))
		return c;
	int keysize = c;
	/* create request */
	snrequest *r = sn_malloc(sizeof(snrequest) + keysize);
	if (snunlikely(r == NULL))
		return -1;
	r->cmd = SN_SELECT;
	r->key = (char*)r + sizeof(snrequest);
	r->keysize = keysize;
	r->value = NULL;
	r->valuesize = 0;
	memcpy(r->key, key, keysize);
	*request = r;
	/* move readahead */
	*start = p;
	return 1;
}

static inline int
sn_request_begin(char **start, char *end, char *p, int argc,
                  snrequest **request)
{
	(void)p;
	(void)end;
	if (snunlikely(argc != 1))
		return -1;
	/* create request */
	snrequest *r = sn_malloc(sizeof(snrequest));
	if (snunlikely(r == NULL))
		return -1;
	r->cmd = SN_BEGIN;
	r->key = NULL;
	r->keysize = 0;
	r->value = NULL;
	r->valuesize = 0;
	*request = r;
	/* move readahead */
	*start = p;
	return 1;
}

static inline int
sn_request_commit(char **start, char *end, char *p, int argc,
                  snrequest **request)
{
	(void)p;
	(void)end;
	if (snunlikely(argc != 1))
		return -1;
	/* create request */
	snrequest *r = sn_malloc(sizeof(snrequest));
	if (snunlikely(r == NULL))
		return -1;
	r->cmd = SN_COMMIT;
	r->key = NULL;
	r->keysize = 0;
	r->value = NULL;
	r->valuesize = 0;
	*request = r;
	/* move readahead */
	*start = p;
	return 1;
}

static inline int
sn_request_rollback(char **start, char *end, char *p, int argc,
                    snrequest **request)
{
	(void)p;
	(void)end;
	if (snunlikely(argc != 1))
		return -1;
	/* create request */
	snrequest *r = sn_malloc(sizeof(snrequest));
	if (snunlikely(r == NULL))
		return -1;
	r->cmd = SN_ROLLBACK;
	r->key = NULL;
	r->keysize = 0;
	r->value = NULL;
	r->valuesize = 0;
	*request = r;
	/* move readahead */
	*start = p;
	return 1;
}

static inline int
sn_request_call(char **start, char *end, char *p, int argc,
                snrequest **request)
{
	if (snunlikely(argc < 2 || argc > 3))
		return -1;
	/* key */
	char *key;
	int c;
	c = sn_request_str(&p, end, &key);
	if (snunlikely(c <= 0))
		return c;
	int keysize = c;
	/* value */
	char *value = NULL;
	int valuesize = 0;
	if (argc == 3) {
		c = sn_request_str(&p, end, &value);
		if (snunlikely(c <= 0))
			return c;
		valuesize = c;
	}
	/* create request */
	snrequest *r = sn_malloc(sizeof(snrequest) + keysize + valuesize);
	if (snunlikely(r == NULL))
		return -1;
	r->cmd = SN_CALL;
	r->key = (char*)r + sizeof(snrequest);
	r->keysize = keysize;
	r->value = r->key + r->keysize;
	r->valuesize = valuesize;
	memcpy(r->key, key, keysize);
	memcpy(r->value, value, valuesize);
	*request = r;
	/* move readahead */
	*start = p;
	return 1;
}

static inline int
sn_request_ping(char **start, char *end, char *p, int argc,
                  snrequest **request)
{
	(void)p;
	(void)end;
	if (snunlikely(argc != 1))
		return -1;
	/* create request */
	snrequest *r = sn_malloc(sizeof(snrequest));
	if (snunlikely(r == NULL))
		return -1;
	r->cmd = SN_PING;
	r->key = NULL;
	r->keysize = 0;
	r->value = NULL;
	r->valuesize = 0;
	*request = r;
	/* move readahead */
	*start = p;
	return 1;
}

static inline int
sn_request_info(char **start, char *end, char *p, int argc,
                  snrequest **request)
{
	(void)p;
	(void)end;
	if (snunlikely(argc != 1))
		return -1;
	/* create request */
	snrequest *r = sn_malloc(sizeof(snrequest));
	if (snunlikely(r == NULL))
		return -1;
	r->cmd = SN_INFO;
	r->key = NULL;
	r->keysize = 0;
	r->value = NULL;
	r->valuesize = 0;
	*request = r;
	/* move readahead */
	*start = p;
	return 1;
}

static inline int
sn_request_shutdown(char **start, char *end, char *p, int argc,
                  snrequest **request)
{
	(void)p;
	(void)end;
	if (snunlikely(argc != 1))
		return -1;
	/* create request */
	snrequest *r = sn_malloc(sizeof(snrequest));
	if (snunlikely(r == NULL))
		return -1;
	r->cmd = SN_SHUTDOWN;
	r->key = NULL;
	r->keysize = 0;
	r->value = NULL;
	r->valuesize = 0;
	*request = r;
	/* move readahead */
	*start = p;
	return 1;
}

int sn_request(char **start, char *end, snrequest **r)
{
	char *p = *start;
	/* array */
	int c = sn_decode_next(&p, end);
	switch (c) {
	default:
	case SN_PERROR: return -1;
	case SN_PINCOMPLETE: return 0;
	case SN_ARRAY: break;
	}
	c = sn_decode_array(&p, end);
	switch (c) {
	case SN_PERROR: return -1;
	case SN_PINCOMPLETE: return 0;
	}
	int count = c;
	if (snunlikely(c == 0))
		return -1;
	/* string */
	char *command;
	c = sn_request_str(&p, end, &command);
	if (snunlikely(c <= 0))
		return c;
	int size = c;
	if ((size == 3) && strncasecmp(command, "set", size) == 0) {
		return sn_request_set(start, end, p, count, r);
	} else
	if ((size == 3) && strncasecmp(command, "del", size) == 0) {
		return sn_request_del(start, end, p, count, r);
	} else
	if ((size == 3) && strncasecmp(command, "get", size) == 0) {
		return sn_request_get(start, end, p, count, r);
	} else
	if ((size == 6) && strncasecmp(command, "exists", size) == 0) {
		return sn_request_exists(start, end, p, count, r);
	} else
	if ((size == 6) && strncasecmp(command, "cursor", size) == 0) {
		return sn_request_cursor(start, end, p, count, r);
	} else
	if ((size == 4) && strncasecmp(command, "next", size) == 0) {
		return sn_request_next(start, end, p, count, r);
	} else
	if ((size == 5) && strncasecmp(command, "close", size) == 0) {
		return sn_request_close(start, end, p, count, r);
	} else
	if ((size == 6) && strncasecmp(command, "select", size) == 0) {
		return sn_request_select(start, end, p, count, r);
	} else
	if ((size == 5) && strncasecmp(command, "begin", size) == 0) {
		return sn_request_begin(start, end, p, count, r);
	} else
	if ((size == 6) && strncasecmp(command, "commit", size) == 0) {
		return sn_request_commit(start, end, p, count, r);
	} else
	if ((size == 8) && strncasecmp(command, "rollback", size) == 0) {
		return sn_request_rollback(start, end, p, count, r);
	} else
	if ((size == 4) && strncasecmp(command, "call", size) == 0) {
		return sn_request_call(start, end, p, count, r);
	} else
	if ((size == 4) && strncasecmp(command, "ping", size) == 0) {
		return sn_request_ping(start, end, p, count, r);
	} else
	if ((size == 4) && strncasecmp(command, "info", size) == 0) {
		return sn_request_info(start, end, p, count, r);
	} else
	if ((size == 8) && strncasecmp(command, "shutdown", size) == 0) {
		return sn_request_shutdown(start, end, p, count, r);
	}
	return -1;
}

int sn_request_free(snrequest *r)
{
	sn_free(r);
	return 0;
}
