#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#include "xml.h"
#include "math.h"
#include "asset.h"

Asset_Header asset_header;

Vec2i screen_dim = { 1920, 1080 };

#include "lib.cpp"
#include "render.cpp"
#include "animation.cpp"
#include "asset.cpp"

typedef SDL_Rect Box_Collider;
typedef size_t Collider_ID;
typedef size_t Sprite_ID;

struct Transform {
	Vec2f position;
	float scale;
};

struct Sprite {
	Transform transform;
	SDL_Texture *texture;
};

struct Player {
	float height;
	Transform transform;
	Animation animation;
};

struct Level {
	SDL_Rect **tiles;
	int tile_width, tile_height;
	int rows, cols;
	SDL_Texture *texture;
};

enum Keys {
	a_key = SDL_SCANCODE_A,
	s_key = SDL_SCANCODE_S,
	w_key = SDL_SCANCODE_W,
	d_key = SDL_SCANCODE_D,
};

#include "sim.cpp"

SDL_Window *
sdl_init()
{
	SDL_Window *window;
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
		zabort("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
	window = SDL_CreateWindow("2d", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, screen_dim.x, screen_dim.y, SDL_WINDOW_SHOWN);
	if(window == NULL)
		zabort("Window could not be created! SDL_Error: %s\n", SDL_GetError());
	return window;
}

int
main(int, char **)
{
	asset_header = asset_load_header();

	SDL_Window *window = sdl_init();
	render_init(window);
	sim_init();
	anim_init();

	Player p;
	p.animation = anim_get(mc_walk_anim);
	p.transform = {{250.0f, 250.0f}, 1.5f};
	p.height = 1.9f;
	//p.collider = make_collider(p.position, 50.0, 50.0);

	/*
	Level l;
	l.position = {250.0, 450.0};
	l.sprite = make_static_sprite({250, 450});
	p.collider = make_collider(l.position, 100.0, 50.0);
	*/

	SDL_Event event;
	bool running = true;
	bool keys[256] = {false};

	//parse_tmx();
	while (running) {
		while(SDL_PollEvent(&event) != 0) {
			if(event.type == SDL_QUIT) running = false;
			if(event.type == SDL_KEYDOWN) keys[event.key.keysym.scancode] = true;
			if(event.type == SDL_KEYUP) keys[event.key.keysym.scancode] = false;
		}
		sim(keys, &p);
		render();
	}

	SDL_Quit();
	SDL_DestroyWindow(window);
	render_quit();
	anim_quit();
	sim_quit();

	return 0;
}

