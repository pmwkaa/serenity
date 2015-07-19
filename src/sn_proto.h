#ifndef SN_PROTO_H_
#define SN_PROTO_H_

/*
 * serenity database
 *
 * Copyright (c) Dmitry Simonenko
 * BSD License
*/

enum {
	SN_ARRAY,
	SN_INT,
	SN_BIN,
	SN_STRING,
	SN_ERROR
};

#if 0
static inline uint32_t
digits10(uint64_t v)
{
	uint32_t result = 0;
	do {
		++result;
		v /= 10;
	} while (v);
	return result;
}

static inline size_t
sn_encode_number(char *dst, uint32_t value)
{
	static const char digits[201] =
	"0001020304050607080910111213141516171819"
	"2021222324252627282930313233343536373839"
	"4041424344454647484950515253545556575859"
	"6061626364656667686970717273747576777879"
	"8081828384858687888990919293949596979899";

	size_t const length = digits10(value);
	size_t next = length - 1;
	while (value >= 100) {
		int i = (value % 100) * 2;
		value /= 100;
		dst[next] = digits[i + 1];
		dst[next - 1] = digits[i];
		next -= 2;
	}
	if (value < 10) {
		dst[next] = '0' + (uint32_t)value;
	} else {
		int i = (uint32_t)value * 2;
		dst[next] = digits[i + 1];
		dst[next - 1] = digits[i];
	}
	return length;
}

static inline int
sn_encode_array(snbuf *b, int count)
{
	int rc = sn_bufensure(b, 64);
	if (snunlikely(rc == -1))
		return -1;
	char *p = b->p;
	*p = '*';
	p++;
	p += sn_encode_number(p, count);
	*p = '\r';
	p++;
	*p = '\n';
	sn_bufadvance(b, p - b->p);
	return p - b->p;

			/*
	int len = snprintf(sz, sizeof(sz), "*%d\r\n", count);
	return sn_bufadd(b, sz, len);
	*/
}
#endif

static inline void
sn_encode_array_update(snbuf *b, int offset, int count)
{
	snprintf(b->s + offset, 6 + 3, "*%06d\r\n", count);
}

static inline int
sn_encode_array_reserve(snbuf *b)
{
	char sz[32];
	int len = snprintf(sz, sizeof(sz), "*000000\r\n");
	return sn_bufadd(b, sz, len);
}

static inline int
sn_encode_array(snbuf *b, int count)
{
	char sz[32];
	int len = snprintf(sz, sizeof(sz), "*%d\r\n", count);
	return sn_bufadd(b, sz, len);
}

static inline int
sn_encode_int(snbuf *b, int integer)
{
	char sz[32];
	int len = snprintf(sz, sizeof(sz), ":%d\r\n", integer);
	return sn_bufadd(b, sz, len);
}

static inline int
sn_encode_bin(snbuf *b, void *ptr, int size)
{
	char sz[32];
	int len = snprintf(sz, sizeof(sz), "$%d\r\n", size);
	if (snunlikely(sn_bufadd(b, sz, len) == -1))
		return -1;
	if (snunlikely(sn_bufadd(b, ptr, size) == -1))
		return -1;
	return sn_bufadd(b, "\r\n", 2);
}

static inline int
sn_encode_nil(snbuf *b)
{
	char sz[32];
	int len = snprintf(sz, sizeof(sz), "$-1\r\n");
	return sn_bufadd(b, sz, len);
}

static inline int
sn_encode_string(snbuf *b, void *ptr, int size)
{
	char sz[32];
	int len = snprintf(sz, sizeof(sz), "+");
	if (snunlikely(sn_bufadd(b, sz, len) == -1))
		return -1;
	if (snunlikely(sn_bufadd(b, ptr, size) == -1))
		return -1;
	return sn_bufadd(b, "\r\n", 2);
}

static inline int
sn_encode_stringf(snbuf *b, char *fmt, ...)
{
	char msg[256];
	va_list args;
	va_start(args, fmt);
	int len = vsnprintf(msg, sizeof(msg), fmt, args);
	va_end(args);
	return sn_encode_string(b, msg, len);
}

static inline int
sn_encode_error(snbuf *b, void *ptr, int size)
{
	char sz[32];
	int len = snprintf(sz, sizeof(sz), "-");
	if (snunlikely(sn_bufadd(b, sz, len) == -1))
		return -1;
	if (snunlikely(sn_bufadd(b, ptr, size) == -1))
		return -1;
	return sn_bufadd(b, "\r\n", 2);
}

static inline int
sn_encode_errorf(snbuf *b, char *fmt, ...)
{
	char msg[256];
	va_list args;
	va_start(args, fmt);
	int len = vsnprintf(msg, sizeof(msg), fmt, args);
	va_end(args);
	return sn_encode_error(b, msg, len);
}

#define SN_PINCOMPLETE (-1)
#define SN_PERROR (-2)

static inline int
sn_decode_has(char *start, char *end, int size)
{
	return (end - start) >= size;
}

static inline int
sn_decode_next(char **p, char *end)
{
	if (! sn_decode_has(*p, end, 1))
		return SN_PINCOMPLETE;
	int rc = SN_PERROR;
	switch (**p) {
	case '*': rc = SN_ARRAY;
		break;
	case '$': rc = SN_BIN;
		break;
	case '+': rc = SN_STRING;
		break;
	case ':': rc = SN_INT;
		break;
	case '-': rc = SN_ERROR;
		break;
	}
	(*p)++;
	return rc;
}

static inline unsigned long long
sn_decode_number(char *p, char *end)
{
	unsigned long long num = 0;
	while (p < end) {
		num = (num * 10) + (*p - '0');
		p++;
	}
	return num;
}

static inline int
sn_decode_len(char **p, char *end)
{
	int len = 0;
	char *c = *p;
	while (c < end && *c != '\r') {
		len = (len * 10) + (*c - '0');
		c++;
	}
	*p = c;
	if (*c == '\r')
		return len;
	return SN_PINCOMPLETE;
}

static inline int
sn_decode_crlf(char **p, char *end)
{
	char *start = *p;
	if (! sn_decode_has(start, end, 2))
		return SN_PINCOMPLETE;
	if ( *start != '\r')
		return SN_PERROR;
	if (*(start + 1) != '\n')
		return SN_PERROR;
	*p += 2;
	return 0;
}

static inline int
sn_decode_int(char **p, char *end)
{
	int len = sn_decode_len(p, end);
	if (len < 0)
		return len;
	int rc = sn_decode_crlf(p, end);
	if (rc < 0)
		return rc;
	return len;
}

static inline int
sn_decode_array(char **p, char *end) {
	return sn_decode_int(p, end);
}

static inline int
sn_decode_string(char **p, char *end, char **result)
{
	char *start = *p;
	char *c = *p;
	*result = NULL;
	while (c < end && *c != '\r') {
		c++;
	}
	*p = c;
	if (*c != '\r')
		return SN_PINCOMPLETE;
	int rc = sn_decode_crlf(p, end);
	if (rc < 0)
		return rc;
	*result = start;
	return c - start;
}

static inline int
sn_decode_value(char **p, char *end, int size, char **result)
{
	char *start = *p;
	if (! sn_decode_has(start, end, size))
		return SN_PINCOMPLETE;
	*p += size;
	int rc = sn_decode_crlf(p, end);
	if (rc < 0)
		return rc;
	*result = start;
	return size;
}

static inline int
sn_decode_bin(char **p, char *end, char **result)
{
	int len = sn_decode_int(p, end);
	if (len < 0)
		return len;
	return sn_decode_value(p, end, len, result);
}

#endif
