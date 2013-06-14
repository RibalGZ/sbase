/* See LICENSE file for copyright and license details. */
#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "util.h"

static void
usage(void)
{
	eprintf("usage: %s string\n", argv0);
	exit(1);
}

int
main(int argc, char *argv[])
{
	ARGBEGIN {
	default:
		usage();
	} ARGEND;

	if(argc < 1)
		usage();

	puts(dirname(argv[optind]));

	return 0;
}

