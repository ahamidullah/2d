all:
	make -C ./src/2d all

assets:
	cd src; ./make_sheets.sh
	make -C ./src/asset_packer all

run:
	cd ./src/2d; ../../build/2d

.PHONY:
	all
