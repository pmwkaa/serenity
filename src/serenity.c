
/*
 * serenity database
 *
 * Copyright (c) Dmitry Simonenko
 * BSD License
*/

#include <serenity.h>

sn serenity;

static inline char*
serenity_config(int argc, char *argv[])
{
	char *config;
	if (argc == 1) {
		config = NULL;
	} else
	if (argc == 2) {
		config = argv[1];
		if (strcmp(config, "-h") == 0 ||
		    strcmp(config, "--help") == 0)
			goto usage;
	} else {
		goto usage;
	}
	return config;
usage:
	printf("serenity database.\n\n");
	printf("usage: %s [config]\n", argv[0]);
	exit(1);
	return NULL;
}

int
main(int argc, char *argv[])
{
	char *config = serenity_config(argc, argv);
	int rc;
	rc = sn_init(&serenity, config);
	if (rc == -1)
		return 1;
	rc = sn_open(&serenity);
	if (rc == -1)
		goto shutdown;
	sn_start(&serenity);
shutdown:
	sn_shutdown(&serenity);
	return 0;
}
