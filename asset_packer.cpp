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

#include "asset_packer.h"

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

std::map<std::string, int> image_map;
std::vector<Asset_Offset> anim_offsets;
std::vector<Asset_Offset> image_offsets;

int
main(int, char **)
{
	FILE *anim_h_file, *asset_file;
	anim_h_file = fopen("asset.h", "w");

	fprintf(anim_h_file, "#ifndef __ASSET_H__\n");
	fprintf(anim_h_file, "#define __ASSET_H__\n\n");
	fprintf(anim_h_file, "#include \"asset_packer.h\"\n\n");

	fprintf(anim_h_file, "enum Anim_ID {\n");

	asset_file = fopen("assets.ahh", "wb");
	if (!asset_file) {
		printf("Failed to open asset file - %s\n", strerror(errno));
		return 1;
	}

	const char *anim_base_path = "assets/animations";
	Dir_Read root_dr;
	struct dirent *ent;
	std::string fpath{anim_base_path};

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
		char sheet_path[256];
		fscanf(anim_file, "[%[^\]]]\n", anim_root);
		fscanf(anim_file, "%s\n", sheet_path);

		std::string img_path = std::string(sheet_path);
		int img_ind = 0;
		auto itr = image_map.find(img_path);
		if (itr == image_map.end()) {
			img_ind = image_map.size();
			image_map[img_path] = img_ind;
		} else
			img_ind = itr->second;
		//fwrite(&img_ind, sizeof(img_ind), 1, asset_file);

		while (fscanf(anim_file, "[%[^\]]]\n", anim_child) != EOF) {
			anim_offsets.push_back(ftell(asset_file));
			Anim_Header ah;
			int num_frames;
			fscanf(anim_file, "%d", &num_frames);
			ah.num_frames = num_frames;
			ah.image_id = img_ind;
			fwrite(&ah, sizeof(ah), 1, asset_file);

			Frame_Info fi;
			for (int i = 0; i < num_frames; ++i) {
				fscanf(anim_file, "%d,%d,%d,%d,%d\n", &fi.delay, &fi.x, &fi.y, &fi.w, &fi.h);
				fwrite(&fi, sizeof(fi), 1, asset_file);
			}

			std::string anim_name = std::string(anim_root) + "_" + std::string(anim_child) + "_anim";
			fprintf(anim_h_file, "\t%s,\n", anim_name.c_str());
		}
	}
	fprintf(anim_h_file, "\tNUM_ANIMATIONS\n");
	fprintf(anim_h_file, "};\n\n");

	fprintf(anim_h_file, "enum Image_ID {\n");
	image_offsets.resize(image_map.size());
	for (auto itr : image_map) {
		SDL_Surface *img;
		if (!(img = IMG_Load(itr.first.c_str()))) {
			printf("IMG_Load failed! IMG_GetError: %s\n", IMG_GetError());
			image_offsets[itr.second] = 0;
			continue;
		}
		image_offsets[itr.second] = ftell(asset_file);

		Image_Header ih;
		ih.w = img->w;
		ih.h = img->h;
		ih.bytes_per_row = img->pitch;
		fwrite(&ih, sizeof(ih), 1, asset_file);
		fwrite(img->pixels, img->pitch * img->h, 1, asset_file);

		std::string img_name = get_base_name(itr.first) + "_img";
		fprintf(anim_h_file, "\t%s,\n", img_name.c_str());
	}
	fprintf(anim_h_file, "\tNUM_IMAGES\n");
	fprintf(anim_h_file, "};\n\n");

	fwrite(anim_offsets.data(), sizeof(anim_offsets[0]), anim_offsets.size(), asset_file);
	fwrite(image_offsets.data(), sizeof(image_offsets[0]), image_offsets.size(), asset_file);

	fprintf(anim_h_file, "struct Asset_Header {\n");;
	fprintf(anim_h_file, "\tAsset_Offset animation_table[NUM_ANIMATIONS];\n");;
	fprintf(anim_h_file, "\tAsset_Offset image_table[NUM_IMAGES];\n");;
	fprintf(anim_h_file, "};\n\n");
	fprintf(anim_h_file, "#endif\n");
}

