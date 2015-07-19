
/*
 * serenity database
 *
 * Copyright (c) Dmitry Simonenko
 * BSD License
*/

#include <serenity.h>

extern sn serenity;

static int
hello_procedure(snclient *c, char *arg, int argsize)
{
	(void)arg;
	(void)argsize;
	return sn_encode_stringf(&c->result, "OK");
}

int severity_module(int shutdown)
{
	sn_log(&serenity.log, "test module init: %d\n", shutdown);
	sn_procadd(&serenity.procs, "hello", hello_procedure);
	return 0;
}
