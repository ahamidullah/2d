all:
	cd src; ./make_sheets.sh
	make -C ./src/asset_packer/ all
	make -C ./src/2d/ all

.PHONY:
	all
