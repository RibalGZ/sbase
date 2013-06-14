/* See LICENSE file for copyright and license details. */
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "text.h"
#include "util.h"

static long chars = 65;
static struct linebuf b = EMPTY_LINEBUF;

static long n_columns;
static long n_rows;

static void
usage(void)
{
	eprintf("usage: %s [-c chars] [FILE...]\n", argv0);
	exit(1);
}

int
main(int argc, char *argv[])
{
	long i, l, col;
	size_t maxlen = 0;
	char *space;
	FILE *fp;

	ARGBEGIN {
	case 'c':
		chars = estrtol(EARGF(usage()), 0);
		if(chars < 3)
			eprintf("%d: too few character columns");
		break;
	default:
		usage();
	} ARGEND;

	/* XXX librarify this chunk, too?  only useful in sponges though */
	if(argc == 0) {
		getlines(stdin, &b);
	} else for(; argc > 0; argc--, argv++) {
		if(!(fp = fopen(argv[0], "r")))
			eprintf("fopen %s:", argv[0]);
		getlines(fp, &b);
		fclose(fp);
	}

	for(l = 0; l < b.nlines; ++l) {
		size_t len = strlen(b.lines[l]);
		if(len > 0 && b.lines[l][len-1] == '\n')
			b.lines[l][--len] = '\0';
		if(len > maxlen)
			maxlen = len;
		if(maxlen > (chars - 1) / 2)
			break;
	}

	n_columns = (chars + 1) / (maxlen + 1);
	if(n_columns <= 1) {
		for(l = 0; l < b.nlines; ++l) {
			fputs(b.lines[l], stdout);
		}
		return 0;
	}

	if(!(space = malloc(maxlen + 2)))
		eprintf("malloc:");
	memset(space, ' ', maxlen + 1);
	space[maxlen + 1] = '\0';

	n_rows = (b.nlines + (n_columns - 1)) / n_columns;
	for(i = 0; i < n_rows; ++i) {
		for(l = i, col = 1; l < b.nlines; l += n_rows, ++col) {
			/*sprintf(b.lines[l], "%ld,%ld$", i, col);*/
			fputs(b.lines[l], stdout);
			if(col < n_columns)
				fputs(space + strlen(b.lines[l]), stdout);
		}
		fputs("\n", stdout);
	}

	return 0;
}

