#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include "alg/alg.h"

#include "acg/sys.h"
#include "acg/types.h"
#include "acg/istr.h"
#include "acg/ds.h"
#include "acg/rand.h"

#include "namegen.h"

VBUF(names, istr, 64);

static int exists(const istr name)
{
	VBUF_FOREACH(names, istr) {
		if (*istr == name)
			return 1;
	}

	return 0;
}

int main(int argc, char **argv)
{
	init_interns();
	srand(time(0));

	const u32 n = MAX(1, MIN(64, argc > 1 ? atoi(argv[1]) : 8));

	for (u32 i = 0; i < n; ++i) {
		istr name = NULL;
		do {
			name = gen_name();
		} while (exists(name));
		printf("\t%s\n", name);
		*VBUF_PUSH(names) = name;
	}

	exit(0);
}
