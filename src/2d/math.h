#ifndef __MATH_H__
#define __MATH_H__

struct Vec2i {
	int x, y;
};

struct Vec2f {
	float x, y;
};

inline float
dot_product(Vec2f a, Vec2f b)
{
	return a.x*b.x + a.y*b.y;
}

#endif
