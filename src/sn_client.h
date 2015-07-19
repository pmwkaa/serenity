#ifndef SN_CLIENT_H_
#define SN_CLIENT_H_

/*
 * serenity database
 *
 * Copyright (c) Dmitry Simonenko
 * BSD License
*/

typedef struct snclient snclient;

struct snclient {
	snloopfd o;
	int fd;
	snbuf readahead;
	snbuf result;
	int result_inprogress;
	int result_pos;
	void *server;
	void *transaction;
	int db;
	sncursorlist cursors;
	snlist link;
};

snclient *sn_client(void);
int sn_clientclose(snclient*);
int sn_clientfree(snclient*);
int sn_clientread(snclient*, int);
int sn_clientwrite(snclient*);
int sn_clientwrite_ready(snclient*);

#endif
