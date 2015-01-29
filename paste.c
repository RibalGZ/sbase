/* See LICENSE file for copyright and license details. */
#include <locale.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "utf.h"
#include "util.h"

struct fdescr {
	FILE *fp;
	const char *name;
};

static size_t
resolveescapes(char *s)
{
        size_t len, i, off, m;

	len = strlen(s);

        for (i = 0; i < len; i++) {
                if (s[i] != '\\')
                        continue;
                off = 0;

                switch (s[i + 1]) {
                case '\\': s[i] = '\\'; off++; break;
                case 'a':  s[i] = '\a'; off++; break;
                case 'b':  s[i] = '\b'; off++; break;
                case 'f':  s[i] = '\f'; off++; break;
                case 'n':  s[i] = '\n'; off++; break;
                case 'r':  s[i] = '\r'; off++; break;
                case 't':  s[i] = '\t'; off++; break;
                case 'v':  s[i] = '\v'; off++; break;
                case '\0':
                        eprintf("paste: null escape sequence in delimiter\n");
                default:
                        eprintf("paste: invalid escape sequence '\\%c' in "
                                "delimiter\n", s[i + 1]);
                }

                for (m = i + 1; m <= len - off; m++)
                        s[m] = s[m + off];
                len -= off;
        }

        return len;
}

static void
sequential(struct fdescr *dsc, int fdescrlen, Rune *delim, size_t delimlen)
{
	Rune c, last;
	size_t i, d;

	for (i = 0; i < fdescrlen; i++) {
		d = 0;
		last = 0;

		while (readrune(dsc[i].name, dsc[i].fp, &c)) {
			if (last == '\n') {
				if (delim[d] != '\0')
					writerune("<stdout>", stdout, &delim[d]);
				d = (d + 1) % delimlen;
			}

			if (c != '\n')
				writerune("<stdout>", stdout, &c);
			last = c;
		}

		if (last == '\n')
			writerune("<stdout>", stdout, &last);
	}
}

static void
parallel(struct fdescr *dsc, int fdescrlen, Rune *delim, size_t delimlen)
{
	Rune c, d;
	size_t i, m;
	ssize_t last;

nextline:
	last = -1;

	for (i = 0; i < fdescrlen; i++) {
		d = delim[i % delimlen];
		c = 0;

		for (; readrune(dsc[i].name, dsc[i].fp, &c) ;) {
			for (m = last + 1; m < i; m++)
				writerune("<stdout>", stdout, &(delim[m % delimlen]));
			last = i;
			if (c == '\n') {
				if (i != fdescrlen - 1)
					c = d;
				writerune("<stdout>", stdout, &c);
				break;
			}
			writerune("<stdout>", stdout, &c);
		}

		if (c == 0 && last != -1) {
			if (i == fdescrlen - 1)
				putchar('\n');
			else
				writerune("<stdout>", stdout, &d);
				last++;
		}
	}
	if (last != -1)
		goto nextline;
}

static void
usage(void)
{
	eprintf("usage: %s [-s] [-d list] file ...\n", argv0);
}

int
main(int argc, char *argv[])
{
	struct fdescr *dsc;
	Rune  *delim;
	size_t i, len;
	int    seq = 0;
	char  *adelim = "\t";

	ARGBEGIN {
	case 's':
		seq = 1;
		break;
	case 'd':
		adelim = EARGF(usage());
		break;
	default:
		usage();
	} ARGEND;

	if (argc == 0)
		usage();

	/* populate delimiters */
	resolveescapes(adelim);
	len = chartorunearr(adelim, &delim);

	/* populate file list */
	dsc = emalloc(argc * sizeof(*dsc));

	for (i = 0; i < argc; i++) {
		if (strcmp(argv[i], "-") == 0)
			dsc[i].fp = stdin;
		else
			dsc[i].fp = fopen(argv[i], "r");

		if (!dsc[i].fp)
			eprintf("fopen %s:", argv[i]);

		dsc[i].name = argv[i];
	}

	if (seq)
		sequential(dsc, argc, delim, len);
	else
		parallel(dsc, argc, delim, len);

	for (i = 0; i < argc; i++) {
		if (dsc[i].fp != stdin)
			(void)fclose(dsc[i].fp);
	}

	return 0;
}
