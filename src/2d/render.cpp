SDL_Renderer *renderer;

float meters_to_pixels = 0.0f;
float scale_meters_to_pixels = 1.0f;

struct Render_Command {
	SDL_Texture *texture;
	SDL_Rect tex_src;
	SDL_Rect screen_dest;
};

// @TEMP
size_t num_render_commands = 0;
Render_Command render_queue[500];

const char *sprite_path = "assets/sprites/";

SDL_Texture *
render_make_texture(Image_Header ih, void *pixels)
{
	SDL_Texture *tex;
	if (!(tex = SDL_CreateTexture(renderer, ih.pixel_format, SDL_TEXTUREACCESS_STATIC, ih.w, ih.h)))
		zabort("%s", SDL_GetError());
	if (SDL_UpdateTexture(tex, NULL, pixels, ih.bytes_per_row))
		zabort("%s", SDL_GetError());
	return tex;
}

void
render_init(SDL_Window *w)
{
	renderer = SDL_CreateRenderer(w, -1, SDL_RENDERER_ACCELERATED);
	if(!renderer)
		zabort("Renderer could not be created! SDL Error: %s\n", SDL_GetError());
	SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0x00);
	if(!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG))
		zabort("SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError());

	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");
	// Compute meters to pixels.
	// Aspect aspect ratio we are going to design against.
	Vec2i logical_ratio = { 16, 9 };
	// Initial value of meters to pixels.
	meters_to_pixels = scale_meters_to_pixels * minf((float)screen_dim.x /  logical_ratio.x, (float)screen_dim.y / logical_ratio.y);
	printf("%f\n", meters_to_pixels);
}

void
render_quit()
{
	SDL_DestroyRenderer(renderer);
}

void
render()
{
	SDL_RenderClear(renderer);
	for (int i = 0; i < num_render_commands; ++i)
		SDL_RenderCopy(renderer, render_queue[i].texture, &render_queue[i].tex_src, &render_queue[i].screen_dest);
	SDL_RenderPresent(renderer);
	num_render_commands = 0;
}

void
render_add_command(SDL_Texture *tex, SDL_Rect src, SDL_Rect dest)
{
	render_queue[num_render_commands].texture = tex;
	render_queue[num_render_commands].tex_src = src;
	render_queue[num_render_commands].screen_dest = dest;
	++num_render_commands;
}
