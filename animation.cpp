struct Animation_Info {
	int num_frames;
	SDL_Rect *frames; // @TODO: use a variable sized struct to keep everything inline?
	unsigned frame_delay;
	SDL_Texture *texture;
};

struct Animation {
	Animation_Info info;
	int cur_frame;
	uint32_t frame_timer;
};

Animation_Info animations[NUM_ANIMATIONS];

SDL_Rect
anim_get_frame(Animation *a)
{
	if (!SDL_TICKS_PASSED(SDL_GetTicks(), a->frame_timer + a->info.frame_delay))
		return a->info.frames[a->cur_frame];
	SDL_Rect frame = a->info.frames[a->cur_frame];
	++a->cur_frame;
	if (a->cur_frame == a->info.num_frames)
		a->cur_frame = 0;
	a->frame_timer = SDL_GetTicks(); // @TODO: Wrong. We might not draw the first frame.
	return frame;
}

//asdff/

Animation
anim_get(Anim_ID id)
{
	Animation_Info *i = &animations[id];
	Animation a;
	memcpy(&a.info, i, sizeof(Animation_Info));
	a.cur_frame = 0;
	a.frame_timer = SDL_GetTicks();
	return a;
}

/*
void
get_line(char *buf, FILE *fp)
{
	int i = 0;
	for (int c = fgetc(fp); c != EOF && !isnewline(c); ++i, c = fgetc(fp))
		buf[i] = c;
	int test_dos_ending = fgetc(fp);
	if (test_dos_ending != '\n') ungetc(test_dos_ending, fp);
	buf[i] = '\0';
}

int
get_num_lines(FILE *fp)
{
	long reset = ftell(fp);
	int num_lines = 0, c;
	while ((c = fgetc(fp)) != EOF) {
		if (isnewline(c)) ++num_lines;
		if ((c = fgetc(fp)) != '\n') ungetc(c, fp); // Check for DOS '\r\n' line ending.
	}
	fseek(fp, reset, SEEK_SET);
	return num_lines;
}

void
eat_newlines(FILE *fp)
{
	int c;
	while ((c = fgetc(fp)) == '\n' || c == '\r')
		;
	ungetc(c, fp);
}

void
load_anim_from_file(Anim_ID id, const char *fname)
{
	char path[256];
	strcpy(path, anim_path);
	strcat(path, fname);
	strcat(path, ".csv");
	FILE *fp = fopen(path, "r+");
	DEFER(fclose(fp));
	if (!fp) {
		zerror("Failed to open file %s - %s", path, strerror(errno));
		return;
	}

	const char *expected_fmt = R"#("Delay(1/1000)","File Name","X","Y","Width","Height")#";
	char first_line[256];
	get_line(first_line, fp);
	if (strcmp(first_line, expected_fmt))
		zabort("Animation CSV format did not match expected format in file %s.\n\tGot: %s.\n\tExpected: %s", path, first_line, expected_fmt);
	
	char *tex_name = first_line;
	int frame_delay, tex_src_x, tex_src_y, tex_src_w, tex_src_h;
	int num_frames = get_num_lines(fp);
	animations[id].num_frames = num_frames;
	animations[id].frames = (SDL_Rect *)malloc(sizeof(SDL_Rect) * num_frames);
	int num_read = fscanf(fp, R"("%d","%[^"]","%d","%d","%d","%d")", &frame_delay, tex_name, &tex_src_x, &tex_src_y, &tex_src_w, &tex_src_h);
	if (num_read != 6) {
		zerror("Failed to parse the animation file %s\n", path);
		return;
	}
	animations[id].frame_delay = frame_delay; // Assume constant frame delay for the entire animation.
	animations[id].texture = render_get_texture(tex_name);

	int cur_frame = 0;
	do {
		animations[id].frames[cur_frame++] = {tex_src_x, tex_src_y, tex_src_w, tex_src_h};
		eat_newlines(fp);
	} while (fscanf(fp, R"("%d","%[^"]","%d","%d","%d","%d")", &frame_delay, tex_name, &tex_src_x, &tex_src_y, &tex_src_w, &tex_src_h) != EOF);
}
*/

void asset_load_anim(Anim_ID id, Animation_Info *anim);

void
anim_init()
{
	for (int i = 0; i < NUM_ANIMATIONS; ++i)
		asset_load_anim((Anim_ID)i, &animations[i]);
/*
#define LOAD_ANIMS
#include "animation.h"
#undef LOAD_ANIMS
*/
/*
	char fpath[256]; // @TEMP ?
	size_t dir_name_len = strlen(dir_name);
	strcpy(fpath, dir_name);
	FILE *file;
	struct dirent *ent;
	while ((ent = readdir(anim_dir))) {
		if (!strcmp(ent->d_name, ".") || !strcmp(ent->d_name, ".."))
			continue;
		strcat(fpath, ent->d_name);
		file = fopen(fpath, "r");
		DEFER(fclose(file));
		if (!file) {
			zerror("Failed to open file %s - %s", ent->d_name, strerror(errno));
			continue;
		}

		// Actually process the file.
		fpath[dir_name_len] = '\0';
	}
	animations[walk_anim].num_frames = 17;
	animations[walk_anim].frame_delay = 550;
	SDL_Rect *r = animations[walk_anim].frames = (SDL_Rect *)malloc(sizeof(SDL_Rect)*17);
	r[0] = {97,166,24,32};
	r[1] = {129,166,22,32};
	r[2] = {159,165,19,33};
	r[3] = {186,165,19,33};
	r[4] = {213,165,19,33};
	r[5] = {240,165,19,33};
	r[6] = {267,166,20,32};
	r[7] = {295,167,20,31};
	r[8] = {323,166,21,32};
	r[9] = {352,166,21,32};
	r[10] = {381,165,22,33};
	r[11] = {411,165,23,33};
	r[12] = {442,165,24,33};
	r[13] = {474,165,25,33};
	r[14] = {507,165,24,33};
	r[15] = {539,167,24,31};
	r[16] = {571,167,22,31};

	animations[walk_anim].texture = render_get_texture("sprite.png");
*/
}

void 
anim_quit()
{

}

