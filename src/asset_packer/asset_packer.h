#ifndef __ASSET_PACKER_H__
#define __ASSET_PACKER_H__

typedef uint32_t Asset_Offset; // Kind of weird. ftell returns a long int, but we don't care about negative values. Should be fine as long as the 
                               // machine that does the packing has a long representation <= 64 bits.

typedef int Asset_ID;

struct Anim_Header {
	int num_frames;
	Asset_ID spritesheet_id;
};

struct Frame_Info {
	int delay, x, y, w, h;
};

struct Image_Header {
	int w, h, bytes_per_row;
	uint32_t pixel_format;
};

struct Map_Header {
	Asset_ID tilesheet_id; // Only one texture for the entire map for now.
	int num_rows, num_cols, tile_width, tile_height;
};

#endif
