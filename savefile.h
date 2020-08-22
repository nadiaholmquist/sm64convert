#pragma once

#include "types.h"

#define COURSE_COUNT 25
#define COURSE_STAGES_COUNT 15
#define NUM_SAVE_FILES 4

struct SaveBlockSignature {
	u16 magic;
	u16 chksum;
};

struct SaveFile {
	u8 capLevel;
	u8 capArea;
	Vec3s capPos;
	u32 flags;
	u8 courseStars[COURSE_COUNT];
	u8 courseCoinScores[COURSE_STAGES_COUNT];
	struct SaveBlockSignature signature;
};

struct MainMenuSaveData {
	u32 coinScoreAges[NUM_SAVE_FILES];
	u16 soundMode;
	u16 language;
	u8 filler[0x200 / 2 - 8 - 4 * (4 + sizeof(struct SaveFile))];

	struct SaveBlockSignature signature;
};

struct SaveBuffer {
	struct SaveFile files[NUM_SAVE_FILES][2];
	struct MainMenuSaveData menuData[2];
};
