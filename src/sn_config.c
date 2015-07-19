
/*
 * serenity database
 *
 * Copyright (c) Dmitry Simonenko
 * BSD License
*/

#include <serenity.h>

int sn_configinit(snconfig *s)
{
	s->bind = strdup("127.0.0.1");
	if (snunlikely(s->bind == NULL))
		return -1;
	s->port = 6379;
	s->dir = strdup("./serenity-db");
	if (s->dir == NULL)
		goto error;
	s->logfile   = NULL;
	s->pidfile   = NULL;
	s->maxmemory = 10ULL * 1024 * 1024 * 1024; /* 10Gb */
	s->databases = 3;
	sn_listinit(&s->scheme);
	s->daemonize = 0;
	return 0;
error:
	sn_configfree(s);
	return -1;
}

int sn_configfree(snconfig *s)
{
	if (s->bind)
		sn_free(s->bind);
	if (s->dir)
		sn_free(s->dir);
	snlist *i, *n;
	sn_listforeach_safe(&s->scheme, i, n) {
		snconfigdb *p = sncast(i, snconfigdb, link);
		if (p->name)
			sn_free(p->name);
		if (p->path)
			sn_free(p->path);
		if (p->key)
			sn_free(p->key);
		sn_free(p);
	}
	return 0;
}

static inline int
chomp(char *s, int size)
{
	int i = size - 1;
	while (i >= 0 && s[i] != '\n') {
		i--;
	}
	if (i >= 0)
		s[i] = 0;
	return i;
}

static inline char*
next_name(char *p, char *end)
{
	while (p < end && isspace(*p)) {
		p++;
		continue;
	}
	if (snunlikely(p == end))
		return NULL;
	return p;
}

static inline char*
next_name_end(char *p, char *end)
{
	while (p < end && *p != ' ') {
		p++;
		continue;
	}
	return p;
}

typedef struct {
	char *name;
	int size;
} snarg;

static inline int
split(char *line, char *end, snarg argv[16])
{
	int argc = 0;
	char *p = line;
	while (p < end) {
		p = next_name(p, end);
		if (snunlikely(p == NULL))
			break;
		if (*p == '#')
			break;
		if (argc == 15)
			return -1;
		char *name = p;
		char *name_end = next_name_end(p, end);
		*name_end = 0;
		argv[argc].size = name_end - name;
		argv[argc].name = name;
		argc++;
		p = name_end + 1;
	}
	return argc;
}

snconfigdb*
sn_configscheme_match(snconfig *s, int id)
{
	snlist *i;
	sn_listforeach(&s->scheme, i) {
		snconfigdb *p = sncast(i, snconfigdb, link);
		if (p->id == id)
			return p;
	}
	return NULL;
}

static snconfigdb*
sn_configscheme_new(snconfig *s, int id)
{
	snconfigdb *db = sn_malloc(sizeof(snconfigdb));
	if (snunlikely(db == NULL))
		return NULL;
	db->id = id;
	db->name = NULL;
	db->path = NULL;
	db->key  = NULL;
	sn_listappend(&s->scheme, &db->link);
	return db;
}

static int
sn_configscheme_set(snconfig *s, int argc, snarg argv[])
{
	if (argc == 1)
		return -1;
	int id = atoi(argv[1].name);
	snconfigdb *db = sn_configscheme_match(s, id);
	if (db == NULL)
		db = sn_configscheme_new(s, id);
	if (db == NULL)
		return -1;
	int n = argc - 2;
	int i = 2;
	while (n > 0) {
		if (strcasecmp(argv[i].name, "name") == 0) {
			n--;
			if (snunlikely(n == 0))
				return -1;
			i++;
			if (isdigit(argv[i].name[0]))
				return -1;
			db->name = sn_strdup(argv[i].name);
			if (snunlikely(db->name == NULL))
				return -1;
			n--;
			i++;
		} else
		if (strcasecmp(argv[i].name, "path") == 0) {
			n--;
			if (snunlikely(n == 0))
				return -1;
			i++;
			db->path = sn_strdup(argv[i].name);
			if (snunlikely(db->path == NULL))
				return -1;
			n--;
			i++;
		} else
		if (strcasecmp(argv[i].name, "primary_key") == 0) {
			n--;
			if (snunlikely(n == 0))
				return -1;
			i++;
			if (strcasecmp(argv[i].name, "string") == 0)
				db->keytype = SN_STRING;
			else
			if (strcasecmp(argv[i].name, "u32") == 0)
				db->keytype = SN_KEYU32;
			else
			if (strcasecmp(argv[i].name, "u64") == 0)
				db->keytype = SN_KEYU64;
			else
				return -1;

			db->key = sn_strdup(argv[i].name);
			if (snunlikely(db->key == NULL))
				return -1;
			n--;
			i++;
		}
	}
	return 0;
}

