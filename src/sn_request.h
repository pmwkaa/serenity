#ifndef SN_REQUEST_H_
#define SN_REQUEST_H_

/*
 * serenity database
 *
 * Copyright (c) Dmitry Simonenko
 * BSD License
*/

typedef struct snrequest snrequest;

typedef enum {
	SN_PING,
	SN_INFO,
	SN_SHUTDOWN,
	SN_SET,
	SN_DEL,
	SN_GET,
	SN_EXISTS,
	SN_SELECT,
	SN_CURSOR,
	SN_NEXT,
	SN_CLOSE,
	SN_BEGIN,
	SN_COMMIT,
	SN_ROLLBACK,
	SN_CALL
} sncommand;

struct snrequest {
	sncommand cmd;
	char *key;
	int keysize;
	char *value;
	int valuesize;
};

int sn_request(char**, char*, snrequest**);
int sn_request_free(snrequest*);

#endif
