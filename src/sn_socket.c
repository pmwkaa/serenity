
/*
 * serenity database
 *
 * Copyright (c) Dmitry Simonenko
 * BSD License
*/

#include <serenity.h>

int sn_socket_set_reuseaddr(int fd)
{
	int opt = 1;
	return setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt));
}

int sn_socket_set_nonblock(int fd)
{
	int flags = fcntl(fd, F_GETFL, 0);
	if (flags < 0)
		return -1;
	flags = flags|O_NONBLOCK;
	return fcntl(fd, F_SETFL, flags);
}

int sn_socket_set_nodelay(int fd)
{
	int val = 1;
	return setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &val, sizeof(val));
}

int sn_socket_peer(int fd, char *addr, int addrsize)
{
    struct sockaddr_storage sa;
    socklen_t salen = sizeof(sa);
	char peer[128];
	int rc;
	int port;
	rc = getpeername(fd, (struct sockaddr*)&sa, &salen);
	if (snunlikely(rc == -1)) {
		snprintf(addr, addrsize, "(fd: %d)", fd);
		return -1;
	}
	if (sa.ss_family == AF_INET) {
		struct sockaddr_in *s = (struct sockaddr_in*)&sa;
		inet_ntop(AF_INET, (void*)&(s->sin_addr), peer, sizeof(peer));
		port = ntohs(s->sin_port);
		snprintf(addr, addrsize, "%s:%d", peer, port);
		return 0;
	}
	if (sa.ss_family == AF_INET6) {
		struct sockaddr_in6 *s = (struct sockaddr_in6*)&sa;
		inet_ntop(AF_INET6, (void*)&(s->sin6_addr), peer, sizeof(peer));
		port = ntohs(s->sin6_port);
		snprintf(addr, addrsize, "%s:%d", peer, port);
		return 0;
	}
	return -1;
}
