all:
	g++ -g asset_packer.cpp -lSDL2 -lSDL2_image -o ap
	./ap
	g++ -g -fno-exceptions xml.cpp 2d.cpp -lSDL2 -lSDL2_image -o out
	./out

optimized:
	gcc -O3 -fno-exceptions 2d.cpp -lSDL2 -lSDL2_image -o out
.PHONY:
	all
