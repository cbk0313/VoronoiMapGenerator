#pragma once

#include "Color.h"
#include <algorithm>


Color Color::operator *(const int num) {
	double cast_num = (double)num;
	return Color(r * cast_num, g * cast_num, b * cast_num, a);
}
Color Color::operator *(const double num) {
	return Color(r * num, g * num, b * num, a);
}
Color Color::operator *=(int num) {
	*this = *this * num;
	return *this;
}

Color Color::operator *=(double num) {
	*this = *this * num;
	return *this;
}

Color Color::operator /(const int num) {
	double cast_num = (double)num;
	return Color(r / cast_num, g / cast_num, b / cast_num, a);
}
Color Color::operator /(const double num) {
	return Color(r / num, g / num, b / num, a);
}
Color Color::operator /=(int num) {
	*this = *this / num;
	return *this;
}

Color Color::operator /=(double num) {
	*this = *this / num;
	return *this;
}

Color Color::operator +(const Color& c) {
	return Color(r + c.r, g + c.g, b + c.b, a);
}

Color Color::operator +=(Color& c) {
	*this = *this + c;
	return *this;
}

Color Color::operator -(const Color& c) {
	return Color(r - c.r, g - c.g, b - c.b, a);
}

Color Color::operator -=(Color& c) {
	*this = *this - c;
	return *this;
}

bool Color::operator ==(Color& c) {
	if (this->r == c.r && this->g == c.g && this->b == c.b && this->a == c.a) {
		return true;
	}
	else {
		return false;
	}
}

Color Color::MixColor(Color& c1, Color& c2) {
	return Color((c1.r + c2.r) / 2, (c1.g + c2.g) / 2, (c1.b + c2.b) / 2, (c1.a + c2.a) / 2);
}

Color Color::lerp(const Color& a, const Color& b, double t) {
	return Color(
		(1 - t) * a.r + t * b.r,
		(1 - t) * a.g + t * b.g,
		(1 - t) * a.b + t * b.b
	);
}
#include <iostream>
Color ::operator CharColor() const {
	return CharColor(
		(char)(std::clamp(r, 0.0, 1.0) * 255),
		(char)(std::clamp(g, 0.0, 1.0) * 255),
		(char)(std::clamp(b, 0.0, 1.0) * 255));
}



CharColor& CharColor::operator =(const Color& a) {
	r = (char)(a.r > 1.0 ? 255.0 : a.r * 255.0);
	g = (char)(a.g > 1.0 ? 255.0 : a.g * 255.0);
	b = (char)(a.g > 1.0 ? 255.0 : a.b * 255.0);
	return *this;
}
CharColor& CharColor::operator =(const Color* a) {
	r = (char)(a->r > 1.0 ? 255.0 : a->r * 255.0);
	g = (char)(a->g > 1.0 ? 255.0 : a->g * 255.0);
	b = (char)(a->g > 1.0 ? 255.0 : a->b * 255.0);
	return *this;
}
