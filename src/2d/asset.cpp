#include "asset_packer.h"
#include "asset.h"

Asset_Header
asset_load_header()
{
	Asset_Header ah;
	SDL_RWops *rw = SDL_RWFromFile("assets.ahh", "rb");
	DEFER(SDL_RWclose(rw));
	if (rw == NULL)
		zabort("Could not open asset file - %s", SDL_GetError());
	int64_t len = SDL_RWseek(rw, -sizeof(Asset_Header), RW_SEEK_END);
	if (len < 0)
		zabort("Could not seek in asset file - %s", SDL_GetError());
	size_t num_read = SDL_RWread(rw, &ah, sizeof(ah), 1);
	if (num_read != 1)
		zabort("Could not read asset file");
	return ah;
}

SDL_Texture *render_make_texture(int w, int h, int pitch, void *pixels);

// Pretty inefficient, loading piecemeal every time it's requested. Should load in bulk and cache eventually.
SDL_Texture *
asset_load_image(Image_ID id)
{
	SDL_RWops *rw = SDL_RWFromFile("assets.ahh", "rb");
	DEFER(SDL_RWclose(rw));

	SDL_RWseek(rw, asset_header.image_table[id], RW_SEEK_SET);
	Image_Header ih;
	size_t num_read = SDL_RWread(rw, &ih, sizeof(ih), 1);
	if (num_read != 1) {
		zerror("Could not read image header %d from asset file", id);
		return NULL;
	}

	size_t total_bytes = ih.h * ih.bytes_per_row;
	char *pixels = (char *)malloc(total_bytes);
	DEFER(free(pixels));
	num_read =  SDL_RWread(rw, pixels, 1, total_bytes);
	if (num_read != total_bytes) {
		zerror("Could not read pixels from image %d from asset file", id);
		return NULL;
	}

	return render_make_texture(ih, pixels);
}

// Pretty inefficient, loading piecemeal every time it's requested. Should load in bulk and cache eventually.
Animation_Info
asset_load_anim(Anim_ID id)
{
	Animation_Info anim;

	SDL_RWops *rw = SDL_RWFromFile("assets.ahh", "rb");
	DEFER(SDL_RWclose(rw));

	SDL_RWseek(rw, asset_header.animation_table[id], RW_SEEK_SET);
	Anim_Header ah;
	size_t num_read = SDL_RWread(rw, &ah, sizeof(ah), 1);
	if (num_read != 1)
		zerror("Could not read animation header %d from asset file", id);
	anim.texture = asset_load_image((Image_ID)ah.image_id);
	anim.num_frames = ah.num_frames;
	Frame_Info *fi = (Frame_Info *)malloc(sizeof(Frame_Info) * anim.num_frames); // @TEMP
	num_read = SDL_RWread(rw, fi, sizeof(Frame_Info), anim.num_frames);
	if (num_read != anim.num_frames)
		zerror("Could not read frame info of animation %d from asset file", id);

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

