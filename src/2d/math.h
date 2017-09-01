#ifndef __MATH_H__
#define __MATH_H__

struct Vec2i {
	int x, y;
};

struct Vec2f {
	float x, y;
	inline Vec2f &operator+=(Vec2f v);
	inline Vec2f &operator+=(float s);
	inline Vec2f &operator-=(Vec2f v);
	inline Vec2f &operator-=(float s);
};

Vec2f
operator/(Vec2f v, float s)
{
	return {v.x/s, v.y/s};
}

Vec2f
operator*(Vec2f v, float s)
{
	return {v.x*s, v.y*s};
}

Vec2f
operator-(Vec2f v, float s)
{
	return {v.x-s, v.y-s};
}

Vec2f
operator*(Vec2f v1, Vec2f v2)
{
	return {v1.x*v2.x, v1.y*v2.y};
}

Vec2f
operator+(Vec2f v, float s)
{
	return {v.x+s, v.y+s};
}

Vec2f
operator+(Vec2f v1, Vec2f v2)
{
	return {v1.x+v2.x, v1.y+v2.y};
}

Vec2f
operator-(Vec2f v1, Vec2f v2)
{
	return {v1.x-v2.x, v1.y-v2.y};
}

inline Vec2f &Vec2f::
operator+=(Vec2f v)
{
	*this = *this + v;
	return *this;
}

inline Vec2f &Vec2f::
operator-=(Vec2f v)
{
	*this = *this - v;
	return *this;
}

inline Vec2f &Vec2f::
operator-=(float s)
{
	*this = *this - s;
	return *this;
}

inline Vec2f &Vec2f::
operator+=(float s)
{
	*this = *this + s;
	return *this;
}

inline float
dot_product(Vec2f a, Vec2f b)
{
	return a.x*b.x + a.y*b.y;
}

inline float
magnitude(Vec2f v)
{
	return sqrt(v.x*v.x + v.y*v.y);
}

inline Vec2f
normalize(Vec2f v, float magnitude)
{
	// @TODO: Could do a check to see if magnitude is already one (floating point error?).
	return v / magnitude;
}

// @TODO: Could maybe avoid the sqrt from magnitude??
inline Vec2f
clamp_vec(Vec2f v, float min, float max)
{
	float mag = magnitude(v);
	if (mag < min)
		return normalize(v, mag) * min;
	else if (mag > max)
		return normalize(v, mag) * max;
	return v;
}

#endif
