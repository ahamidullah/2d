uint64_t ticks_per_sec;
uint64_t last_sim_time = 0;

struct Polygon_Collider {
	int num_points;
	Vec2f *points;
};

// @TEMP
size_t num_colliders = 0;
Box_Collider box_colliders[256];
Polygon_Collider poly_colliders[256];

Level level;

using namespace tinyxml2;

// Hacky parser. This, along with tinyxml2, should get out of here and into an asset packer
// eventually!
/*
Level
parse_tmx()
{
	XMLDocument doc;
	doc.LoadFile("assets/maps/untitled.tmx");

	Level l;
	XMLElement *tileset_xml = doc.FirstChildElement("map")->FirstChildElement("tileset");
	tileset_xml->QueryIntAttribute("tilewidth", &l.tile_width);
	tileset_xml->QueryIntAttribute("tileheight", &l.tile_height);
	int num_tilesheet_cols;
	tileset_xml->QueryIntAttribute("columns", &num_tilesheet_cols);
	const char *tex_fname = tileset_xml->FirstChildElement("image")->Attribute("source");
	l.texture = render_get_texture(tex_fname);

	XMLElement *layer_xml = doc.FirstChildElement("map")->FirstChildElement("layer");
	layer_xml->QueryIntAttribute("width", &l.cols);
	layer_xml->QueryIntAttribute("height", &l.rows);
	l.tiles = (SDL_Rect **)malloc(sizeof(SDL_Rect *) * l.cols);
	for (int i = 0; i < l.cols; ++i)
		l.tiles[i] = (SDL_Rect *)malloc(sizeof(SDL_Rect) * l.rows);
	const char *data = layer_xml->FirstChildElement("data")->GetText();
	for (int i = 0; i < l.rows; ++i) {
		for (int j = 0; j < l.cols; ++j) {
			while (!isdigit(data[0]))
				data += 1;
			char ind_buf[16];
			sscanf(data, "%[0-9]", ind_buf);
			data += strlen(ind_buf);
			int tile_ind = atoi(ind_buf) - 1; // Make it 0 indexed;
			SDL_Rect tex_src;
			tex_src.x = (tile_ind % num_tilesheet_cols) * l.tile_width;
			tex_src.y = (tile_ind / num_tilesheet_cols) * l.tile_height;
			tex_src.w = l.tile_width;
			tex_src.h = l.tile_height;
			l.tiles[i][j] = tex_src;
		}
	}
	return l;
}
*/

void
sim_init()
{
//	level = parse_tmx();
	ticks_per_sec = SDL_GetPerformanceFrequency();
}

SDL_Rect
get_dest(Player *p)
{
	SDL_Rect frame = p->animation.info.frames[p->animation.cur_frame];
	return {
		p->transform.position.x,
		p->transform.position.y,
		frame.w * p->transform.scale,
		frame.h * p->transform.scale,
	};
}

void
sim(bool *keys, Player *p)
{
	const double delta_time = last_sim_time == 0 ? 0.0 : (double)(SDL_GetPerformanceCounter() - last_sim_time) / ticks_per_sec;

	//render_set_sprite(p->sprite_id, anim_get_frame(&p->animation), p->transform.scale);

	if (keys[a_key])
		p->transform.position.x -= 1200.0*delta_time;
	if (keys[d_key])
		p->transform.position.x += 1200.0*delta_time;
	if (keys[w_key])
		p->transform.position.y -= 1200.0*delta_time;
	if (keys[s_key])
		p->transform.position.y += 1200.0*delta_time;
	
	// IMPORTANT: This needs to happen right after the last use of delta time.
	last_sim_time = SDL_GetPerformanceCounter();
	////////////////////////

/*
	for (int i = 0; i < level.rows; ++i) {
		for (int j = 0; j < level.cols; ++j) {
			SDL_Rect dest = { j*level.tile_width, i*level.tile_height, level.tiles[i][j].w, level.tiles[i][j].h };
			render_add_command(level.texture, level.tiles[i][j], dest);
		}
	}
*/
	render_add_command(p->animation.info.texture, anim_get_frame(&p->animation), p->transform.position, p->height, p->animation.info.frames[0].w / p->animation.info.frames[0].h);
}

void
sim_quit()
{

}

