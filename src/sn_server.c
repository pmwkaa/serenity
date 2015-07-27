
/*
 * serenity database
 *
 * Copyright (c) Dmitry Simonenko
 * BSD License
*/

#include <serenity.h>

int sn_serverinit(snserver *s, snlog *log, snloop *l, snhandle *fh,
                  sn_onconnectf on_connect,
                  sn_onreadf on_read,
                  sn_onwritef on_write,
                  void *ptr)
{
	s->ptr = ptr;
	s->log = log;
	s->loop = l;
	s->epoll = fh;
	s->fd = -1;
	s->on_connect = on_connect;
	s->on_read = on_read;
	s->on_write = on_write;
	sn_clientlist_init(&s->cl);
	memset(&s->server, 0, sizeof(s->server));
	return 0;
}

static int
sn_client_onevent(snloopfd *fd, int type)
{
	snclient *client = fd->arg;
	snserver *s = client->server;
	if ((type & SN_LOOPFD_R) && s->on_read)
		return s->on_read(s, client);
	if ((type & SN_LOOPFD_W) && s->on_write)
		return s->on_write(s, client);
	return 0;
}

int sn_serverdelete(snserver *s, snclient *client)
{
	sn_epoll_delete(s->epoll, &client->o);
	return 0;
}

static int
sn_server_onconnect(snloopfd *fd, int type snunused)
{
	snserver *s = fd->arg;
	/* accept */
	struct sockaddr_in addr;
	socklen_t addrlen = sizeof(addr);
	int client_fd;
	client_fd = accept(fd->fd, (struct sockaddr*)&addr,
	                   (socklen_t*)&addrlen);
	if (snunlikely(client_fd == -1)) {
		sn_log(s->log, "error: accept(): %s",
		       strerror(errno));
		return -1;
	}
	/* new client */
	snclient *client = sn_client();
	if (snunlikely(client == NULL))
		return -1;
	client->fd = client_fd;
	client->server = s;
	int rc;
	rc = sn_socket_set_nonblock(client->fd);
	if (snunlikely(rc == -1)) {
		sn_log(s->log, "error: fcntl(F_GETFL/F_SETFL): %s",
		       strerror(errno));
		return -1;
	}
	rc = sn_socket_set_nodelay(client->fd);
	if (snunlikely(rc == -1)) {
		sn_log(s->log, "error: setsockopt(TCP_NODELAY): %s",
		       strerror(errno));
		return -1;
	}
	sn_clientlist_add(&s->cl, client);
	sn_loopfd_init(&client->o, client_fd, sn_client_onevent, client);
	sn_epoll_add(s->epoll, &client->o, SN_LOOPFD_R|SN_LOOPFD_W);
	if (s->on_connect)
		return s->on_connect(s, client);
	return 0;
}

int sn_serveropen(snserver *s, char *host, int port)
{
	int fd = socket(AF_INET, SOCK_STREAM, 0);
	if (snunlikely(fd == -1)) {
		sn_log(s->log, "error: socket(): %s", strerror(errno));
		return -1;
	}
	int rc;
	rc = sn_socket_set_reuseaddr(fd);
	if (snunlikely(rc == -1)) {
		sn_log(s->log, "error: setsockopt(SO_REUSEADDR): %s",
		       strerror(errno));
		goto error;
	}
	rc = sn_socket_set_nonblock(fd);
	if (snunlikely(rc == -1)) {
		sn_log(s->log, "error: fcntl(F_GETFL/F_SETFL): %s",
		       strerror(errno));
		goto error;
	}
	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(host);
	addr.sin_port = htons(port);
	rc = bind(fd, (struct sockaddr *)&addr, sizeof(addr));
	if (snunlikely(rc == -1)) {
		sn_log(s->log, "error: bind(): %s", strerror(errno));
		goto error;
	}
	rc = listen(fd, 16);
	if (snunlikely(rc == -1)) {
		sn_log(s->log, "error: listen(): %s", strerror(errno));
		goto error;
	}
	s->fd = fd;
	sn_loopfd_init(&s->server, s->fd, sn_server_onconnect, s);
	sn_epoll_add(s->epoll, &s->server, SN_LOOPFD_R);
	return 0;
error:
	close(fd);
	return -1;
}

int sn_servershutdown(snserver *s)
{
	sn_clientlist_shutdown(&s->cl);
	if (s->fd != -1) {
		int rc = close(s->fd);
		if (rc == -1) {
			sn_log(s->log, "error: close(): %s",
			       strerror(errno));
		}
		s->fd = -1;
	}
	return 0;
}
