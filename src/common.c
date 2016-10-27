/*
 * common.c
 *
 * Copyright (C) 2016 - Victor Hugo Santos Pucheta <vctrhgsp@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

#include <errno.h>
#include <limits.h>
#include <strings.h>
#include <unistd.h>
#include <sys/syscall.h>
#ifndef SYS_getrandom
# include <fcntl.h>
#endif /* SYS_getrandom */

#include "common.h"

#ifdef SYS_getrandom

static int
getrandom_get(unsigned char *outbuf, const size_t outbuf_size)
{
	/* No, you don't need more than 64 bytes. */
	if (outbuf_size > 64) {
		return 1;
	}

	int ret = 0;
	for (;;) {
		ret = syscall(SYS_getrandom, outbuf, outbuf_size, 0);
		if (ret >= 0) {
			break;
		}

		if (errno != EINTR) {
			return 1;
		}
	}

	if (ret != (int) outbuf_size) {
		bzero(outbuf, outbuf_size);
		return 1;
	}

	return 0;
}

#else /* SYS_getrandom */

static int urandom_fd = -1;

static int urandom_open(void)
{       
	for (;;) {
		urandom_fd = open("/dev/urandom", O_RDONLY);
		if (urandom_fd >= 0) {
			break;
		}

		if (errno != EINTR) {
			return 1;
		}
	}

	return 0;
}

static void urandom_close(void)
{       
	if (urandom_fd != -1) {
		close(urandom_fd);
		urandom_fd = -1;
	}
}

static int urandom_get(unsigned char *outbuf, const size_t outbuf_size)
{       
	if (urandom_fd == -1) {
		return 1;
	}

	if (outbuf == NULL) {
		return 1;
	}

	if (outbuf_size > SSIZE_MAX || outbuf_size == 0) {
		return 1;
	}

	size_t bytes_needed = outbuf_size;
	unsigned char *cur_outbuf = outbuf;
	ssize_t nread = 0;
	while (bytes_needed > 0) {
		nread = read(urandom_fd, cur_outbuf, bytes_needed);
		if (nread < 0) {
			if (errno != EINTR) {
				goto fail_read;
			}

			continue;
		}

		/* EOF? */
		if (nread == 0) {
			goto fail_read;
		}

		bytes_needed -= nread;
		cur_outbuf += nread;
	}

	return 0;

fail_read:
	bzero(outbuf, outbuf_size);

	return 1;
}

#endif /* SYS_getrandom */

int
strong_random_init(void)
{
#ifndef SYS_getrandom
	return urandom_open();
#else /* SYS_getrandom */
	return 0;
#endif /* SYS_getrandom */
}

void
strong_random_destroy(void)
{
#ifndef SYS_getrandom
	urandom_close();
#endif /* SYS_getrandom */
}

int
strong_random_reseed(void)
{
	return 0;
}

int
strong_random_get(unsigned char *outbuf, const size_t outbuf_size)
{
#ifndef SYS_getrandom
	return urandom_get(outbuf, outbuf_size);
#else /* SYS_getrandom */
	return getrandom_get(outbuf, outbuf_size);
#endif /* SYS_getrandom */
}

void
terminate(const int exit_code)
{
	strong_random_destroy();
	exit(exit_code);
}

char *
format_msg (const char *format, va_list va)
{
	size_t size;
	char *msg;
	va_list va_copy;

	va_copy (va_copy, va);
	size = vsnprintf (NULL, 0, format, va_copy) + 1;
	va_end (va_copy);

	msg = xmalloc (size);
	vsnprintf (msg, size, format, va);
	return msg;
}

void
error(const char *format, ...)
{
	va_list va;
	char *msg;

	va_start(va, format);
	msg = format_msg(format, va);
	va_end(va);

	fprintf(stderr, "[ERROR]: %s\n", msg);
	free(msg);
}

void
warning(const char *format, ...)
{
	va_list va;
	char *msg;

	va_start(va, format);
	msg = format_msg(format, va);
	va_end(va);

	fprintf(stderr, "[WARNING]: %s\n", msg);
	free(msg);
}

void
fatal(const char *format, ...)
{
	va_list va;
	char *msg;

	va_start(va, format);
	msg = format_msg(format, va);
	va_end(va);

	fprintf(stderr, "[FATAL_ERROR]: %s\n", msg);
	free(msg);
	terminate(EXIT_FAILURE);
}

void *
xmalloc(size_t size)
{
	void *ptr;

	if (size == 0)
		fatal("Zero size");

	ptr = malloc(size);
	if (ptr == NULL)
		fatal("Can't allocate memory!");
	return ptr;
}

void *
xcalloc(size_t nmemb, size_t size)
{
	void *ptr;

	if (size == 0 || nmemb == 0)
		fatal("Zero size");

	ptr = calloc(nmemb, size);
	if (ptr == NULL)
		fatal("Can't allocate memory!");
	return ptr;
}

