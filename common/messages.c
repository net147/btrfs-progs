/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License v2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 021110-1307, USA.
 */

#include <stdio.h>
#include <stdarg.h>
#include "common/messages.h"
#include "common/utils.h"

static const char *common_error_string[] = {
	[ERROR_MSG_MEMORY]	= "not enough memory",
	[ERROR_MSG_START_TRANS] = "failed to start transaction",
	[ERROR_MSG_COMMIT_TRANS] = "failed to commit transaction",
};

__attribute__ ((format (printf, 1, 2)))
void __btrfs_printf(const char *fmt, ...)
{
	va_list args;

	va_start(args, fmt);
	vfprintf(stderr, fmt, args);
	va_end(args);
}

#ifndef PV_WORKAROUND

static int va_modifier = -1;

static int print_va_format(FILE *stream, const struct printf_info *info,
			   const void *const *args)
{
	const struct va_format *fmt;

	if (!(info->user & va_modifier))
		return -2;

	fmt = *((const struct va_format **)(args[0]));
	return vfprintf(stream, fmt->fmt, *(fmt->va));
}

static int print_va_format_arginfo(const struct printf_info *info,
				   size_t n, int *argtypes, int *size)
{
	if (n > 0) {
		argtypes[0] = PA_POINTER;
		size[0] = sizeof(struct va_format *);
	}
	return 1;
}
#endif

__attribute__ ((format (printf, 2, 3)))
void btrfs_no_printk(const void *fs_info, const char *fmt, ...)
{
	va_list args;

#ifndef PV_WORKAROUND
	if (va_modifier == -1) {
		register_printf_specifier('V', print_va_format,
					  print_va_format_arginfo);
		va_modifier = register_printf_modifier(L"p");
	}
#endif

	va_start(args, fmt);
	vfprintf(stderr, fmt, args);
	va_end(args);
	fputc('\n', stderr);
}

static bool should_print(int level)
{
	if (bconf.verbose == BTRFS_BCONF_QUIET || level == BTRFS_BCONF_QUIET)
		return false;

	if (bconf.verbose == BTRFS_BCONF_UNSET && level == LOG_DEFAULT)
		return true;
	if (bconf.verbose < level)
		return false;

	return true;
}

/*
 * Print a message according to the global verbosity level.
 *
 * level: minimum verbose level at which the message will be printed
 */
__attribute__ ((format (printf, 2, 3)))
void pr_verbose(int level, const char *fmt, ...)
{
	va_list args;

	if (!should_print(level))
		return;

	va_start(args, fmt);
	vfprintf(stdout, fmt, args);
	va_end(args);
}

/* Print common error message with optional data, appended after the generic text */
__attribute__ ((format (printf, 2, 3)))
void error_msg(enum common_error error, const char *msg, ...)
{
	const char *str = common_error_string[error];

	if (msg) {
		va_list args;

		va_start(args, msg);
		fprintf(stderr, PREFIX_ERROR "%s: ", str);
		vfprintf(stderr, msg, args);
		va_end(args);
		fputc('\n', stderr);
	} else {
		fprintf(stderr, PREFIX_ERROR "%s\n", str);
	}
}

/*
 * Print a message according to the global verbosity level - to stderr.
 *
 * level: minimum verbose level at which the message will be printed
 */
__attribute__ ((format (printf, 2, 3)))
void pr_stderr(int level, const char *fmt, ...)
{
	va_list args;

	if (!should_print(level))
		return;

	va_start(args, fmt);
	vfprintf(stderr, fmt, args);
	va_end(args);
}
