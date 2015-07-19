#ifndef SN_SOCKET_H_
#define SN_SOCKET_H_

/*
 * serenity database
 *
 * Copyright (c) Dmitry Simonenko
 * BSD License
*/

int sn_socket_set_reuseaddr(int);
int sn_socket_set_nonblock(int);
int sn_socket_set_nodelay(int);
int sn_socket_peer(int, char*, int);

#endif
