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

#define DUMP_BOARD_NAME		(0x01)
#define DUMP_SYSTEM_NAME	(0x02)
#define DUMP_MAC_ADDRESS	(0x04)
#define MAC_ADDRESS_LENGTH 	6

int quiet = 0;
int verbose = 0;

struct FRU_COMMON_HEADER {
	unsigned char format_ver; 		/* Common Header Format Version */
	unsigned char internal_offset;		/* Internal Use Area Starting. */
	unsigned char chassis_offset;		/* Chassis Info Area Starting Offset */
	unsigned char board_offset;		/* Board Area Starting Offset */
	unsigned char product_offset;		/* Product Info Area Starting Offset */
	unsigned char multirecord_offset;	/* MultiRecord Area Starting Offset */
	unsigned char pad;			/* PAD */
	unsigned char checksum;			/* Common Header Checksum */
};

void usage (void)
{
	printf("Hello World");
}

int main(int argc, char **argv) {

	int fru_dev_fp = 0;
	char *input_file = NULL;
	int mac_index = 0;
	int c;
	int dump = 0;

	opterr = 0;
	while ((c = getopt (argc, argv, "bsv?i:m:")) != -1)
	switch (c) {
		case 'b':
			dump |= DUMP_BOARD_NAME;
			break;
		case 's':
			dump |= DUMP_SYSTEM_NAME;
			break;
		case 'v':
			verbose = 1;
			break;
		case 'i':
			input_file = optarg;
			break;
		case 'm':
			dump |= DUMP_MAC_ADDRESS;
			mac_index = atoi(optarg);
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

	if (input_file) {
		fru_dev_fp = open(input_file, O_RDONLY);
		if (fru_dev_fp < 0) {
			printf("Unable to open device\n");
			exit(EXIT_FAILURE);
		}
		if (dump & (DUMP_BOARD_NAME | DUMP_SYSTEM_NAME)) {
			//struct FRU_COMMON_HEADER* fru_c_header = (struct FRU_COMMON_HEADER*) malloc(sizeof(struct FRU_COMMON_HEADER));
			struct FRU_COMMON_HEADER fru_c_header;
			char output[32] = {0};
			unsigned int offset_mfg_tlen_filed = 0;
			unsigned int tlen_mfg = 0;
			unsigned int tlen_product_name = 0;

			lseek(fru_dev_fp, 0x00, SEEK_SET);
			read(fru_dev_fp, &fru_c_header, sizeof(struct FRU_COMMON_HEADER));
			if (DUMP_BOARD_NAME == dump && DUMP_BOARD_NAME) {
				offset_mfg_tlen_filed = fru_c_header.board_offset * 8 + 6;
			} else if (DUMP_SYSTEM_NAME == dump && DUMP_SYSTEM_NAME) {
				offset_mfg_tlen_filed = fru_c_header.product_offset * 8 + 6;
			} else {

			}
			lseek(fru_dev_fp, offset_mfg_tlen_filed, SEEK_SET);
			read(fru_dev_fp, &tlen_mfg, 1);
			tlen_mfg &= 0x1F;
			const unsigned int offset_product_name_tlen_field = offset_mfg_tlen_filed + tlen_mfg + 1;
			lseek(fru_dev_fp, offset_product_name_tlen_field , SEEK_SET);
			read(fru_dev_fp, &tlen_product_name, 1);
			tlen_product_name &= 0x1F;
			lseek(fru_dev_fp, offset_product_name_tlen_field + 1, SEEK_SET);
			read(fru_dev_fp, output, tlen_product_name);
			output[tlen_product_name] = 0;
			printf("%s\n", output);
		} else if (dump & DUMP_MAC_ADDRESS) {
			char mac_addr[MAC_ADDRESS_LENGTH] = {0};
			const unsigned int offset_mac_addr = 0x2040 + mac_index * MAC_ADDRESS_LENGTH;
			lseek(fru_dev_fp, offset_mac_addr, SEEK_SET);
			read(fru_dev_fp, mac_addr, MAC_ADDRESS_LENGTH);
			printf("%02x:%02X:%02X:%02X:%02X:%02X\n",  mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
		}
	} else {
		printf("no input file specified\n");
		exit(EXIT_FAILURE);
	}

	close(fru_dev_fp);
	exit(EXIT_SUCCESS);

}
