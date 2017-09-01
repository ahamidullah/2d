#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <string.h>
#include <stdarg.h>
#include <vector>
#include <string>
#include <map>
#include <dirent.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include "xml.h"
#include "asset_packer.h"

using namespace tinyxml2;

struct Dir_Read {
	DIR *dir = NULL;
	struct dirent *ent = NULL;
};

struct dirent *
each_file(Dir_Read *dr, std::string &path)
{
	if (!dr->dir) { // First read.
		dr->dir = opendir(path.c_str());
		if (!dr->dir) {
			printf("Failed to open animation directory %s - %s\n", path.c_str(), strerror(errno));
			return NULL;
		}
	}
	if (dr->ent)
		path.resize(path.size() - strlen(dr->ent->d_name) - 1);
	while ((dr->ent = readdir(dr->dir))) {
		if (!strcmp(dr->ent->d_name, ".") || !strcmp(dr->ent->d_name, ".."))
			continue;
		path += (std::string("/") + std::string(dr->ent->d_name));
		return dr->ent;
	}
}

std::string
get_base_name(std::string s)
{
	return s.substr(s.find_last_of('/')+1, s.find_last_of('.') - s.find_last_of('/') - 1);
}

/*
Map_Header
parse_tmx()
{
	XMLDocument doc;
	doc.LoadFile("../../assets/maps/untitled.tmx");

	Map_Header mh;
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
			while (!isdigit(data[0])) // Skip newlines, commas, etc.
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

FILE *gen_header_file;

void
gen_id(std::string name, const std::vector<std::string> &values)
{
	// Enum.
	{
		fprintf(gen_header_file, "enum %s : Asset_ID {\n", name.c_str());
	}

	// To string.
}

int
main(int, char **)
{
	std::map<std::string, Asset_ID> image_map;
	std::vector<Asset_Offset> anim_offsets;
	std::vector<Asset_Offset> image_offsets;

	FILE *asset_file;
	gen_header_file = fopen("../2d/asset.h", "w+");
	if (!gen_header_file) {
		printf("Failed to open gen_header_file - %s\n", strerror(errno));
		return 1;
	}

	fprintf(gen_header_file, "#ifndef __ASSET_H__\n");
	fprintf(gen_header_file, "#define __ASSET_H__\n\n");
	fprintf(gen_header_file, "#include \"../asset_packer/asset_packer.h\"\n\n");

	asset_file = fopen("../../assets/assets.ahh", "wb");
	if (!asset_file) {
		printf("Failed to open asset file - %s\n", strerror(errno));
		return 1;
	}

	// Animations.
	{
		const char *anim_base_path = "../../assets/animations";
		Dir_Read root_dr;
		struct dirent *ent;
		std::string fpath{anim_base_path};

		fprintf(gen_header_file, "enum Anim_ID : Asset_ID {\n");
		while ((ent = each_file(&root_dr, fpath))) {
			if (ent->d_type != DT_DIR)
				continue;
			std::string anim_fpath = fpath + std::string("/") + std::string(ent->d_name) + std::string(".anim");
			FILE *anim_file = fopen(anim_fpath.c_str(), "r");
			if (!anim_file) {
				printf("Failed to open animaton file %s - %s\n", anim_fpath.c_str(), strerror(errno));
				continue;
			}
			char anim_root[256];
			char anim_child[256];
			char c_sheet_path[256];
			fscanf(anim_file, "[%[^\]]]\n", anim_root);
			fscanf(anim_file, "%s\n", c_sheet_path);

			std::string sheet_path = std::string(c_sheet_path);
			Asset_ID sheet_id;
			auto itr = image_map.find(sheet_path);
			if (itr == image_map.end()) {
				sheet_id = (Asset_ID)image_map.size();
				image_map[sheet_path] = sheet_id;
			} else
				sheet_id = itr->second;

			while (fscanf(anim_file, "[%[^\]]]\n", anim_child) != EOF) {
				anim_offsets.push_back(ftell(asset_file));
				Anim_Header ah;
				int num_frames;
				fscanf(anim_file, "%d", &num_frames);
				ah.num_frames = num_frames;
				ah.spritesheet_id = sheet_id;
				fwrite(&ah, sizeof(ah), 1, asset_file);

				Frame_Info fi;
				for (int i = 0; i < num_frames; ++i) {
					fscanf(anim_file, "%d,%d,%d,%d,%d\n", &fi.delay, &fi.x, &fi.y, &fi.w, &fi.h);
					fwrite(&fi, sizeof(fi), 1, asset_file);
				}

				std::string anim_name = std::string(anim_root) + "_" + std::string(anim_child) + "_anim";
				fprintf(gen_header_file, "\t%s,\n", anim_name.c_str());
			}
		}
		fprintf(gen_header_file, "\tNUM_ANIMATIONS\n");
		fprintf(gen_header_file, "};\n\n");
	}

	// Map.
	{
		Map_Header mh;

		XMLDocument doc;
		doc.LoadFile("../../assets/maps/untitled.tmx");

		XMLElement *tileset_xml = doc.FirstChildElement("map")->FirstChildElement("tileset");
		tileset_xml->QueryIntAttribute("tilewidth", &mh.tile_width);
		tileset_xml->QueryIntAttribute("tileheight", &mh.tile_height);
		int num_tilesheet_cols;
		tileset_xml->QueryIntAttribute("columns", &num_tilesheet_cols);
		std::string tex_path = std::string("../../assets/maps/") + std::string(tileset_xml->FirstChildElement("image")->Attribute("source"));
		image_map[tex_path] = mh.tilesheet_id;
		mh.tilesheet_id = (Asset_ID)image_map.size();

		XMLElement *layer_xml = doc.FirstChildElement("map")->FirstChildElement("layer");
		layer_xml->QueryIntAttribute("width", &mh.num_cols);
		layer_xml->QueryIntAttribute("height", &mh.num_rows);

		fwrite(&mh, sizeof(mh), 1, asset_file);

		//l.tiles = (SDL_Rect **)malloc(sizeof(SDL_Rect *) * l.cols);
		//for (int i = 0; i < l.cols; ++i)
		//	l.tiles[i] = (SDL_Rect *)malloc(sizeof(SDL_Rect) * l.rows);
		const char *data = layer_xml->FirstChildElement("data")->GetText();
		for (int i = 0; i < mh.num_rows; ++i) {
			for (int j = 0; j < mh.num_cols; ++j) {
				while (!isdigit(data[0])) // Skip newlines, commas, etc.
					data += 1;
				char ind_buf[16];
				sscanf(data, "%[0-9]", ind_buf);
				data += strlen(ind_buf);
				int tile_ind = atoi(ind_buf) - 1; // Make it 0 indexed;
				SDL_Rect tex_src;
				tex_src.x = (tile_ind % num_tilesheet_cols) * mh.tile_width;
				tex_src.y = (tile_ind / num_tilesheet_cols) * mh.tile_height;
				tex_src.w = mh.tile_width;
				tex_src.h = mh.tile_height;
				fwrite(&tex_src, sizeof(tex_src), 1, asset_file);
				//l.tiles[i][j] = tex_src;
			}
		}
	}

	// Images.
	{
		fprintf(gen_header_file, "enum Image_ID : Asset_ID {\n");
		image_offsets.resize(image_map.size());
		for (auto itr : image_map) {
			SDL_Surface *img;
			if (!(img = IMG_Load(itr.first.c_str()))) {
				printf("IMG_Load failed! IMG_GetError: %s\n", IMG_GetError());
				image_offsets[(int)itr.second] = -1;
				continue;
			}
			image_offsets[(int)itr.second] = ftell(asset_file);

			Image_Header ih;
			ih.w = img->w;
			ih.h = img->h;
			ih.bytes_per_row = img->pitch;
			ih.pixel_format = img->format->format;
			fwrite(&ih, sizeof(ih), 1, asset_file);
			fwrite(img->pixels, img->pitch * img->h, 1, asset_file);

			std::string img_name = get_base_name(itr.first) + "_img";
			fprintf(gen_header_file, "\t%s,\n", img_name.c_str());
		}
		fprintf(gen_header_file, "\tNUM_IMAGES\n");
		fprintf(gen_header_file, "};\n\n");
	}

	// Asset File Header.
	{
		fwrite(anim_offsets.data(), sizeof(anim_offsets[0]), anim_offsets.size(), asset_file);
		fwrite(image_offsets.data(), sizeof(image_offsets[0]), image_offsets.size(), asset_file);

		fprintf(gen_header_file, "struct Asset_Header {\n");;
		fprintf(gen_header_file, "\tAsset_Offset animation_table[NUM_ANIMATIONS];\n");;
		fprintf(gen_header_file, "\tAsset_Offset image_table[NUM_IMAGES];\n");;
		fprintf(gen_header_file, "};\n\n");
	}

	fprintf(gen_header_file, "#endif\n");
}

