/*
 * common.h
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


#ifndef COMMON_H
#define COMMON_H
 
int	strong_random_init(void);
void	strong_random_destroy(void);
int	strong_random_reseed(void);
int	strong_random_get(unsigned char *outbuf, const size_t outbuf_size);

void	terminate(const int exit_code);
char	*format_msg (const char *, va_list);
void	error(const char *, ...);
void	warning(const char *, ...);
void	fatal(const char *, ...);
void	*xmalloc(size_t);
void	*xcalloc(size_t, size_t);

#endif
