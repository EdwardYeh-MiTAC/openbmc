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
#include <time.h>
#include <stdlib.h>

#define DUMP_BOARD_NAME				(0x01)
#define DUMP_SYSTEM_NAME			(0x02)
#define DUMP_MAC_ADDRESS			(0x04)
#define DUMP_MAC_ADDRESS_FROM_CHASSIS_INFO	(0x08)
#define DUMP_MAC_ADDRESS_FROM_OFFSET		(0x10)
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

void usage (void) {
	printf("Hello World");
}

unsigned long mix(unsigned long a, unsigned long b, unsigned long c)
{
    a=a-b;  a=a-c;  a=a^(c >> 13);
    b=b-c;  b=b-a;  b=b^(a << 8);
    c=c-a;  c=c-b;  c=c^(b >> 13);
    a=a-b;  a=a-c;  a=a^(c >> 12);
    b=b-c;  b=b-a;  b=b^(a << 16);
    c=c-a;  c=c-b;  c=c^(b >> 5);
    a=a-b;  a=a-c;  a=a^(c >> 3);
    b=b-c;  b=b-a;  b=b^(a << 10);
    c=c-a;  c=c-b;  c=c^(b >> 15);
    return c;
}


void convertMAC(const char* mac_ascii, char* mac_addr) {
	for (unsigned int index = 0; index < 12; index ++ ){
		unsigned int oct = rand() % 16;  //Default for worst case, it replace invalid data.
		if ((mac_ascii[index] >= 'a') && (mac_ascii[index] <= 'f')) {
			oct = mac_ascii[index] - 'a' + 10;
		} else if ((mac_ascii[index] >= 'A') && (mac_ascii[index] <= 'F')) {
			//Convert to lower case
			oct = mac_ascii[index] - 'A' + 10;
		} else if ((mac_ascii[index] >= '0') && (mac_ascii[index] <= '9')) {
			oct = mac_ascii[index] - '0';
		}
		mac_addr[index/2] += oct << (4 * ((index + 1) % 2));
	}
}

int main(int argc, char **argv) {

	int fru_dev_fp = 0;
	char *input_file = NULL;
	int mac_index = 0;
	unsigned long fru_mac_offset = 0; //strtoul(hex_value1, NULL, 0);
	int c;
	int dump = 0;

	opterr = 0;
	while ((c = getopt (argc, argv, "bsv?i:m:M:o:")) != -1)
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
		case 'o':
			/* Get MAC Address from static address of MB FRU */
			dump |= DUMP_MAC_ADDRESS_FROM_OFFSET; 
			fru_mac_offset = strtol(optarg, NULL, 0);
			break;
		case 'm':
			/* Get MAC Address from static address of MB FRU */
			dump |= DUMP_MAC_ADDRESS; 
			mac_index = atoi(optarg);
			break;
		case 'M':
			/* Get MAC Address from Custom Chassis Info Field */
			dump |= DUMP_MAC_ADDRESS_FROM_CHASSIS_INFO; 
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
			printf("%02X:%02X:%02X:%02X:%02X:%02X\n",  mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
		} else if (dump & DUMP_MAC_ADDRESS_FROM_OFFSET) {
			char mac_addr[MAC_ADDRESS_LENGTH] = {0};
			lseek(fru_dev_fp, fru_mac_offset, SEEK_SET);
			read(fru_dev_fp, mac_addr, MAC_ADDRESS_LENGTH);
			printf("%02X:%02X:%02X:%02X:%02X:%02X\n",  mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
		} else if (dump & DUMP_MAC_ADDRESS_FROM_CHASSIS_INFO)  {
			struct FRU_COMMON_HEADER fru_c_header;
			unsigned int field_offset = 0;
			unsigned int field_len = 0;

			//Avoid using time for random seed when generate MAC address.
			unsigned long seed = mix(clock(), time(NULL), getpid());
			srand(seed);

			lseek(fru_dev_fp, 0x00, SEEK_SET);
			read(fru_dev_fp, &fru_c_header, sizeof(struct FRU_COMMON_HEADER));
			if (fru_c_header.chassis_offset == 0){
				printf("Chassis Info Area is not Found.\n");
				exit(EXIT_FAILURE);
			}
			//Chassis Part Number
			field_offset = fru_c_header.chassis_offset * 8 + 3;
			lseek(fru_dev_fp, field_offset, SEEK_SET);
			read(fru_dev_fp, &field_len, 1);
			//Chassis Serial Number
			field_offset += (field_len & 0x1F) + 1;
			lseek(fru_dev_fp, field_offset, SEEK_SET);
			read(fru_dev_fp, &field_len, 1);
			//Custom Chassis Info field 1
			field_offset += (field_len & 0x1F) + 1;
			lseek(fru_dev_fp, field_offset, SEEK_SET);
			read(fru_dev_fp, &field_len, 1);
			if (mac_index == 1) {
				char mac_addr[MAC_ADDRESS_LENGTH] = {0};
				const unsigned int LEN_MAC_ASCII = 0x0C;
				if ((field_len & 0x1F) == LEN_MAC_ASCII) {
					char mac_ascii[LEN_MAC_ASCII];
					read(fru_dev_fp, &mac_ascii, LEN_MAC_ASCII);
					convertMAC(mac_ascii, mac_addr);
					printf("%02X:%02X:%02X:%02X:%02X:%02X\n", mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
				} else {
					printf("%X%X:%X%X:%X%X:%X%X:%X%X:%X%X\n", rand() % 16, rand() % 16 ,
											rand() % 16 ,  rand() % 16,
											rand() % 16 ,  rand() % 16,
											rand() % 16 ,  rand() % 16,
											rand() % 16 ,  rand() % 16,
											rand() % 16 ,  rand() % 16);
				}
				close(fru_dev_fp);
				exit(EXIT_SUCCESS);
			}
			//Custom Chassis Info field 2
			field_offset += (field_len & 0x1F) + 1;
			lseek(fru_dev_fp, field_offset, SEEK_SET);
			read(fru_dev_fp, &field_len, 1);
			if (mac_index == 2)  {
				char mac_addr[MAC_ADDRESS_LENGTH] = {0};
				const unsigned int LEN_MAC_ASCII = 0x0C;
				if ((field_len & 0x1F) == LEN_MAC_ASCII) {
					char mac_ascii[LEN_MAC_ASCII];
					read(fru_dev_fp, &mac_ascii, LEN_MAC_ASCII);
					convertMAC(mac_ascii, mac_addr);
					printf("%02X:%02X:%02X:%02X:%02X:%02X\n", mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
				} else {
					printf("%X%X:%X%X:%X%X:%X%X:%X%X:%X%X\n", rand() % 16, rand() % 16 ,
											rand() % 16 ,  rand() % 16,
											rand() % 16 ,  rand() % 16,
											rand() % 16 ,  rand() % 16,
											rand() % 16 ,  rand() % 16,
											rand() % 16 ,  rand() % 16);
				}
				close(fru_dev_fp);
				exit(EXIT_SUCCESS);
			}
			printf("Index should be less than 3\n");
			exit(EXIT_FAILURE);	
		}
	} else {
		printf("no input file specified\n");
		exit(EXIT_FAILURE);
	}

	close(fru_dev_fp);
	exit(EXIT_SUCCESS);

} 
