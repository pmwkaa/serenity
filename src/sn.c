
/*
 * serenity database
 *
 * Copyright (c) Dmitry Simonenko
 * BSD License
*/

#include <serenity.h>

static inline int
sn_closeof(snserver *s, snclient *c)
{
	sn_epoll_delete(s->epoll, &c->o);
	sn_clientlist_delete(&s->cl, c);
	sn_clientclose(c);
	sn_clientfree(c);
	return 0;
}

static int on_write(snserver *s, snclient *c)
{
	int rc = sn_clientwrite(c);
	if (rc < 0) {
		sn_closeof(s, c);
		return 0;
	}
	sn_clientwrite_ready(c);
	return 0;
}

static int on_read(snserver *s, snclient *c)
{
	int rc = sn_clientread(c, 1024);
	if (rc <= 0) {
		sn_closeof(s, c);
		return 0;
	}
	char *position = c->readahead.s;
	char *end = c->readahead.p;
	while (1) {
		snrequest *req;
		int rc = sn_request(&position, end, &req);
		if (rc == 0) {
			sn_bufmove(&c->readahead, position);
			break;
		}
		if (rc == -1) {
			sn_encode_errorf(&c->result, "ERROR bad request");
			sn_clientwrite(c);
			sn_closeof(s, c);
			break;
		}
		sn *serenity = s->ptr;
		sn_execute(serenity, c, req);
		sn_clientwrite_ready(c);
		sn_request_free(req);
	}
	return 0;
}

int sn_init(sn *s, char *config)
{
	memset(s, 0, sizeof(*s));
	sn_loginit(&s->log);
	sn_moduleinit(&s->modules);
	sn_procinit(&s->procs);
	int rc = sn_configinit(&s->conf);
	if (rc == -1)
		return -1;
	rc = sn_configopen(&s->conf, &s->log, &s->modules, config);
	if (rc == -1)
		return -1;
	if (s->conf.logfile) {
		rc = sn_logopen(&s->log, s->conf.logfile);
		if (rc == -1) {
			sn_log(&s->log, "error: failed to open config file '%s'",
			       s->conf.logfile);
			return -1;
		}
	}
	sn_log(&s->log, "");
	sn_log(&s->log, "serenity database.");
	sn_log(&s->log, "");
	s->active = 0;
	sn_loopinit(&s->loop, s);
	s->epoll = sn_epoll(&s->loop);
	if (snunlikely(s->epoll == NULL))
		return -1;
	sn_loopregister(&s->loop, s->epoll);
	sn_serverinit(&s->server, &s->log, &s->loop, s->epoll, NULL,
	              on_read, on_write, s);
	return 0;
}

sig_atomic_t on_signal = 0;

static void
sn_on_signal(int id)
{
	if (on_signal == id) {
		_exit(1);
	}
	on_signal = id;
}

static void
sn_pidfile(char *path, int create)
{
	if (! create) {
		unlink(path);
		return;
	}
	FILE *f = fopen(path, "w");
	if (f == NULL)
		return;
	fprintf(f, "%d\n", getpid());
	fclose(f);
}

static void
sn_daemonize(sn *s snunused)
{
	int rc = fork();
	if (rc != 0)
		exit(0);
	setsid();
	int fd = open("/dev/null", O_RDWR, 0);
	dup2(fd, STDIN_FILENO);
	dup2(fd, STDOUT_FILENO);
	dup2(fd, STDERR_FILENO);
	if (fd > STDERR_FILENO)
		close(fd);
	if (s->conf.pidfile)
		sn_pidfile(s->conf.pidfile, 1);
}

static void
sn_setup(sn *s)
{
	signal(SIGHUP, SIG_IGN);
	signal(SIGPIPE, SIG_IGN);
	struct sigaction sa;
	memset(&sa, 0, sizeof(sa));
	sa.sa_flags   = 0;
	sa.sa_handler = sn_on_signal;
	sigaction(SIGINT, &sa, NULL);
	sigaction(SIGTERM, &sa, NULL);
	if (s->conf.daemonize)
		sn_daemonize(s);
}

int sn_open(sn *s)
{
	sn_configshow(&s->conf, &s->log);
	sn_setup(s);
	int rc;
	rc = sn_storageopen(&s->storage, &s->conf, &s->log);
	if (snunlikely(rc == -1))
		return -1;
	rc = sn_moduleopen(&s->modules, &s->log);
	if (snunlikely(rc == -1))
		return -1;
	sn_log(&s->log, "complete");
	sn_log(&s->log, "");
	rc = sn_serveropen(&s->server, s->conf.bind, s->conf.port);
	if (snunlikely(rc == -1)) {
		return -1;
	}
	return 0;
}

int sn_shutdown(sn *s)
{
	if (on_signal) {
		sn_log(&s->log, "shutdown by signal %d", on_signal);
	} else {
		sn_log(&s->log, "shutdown");
	}
	int rcret = 0;
	int rc;
	sn_servershutdown(&s->server);
	rc = sn_storageclose(&s->storage);
	if (snunlikely(rc == -1))
		rcret = -1;
	sn_loopfree(&s->loop);
	sn_procfree(&s->procs);
	sn_moduleshutdown(&s->modules);
	if (s->conf.daemonize && s->conf.pidfile)
		sn_pidfile(s->conf.pidfile, 0);
	sn_configfree(&s->conf);
	sn_logclose(&s->log);
	return rcret;
}

int sn_start(sn *s)
{
	s->active = 1;
	while (s->active && !on_signal) {
		sn_tick(&s->loop);
	}
	return 0;
}

int sn_stop(sn *s)
{
	s->active = 0;
	return 0;
}
