/*
 * fru-simple-read
 *
 */


#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#define DUMP_REG 		1
#define PASSTHROUGH_MODE 	2


int quiet = 0;
int verbose = 0;

void usage (void)
{
	printf("Hello World");
}

int main(int argc, char **argv) {

	int c;
	int action = 0;
	unsigned int dump = 0;
	char* regnum;
	unsigned int* REG = 0x1e6e251c;

	opterr = 0;
	while ((c = getopt (argc, argv, "pv?r:")) != -1)
	switch (c) {
		case 'p':
			dump |= PASSTHROUGH_MODE;
			break;
		case 'v':
			verbose = 1;
			break;
		case 'r':
			dump |= DUMP_REG;
			regnum = optarg;
			break;
		case '?':
		case 'h':
			usage();
			exit(EXIT_SUCCESS);
			break;
		default:
			printf("Unknown option: %c\n", c);
			usage();
			exit(EXIT_FAILURE);
	}
	printf("regnum: %s\n", regnum);
	printf("REG: %x", &REG);

	exit(EXIT_SUCCESS);

}
