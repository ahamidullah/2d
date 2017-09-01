#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <limits.h>

#include "math.h"
#include "asset.h"

Vec2i screen_dim = { 1920, 1080 };

#include "lib.cpp"
#include "render.cpp"
#include "animation.cpp"
#include "asset.cpp"

typedef uint32_t Collider_ID;

struct Transform {
	Vec2f position;
	Vec2f velocity;
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
	Collider_ID collider_id;
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

struct Tweak_Decl {
	const char *name;
	void *mem;
};

size_t num_tweaks = 0;
#define MAX_TWEAKS 256
Tweak_Decl tweaks[MAX_TWEAKS]; // @TEMP

#define tweak_attach(name, mem) tweaks[num_tweaks++] = { #name, mem }

#include <sys/inotify.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/signal.h>

void
reload_tweaks()
{
	FILE *fp = fopen("tweaks.var", "r");
	if (!fp)
		zabort("Failed to open tweak file - %s", strerror(errno));
	char tweak_name[256];
	float val;
	for (int i = i; i < num_tweaks; ++i) {
		if (fscanf(fp, "%s %f\n", tweak_name, &val) != 2)
			zerror("a;lskfj");
		*(float *)tweaks[i].mem = val;
	}
}

void
sigio_handler(int signo, siginfo_t *, void *)
{
	printf("haha\n");
	reload_tweaks();
}

void
tweak_init()
{
	tweak_attach(scale, &scale);

	FILE *fp = fopen("tweaks.var", "r");
	if (!fp)
		zabort("Failed to open tweak file - %s", strerror(errno));
	char tweak_name[256];
	float val;
	for (int i = 0; fscanf(fp, "%s %f\n", tweak_name, &val) != EOF;) {
		for (int j = i; j < num_tweaks; ++j) {
			if (strcmp(tweak_name, tweaks[j].name) == 0) {
				*(float *)tweaks[j].mem = val;
				Tweak_Decl tmp = tweaks[i];
				tweaks[i] = tweaks[j];
				tweaks[j] = tmp;
				++i;
				break;
			}
			if (j == num_tweaks-1)
				zerror("Could not find tweak declaration matching name '%s'", tweak_name);
		}
	}

	int fd;
	if ((fd = open(".", O_RDONLY)) < 0)
		zabort("Could not open the tweak file descriptor - %s", strerror(errno));
	if (fcntl(fd, F_SETSIG, SIGRTMIN+1) == -1)
		zabort("Could not F_SETSIG the tweak file descriptor - %s", strerror(errno));
	// VIM creates a new "tweaks.var" file whenever the tweaks.var is edited, so just watch the dir and reload tweaks when the old file is overwritten.
	if (fcntl(fd, F_NOTIFY, DN_DELETE | DN_MULTISHOT))
		zabort("Could not F_NOTIFY the tweak file descriptor - %s", strerror(errno));
	struct sigaction sa;
	sa.sa_sigaction = sigio_handler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_SIGINFO;
	sigaction(SIGRTMIN + 1, &sa, NULL);
}

int
main(int, char **)
{
	SDL_Window *window = sdl_init();
	render_init(window);
	asset_init();
	sim_init();
	anim_init();
	tweak_init();

	Player p;
	p.animation = anim_get(mc_walk_anim);
	p.transform = {{0.0f, 0.0f}, 1.0f};
	p.height = 1.0f;
	p.transform.velocity = {0.0f, 0.0f};
	//p.collider_id = make_collider(p.transform.position, { 50.0, 50.0 });

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

