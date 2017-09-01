#ifndef __ASSET_H__
#define __ASSET_H__

#include "../asset_packer/asset_packer.h"

enum Anim_ID : Asset_ID {
	mc_idle_anim,
	mc_walk_anim,
	NUM_ANIMATIONS
};

enum Image_ID : Asset_ID {
	tiles_img,
	mc_img,
	NUM_IMAGES
};

struct Asset_Header {
	Asset_Offset animation_table[NUM_ANIMATIONS];
	Asset_Offset image_table[NUM_IMAGES];
};

#endif
