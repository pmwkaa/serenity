#ifndef SN_LOG_H_
#define SN_LOG_H_

/*
 * serenity database
 *
 * Copyright (c) Dmitry Simonenko
 * BSD License
*/

typedef struct snlog snlog;

struct snlog {
	int pid;
	int fd;
};

int sn_loginit(snlog*);
int sn_logopen(snlog*, char*);
int sn_logclose(snlog*);
int sn_log(snlog*, char*, ...);

#endif
