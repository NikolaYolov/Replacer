#include <stdio.h>

#include "aut_table.h"
#include "translate.h"
#include "s_alloc.h"

int main(int argc, char * argv[]) {
	struct a_table translator;
	static const int init_size = 0x00080000; // 2^19 states initially
	FILE * dict = 0, * in = 0, * out = 0;

	if (argc != 4) {
		printf("Invalid argumets! Extected 3, provided: %d.\n", argc - 1);
		return 1;
	}

	dict = fopen(argv[1], "r");
	if (dict == 0) {
		printf("Unable to open the dict file. Argument value is %s.\n", argv[1]);
		return 1;
	}

	/* initialize the allocator */
	mem_init();

	/* initialize the table */
	a_create_n(&translator, init_size);

	/* step1: create a trie */
	create_trie(&translator, dict);

	/* step2: create the full automaton */
	add_fail_links(&translator);

	in = fopen(argv[2], "rb");
	if (in == 0) {
		printf("Unable to open the input file. Argument value is %s.\n", argv[2]);
		fclose(dict);
		a_free(&translator);
		mem_fin();
		return 1;
	}

	out = fopen(argv[3], "wb");
	if (out == 0) {
		printf("Unable to open the output file. Argument value is %s.\n", argv[3]);
		fclose(in);
		fclose(dict);
		a_free(&translator);
		mem_fin();
		return 1;
	}

	/* finally we are going to use the automaton */
	travers(&translator, in, out);

	/* finalize the table */
	a_free(&translator);

	/* finalize the allocator */
	mem_fin();
}