static int
sn_configset(snconfig *s, snmodules *m, int argc, snarg argv[])
{
	char *sz;
	if (strcasecmp(argv[0].name, "dir") == 0) {
		if (argc != 2)
			return -1;
		sz = strdup(argv[1].name);
		if (sz == NULL)
			return -1;
		if (s->dir)
			free(s->dir);
		s->dir = sz;
	} else
	if (strcasecmp(argv[0].name, "logfile") == 0) {
		if (argc != 2)
			return -1;
		sz = strdup(argv[1].name);
		if (sz == NULL)
			return -1;
		if (s->logfile)
			free(s->logfile);
		s->logfile = sz;
	} else
	if (strcasecmp(argv[0].name, "pidfile") == 0) {
		if (argc != 2)
			return -1;
		sz = strdup(argv[1].name);
		if (sz == NULL)
			return -1;
		if (s->pidfile)
			free(s->pidfile);
		s->pidfile = sz;
	} else
	if (strcasecmp(argv[0].name, "bind") == 0) {
		if (argc != 2)
			return -1;
		sz = strdup(argv[1].name);
		if (sz == NULL)
			return -1;
		if (s->bind)
			free(s->bind);
		s->bind = sz;
	} else
	if (strcasecmp(argv[0].name, "port") == 0) {
		if (argc != 2)
			return -1;
		s->port = atoi(argv[1].name);
	} else
	if (strcasecmp(argv[0].name, "databases") == 0) {
		if (argc != 2)
			return -1;
		s->databases = atoi(argv[1].name);
	} else
	if (strcasecmp(argv[0].name, "db") == 0) {
		return sn_configscheme_set(s, argc, argv);
	} else
	if (strcasecmp(argv[0].name, "daemonize") == 0) {
		if (argc != 2)
			return -1;
		if (strcasecmp(argv[1].name, "no") == 0)
			s->daemonize = 0;
		else
		if (strcasecmp(argv[1].name, "yes") == 0)
			s->daemonize = 1;
		else
			return -1;
	} else
	if (strcasecmp(argv[0].name, "maxmemory") == 0) {
		if (argc != 2)
			return -1;
		s->maxmemory = atoll(argv[1].name);
	} else
	if (strcasecmp(argv[0].name, "module") == 0) {
		if (argc != 2)
			return -1;
		int rc = sn_moduleadd(m, argv[1].name);
		if (snunlikely(rc == -1))
			return -1;
	} else {
		return -1;
	}
	return 0;
}

int sn_configopen(snconfig *s, snlog *log, snmodules *m, char *path)
{
	s->config = path;
	if (path == NULL)
		return 0;
	FILE *f = fopen(path, "r");
	if (f == NULL) {
		sn_log(log, "error: failed to open config file '%s'", path);
		return -1;
	}
	int line_number = 1;
	char line[512];
	while (fgets(line, sizeof(line), f)) {
		int len = strlen(line);
		len = chomp(line, len);
		if (snunlikely(len == 0)) {
			line_number++;
			continue;
		}
		char *end = line + len;
		snarg argv[16];
		int argc = split(line, end, argv);
		if (argc == 0) {
			line_number++;
			continue;
		}
		if (argc == -1) {
			sn_log(log, "error: config file '%s' line %d error",
			       path, line_number);
			goto error;
		}
		int rc = sn_configset(s, m, argc, argv);
		if (snunlikely(rc == -1)) {
			sn_log(log, "error: config file '%s' line %d error",
			       path, line_number);
			goto error;
		}
		line_number++;
	}
	fclose(f);
	return 0;
error:
	fclose(f);
	return -1;
}

int sn_configshow(snconfig *c, snlog *l)
{
	if (c->config)
		sn_log(l, "using configuration file '%s'", c->config);
	else
		sn_log(l, "using default settings");
	sn_log(l, "");
	if (c->bind)
		sn_log(l, "bind %s", c->bind);
	if (c->port)
		sn_log(l, "port %d", c->port);
	if (c->maxmemory)
		sn_log(l, "maxmemory %"PRIu64, c->maxmemory);
	if (c->daemonize)
		sn_log(l, "daemonize yes");
	if (c->pidfile)
		sn_log(l, "pidfile %s", c->pidfile);
	if (c->logfile)
		sn_log(l, "logfile %s", c->logfile);
	if (c->dir)
		sn_log(l, "dir %s", c->dir);
	sn_log(l, "databases %d", c->databases);
	return 0;
}
