
/*
 * serenity database
 *
 * Copyright (c) Dmitry Simonenko
 * BSD License
*/

#include <serenity.h>

snclient *sn_client(void)
{
	snclient *client = sn_malloc(sizeof(snclient));
	if (snunlikely(client == NULL))
		return NULL;
	sn_bufinit(&client->readahead);
	sn_bufinit(&client->result);
	sn_listinit(&client->link);
	memset(&client->fd, 0, sizeof(client->fd));
	client->server = NULL;
	client->fd = -1;
	client->result_pos = 0;
	client->result_inprogress = 1;
	client->transaction = NULL;
	client->db = 0;
	sn_cursorlist_init(&client->cursors);
	return client;
}

int sn_clientclose(snclient *c)
{
	int rc = close(c->fd);
	c->fd = -1;
	if (c->transaction) {
		sp_destroy(c->transaction);
		c->transaction = NULL;
	}
	sn_cursorlist_free(&c->cursors);
	return rc;
}

int sn_clientfree(snclient *c)
{
	assert(c->fd == -1);
	sn_buffree(&c->readahead);
	sn_buffree(&c->result);
	sn_free(c);
	return 0;
}

int sn_clientread(snclient *c, int size)
{
	int rc = sn_bufensure(&c->readahead, size);
	if (snunlikely(rc == -1))
		return -1;
	snserver *s = c->server;
	rc = read(c->fd, c->readahead.p, size);
	if (snunlikely(rc == -1)) {
		if (errno == EAGAIN || errno == EINTR)
			return 0;
		char peer[128];
		peer[0] = 0;
		sn_socket_peer(c->fd, peer, sizeof(peer));
		sn_log(s->log, "client %s read error: %s",
		       peer, strerror(errno));
		return -1;
	}
	sn_bufadvance(&c->readahead, rc);
	return rc;
}

int sn_clientwrite(snclient *c)
{
	snserver *s = c->server;
	int total = sn_bufused(&c->result);
	int to_write = total - c->result_pos;
	int rc = write(c->fd, c->result.s + c->result_pos, to_write);
	if (rc < 0) {
		char peer[128];
		peer[0] = 0;
		sn_socket_peer(c->fd, peer, sizeof(peer));
		sn_log(s->log, "client %s write error: %s",
		       peer, strerror(errno));
		return rc;
	}
	if (rc == 0)
		return 0;
	c->result_pos += rc;
	if (c->result_pos == total) {
		c->result_pos = 0;
		sn_bufreset(&c->result);
	}	
	return rc;
}

int sn_clientwrite_ready(snclient *c)
{
	snserver *s = c->server;
	if (c->result_inprogress) {
		if (sn_bufused(&c->result) > 0)
			return 0;
		sn_epoll_modify(s->epoll, &c->o, SN_LOOPFD_R);
		c->result_inprogress = 0;
		return 0;
	}
	if (sn_bufused(&c->result) == 0)
		return 0;
	sn_epoll_modify(s->epoll, &c->o, SN_LOOPFD_R|SN_LOOPFD_W);
	c->result_inprogress = 1;
	return 0;
}
