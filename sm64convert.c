#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "types.h"
#include "savefile.h"

#define MENU_DATA_MAGIC 0x4849
#define SAVE_FILE_MAGIC 0x4441
#define bswap16 __builtin_bswap16
#define bswap32 __builtin_bswap32
#define bswap64 __builtin_bswap64

bool little_endian = false;

static u16 checksum(const u8* data, s32 size) {
	u16 sum = 0;

	for (s32 i = 0; i < size - 2; i++) {
		sum += data[i];
	}

	return sum;
}

bool validate_save(struct SaveBuffer* save) {
	bool valid = true;

	for (size_t i = 0; i < 2; i++) {
		struct MainMenuSaveData* data = &save->menuData[i];
		u16 magic = data->signature.magic;

		s32 sum = checksum((u8*) data, sizeof(struct MainMenuSaveData));
		if (!(data->signature.chksum == sum || data->signature.chksum == bswap16(sum))) {
			fprintf(stderr, "Main menu block %ld is invalid: Bad checksum\n", i);
			valid = false;
		}
		if (!(magic == MENU_DATA_MAGIC || magic == bswap16(MENU_DATA_MAGIC))) {
			fprintf(stderr, "Main menu block %ld is invalid: Bad magic\n", i);
			valid = false;
		}
	}

	for (size_t i = 0; i < NUM_SAVE_FILES; i++) {
		for (size_t j = 0; j < 2; j++) {
			struct SaveFile* file = &save->files[i][j];
			s16 sum = checksum((u8*) file, sizeof(struct SaveFile));
			u16 magic = file->signature.magic;

			if (!(file->signature.chksum == sum || file->signature.chksum == bswap16(sum))) {
				fprintf(stderr, "%s save file %ld is invalid.\n", j == 0 ? "Main" : "Backup", i + 1);
				valid = false;
			}
			if (!(magic == SAVE_FILE_MAGIC || magic == bswap16(SAVE_FILE_MAGIC))) {
				fprintf(stderr, "Main menu block %ld is invalid: Bad magic\n", i);
				valid = false;
			}
		}
	}

	return valid;
}

void byteswap_save_file(struct SaveFile* file) {
	for (int i = 0; i < 3; i++)
		file->capPos[i] = bswap16(file->capPos[i]);
	file->flags = bswap32(file->flags);
	file->signature.magic = bswap16(file->signature.magic);
	file->signature.chksum = bswap16(file->signature.chksum);
}

void byteswap_main_menu_data(struct MainMenuSaveData* file) {
	for (int i = 0; i < NUM_SAVE_FILES; i++)
		file->coinScoreAges[i] = bswap32(file->coinScoreAges[i]);
	file->soundMode = bswap16(file->soundMode);
	file->language = bswap16(file->language);
	file->signature.magic = bswap16(file->signature.magic);
	file->signature.chksum = bswap16(file->signature.chksum);
}

int main(int argc, char** argv) {
	if (argc < 2) {
		fprintf(stderr, "Usage: %s <infile> [outfile]\n", argv[0]);
		fprintf(stderr, "If output file is not specified, the save file will be converted in place.\n");
		return 1;
	}

	const char* inname = argv[1];
	const char* outname = argc == 3 ? argv[2] : argv[1];
	struct SaveBuffer save;

	FILE* infile = fopen(inname, "r");
	if (!infile) {
		perror(inname);
		return 1;
	}

	size_t read = fread(&save, sizeof(struct SaveBuffer), 1, infile);
	if (read < 1) {
		fprintf(stderr, "Input file too small.");
		return 1;
	}
	fclose(infile);

	// First check if all save blocks are valid
	printf("Validating input save file: ");
	bool valid = validate_save(&save);

	if (valid) {
		printf("OK\n");
	} else {
		printf("Failed\n");
		printf("Save file is probably corrupted. Try again after loading it once in the game.\n");
		return 1;
	}

	u16 endian_test = 0x0100;
	u8 host_endian = *((u8*) &endian_test);

	little_endian = (save.menuData[0].signature.magic == MENU_DATA_MAGIC) ^ host_endian;

	printf("Input file endianness: %s endian\n", little_endian ? "little" : "big");

	for (int i = 0; i < 2; i++) {
		byteswap_main_menu_data(&save.menuData[i]);
	}
	for (int i = 0; i < NUM_SAVE_FILES; i++) {
		for (int j = 0; j < 2; j++) {
			byteswap_save_file(&save.files[i][j]);
		}
	}

	FILE* outfile = fopen(outname, "w");
	if (!outfile) {
		perror(outname);
		return 1;
	}

	size_t written = fwrite(&save, sizeof(struct SaveBuffer), 1, outfile);
	if (written != 1) {
		perror("Could not write output file");
	}

	printf("\nSaved %s endian save file to: %s\n", little_endian ? "big" : "little", outname);

	return 0;
}
