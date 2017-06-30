#include "asset.h"
#include "errno.h"

Asset_Header asset_header;

void
read_assets(long offset, int origin, size_t sz, size_t count, void *mem, FILE *fp)
{
	if (fseek(fp, offset, origin) != 0)
		zabort("Could not seek in asset file - %s", strerror(errno));
	if (fread(mem, sz, count, fp) != count)
		zabort("Could not read all elements requested from the asset file - %s", strerror(errno));
}

FILE *
open_asset_file()
{
	FILE *fp = fopen("../../assets/assets.ahh", "rb");
	if (!fp)
		zabort("Could not open asset file - %s", strerror(errno));
}

void
asset_init()
{
	FILE *fp = open_asset_file();
	read_assets(-sizeof(asset_header), SEEK_END, sizeof(asset_header), 1, &asset_header, fp);
}

SDL_Texture *render_make_texture(int w, int h, int pitch, void *pixels);

// Pretty inefficient, loading piecemeal every time it's requested. Should load in bulk and cache eventually.
SDL_Texture *
asset_load_image(Image_ID id)
{
	FILE *fp = open_asset_file();
	DEFER(fclose(fp));
	Image_Header ih;
	read_assets(asset_header.image_table[id], SEEK_SET, sizeof(ih), 1, &ih, fp);

	size_t total_bytes = ih.h * ih.bytes_per_row;
	char *pixels = (char *)malloc(total_bytes); // @TEMP
	DEFER(free(pixels));
	read_assets(0, SEEK_CUR, 1, total_bytes, pixels, fp);

	return render_make_texture(ih, pixels);
}

// Pretty inefficient, loading piecemeal every time it's requested. Should load in bulk and cache eventually.
Animation_Info
asset_load_anim(Anim_ID id)
{
	FILE *fp = open_asset_file();
	DEFER(fclose(fp));
	Animation_Info anim;
	Anim_Header ah;

	read_assets(asset_header.animation_table[id], SEEK_SET, sizeof(ah), 1, &ah, fp);

	anim.num_frames = ah.num_frames;
	Frame_Info *fi = (Frame_Info *)malloc(sizeof(Frame_Info) * anim.num_frames); // @TEMP
	read_assets(0, SEEK_CUR, sizeof(Frame_Info), anim.num_frames, fi, fp);
	anim.texture = asset_load_image((Image_ID)ah.image_id);

	anim.frames = (SDL_Rect *)malloc(sizeof(SDL_Rect) * anim.num_frames); // @TEMP
	anim.frame_delay = fi[0].delay; // Assuming they are all the same for now...
	for (int i = 0; i < anim.num_frames; ++i) {
		anim.frames[i].x = fi[i].x;
		anim.frames[i].y = fi[i].y;
		anim.frames[i].w = fi[i].w;
		anim.frames[i].h = fi[i].h;
	}
	return anim;
}

