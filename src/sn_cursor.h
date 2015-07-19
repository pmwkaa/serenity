#ifndef SN_CURSOR_H_
#define SN_CURSOR_H_

/*
 * serenity database
 *
 * Copyright (c) Dmitry Simonenko
 * BSD License
*/

typedef struct sncursorlist sncursorlist;
typedef struct sncursor sncursor;

struct sncursor {
	int id;
	void *c;
	snlist link;
};

struct sncursorlist {
	snlist list;
	int count;
	int seq;
};

void sn_cursorlist_init(sncursorlist*);
void sn_cursorlist_free(sncursorlist*);
void sn_cursorfree(sncursorlist*, sncursor*);
void sn_cursorattach(sncursorlist*, sncursor*);
sncursor *sn_cursornew(sncursorlist*);
sncursor *sn_cursormatch(sncursorlist*, int);

#endif
