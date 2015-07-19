
/*
 * serenity database
 *
 * Copyright (c) Dmitry Simonenko
 * BSD License
*/

#include <serenity.h>

int sn_loginit(snlog *l)
{
	l->pid = getpid();
	l->fd = -1;
	return 0;
}

int sn_logopen(snlog *l, char *path)
{
	int rc = open(path, O_RDWR|O_CREAT|O_APPEND, 0644);
	if (rc == -1)
		return -1;
	l->fd = rc;
	return 0;
}

int sn_logclose(snlog *l)
{
	if (l->fd == -1)
		return 0;
	int rc = close(l->fd);
	l->fd = -1;
	return rc;
}

int sn_log(snlog *l, char *fmt, ...)
{
	char buffer[16 * 1024];
	/* pid + role */
	int len = snprintf(buffer, sizeof(buffer), "%d:M ", l->pid);
	va_list args;
	va_start(args, fmt);
	/* time */
	struct timeval tv;
	gettimeofday(&tv, NULL);
	len += strftime(buffer + len, sizeof(buffer) - len, "%d %b %H:%M:%S.",
	                localtime(&tv.tv_sec));
	len += snprintf(buffer + len, sizeof(buffer) - len, "%03d  ",
	                (int)tv.tv_usec / 1000);
	/* message */
	len += vsnprintf(buffer + len, sizeof(buffer) - len, fmt, args);
	va_end(args);
	len += snprintf(buffer + len, sizeof(buffer), "\n");
	if (l->fd >= 0) {
		if (write(l->fd, buffer, len) == -1)
			return -1;
	} else {
		printf("%s", buffer);
	}
	return 0;
}
