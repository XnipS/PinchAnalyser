#pragma once
#define Debug true
// Macros go here :)
#define FB_TARGET_TICKRATE 60
#define FB_TICKRATE_TIME (1000 / FB_TARGET_TICKRATE)
#define FB_DELTATIME (1.0 / FB_TARGET_TICKRATE)
#define FB_SIZE 100
#define FB_IMAGE_SCALE 5
#define FB_GRAVITY 1.0

#define FB_DAMPEN 0.5
#define FB_MOLECULE_COUNT 50
#define FB_MOLECULE_SPAWNRANDOM true
#define FB_MOLECULE_SIZE 1