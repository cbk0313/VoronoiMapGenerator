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

Color const Color::black = Color(0, 0, 0);
Color const Color::lake = Color(0.2, 0.4, 0.6);
Color const Color::white = Color(1, 1, 1);
Color const Color::edgeOcean = Color(0.1, 0, 0.3, 1);
Color const Color::ocean = Color(0.2, 0, 0.6, 1);
Color const Color::coast = Color(0.4, 0, 1, 1);

#include <iostream>
Color ::operator CharColor() const {
	
	//auto t = CharColor(
	//	(unsigned char)(std::clamp(r, 0.0, 1.0) * 255.0),
	//	(unsigned char)(std::clamp(g, 0.0, 1.0) * 255.0),
	//	(unsigned char)(std::clamp(b, 0.0, 1.0) * 255.0));

	//if ((unsigned int)t.b > 255) {
	//	std::cout << r << ", " << g << ", " << b << "\n";
	//	std::cout << (unsigned int)t.r << ", " << (unsigned int)t.g << ", " << (unsigned int)t.b << "\n";

	//	std::cout << std::clamp(b, 0.0, 1.0) << "\n";
	//	std::cout << (std::clamp(b, 0.0, 1.0) * 255.0) << "\n";
	//	std::cout << (unsigned int)(std::clamp(b, 0.0, 1.0) * 255.0) << "\n";
	//	std::cout << (unsigned int)((unsigned char)((unsigned int)(std::clamp(b, 0.0, 1.0) * 255.0))) << "\n";
	//	std::cout << (unsigned int)((unsigned char)((unsigned int)(std::clamp(b, 0.0, 1.0) * 255.0))) << "\n";

	//	unsigned int test = static_cast<unsigned int>(std::clamp(b, 0.0, 1.0) * 255.0);
	//	char test2 = static_cast<unsigned char>(test);
	//	unsigned int test3 = test2;
	//	std::cout << test3 << "\n";
	//}
	//
	return CharColor(
		(unsigned char)(std::clamp(r * 255.0, 0.0, 255.0)),
		(unsigned char)(std::clamp(g * 255.0, 0.0, 255.0)),
		(unsigned char)(std::clamp(b * 255.0, 0.0, 255.0)));
}

uint16_t Color::Graysacle() const {
	double c = ((r + g + b) / 3) * a;
	unsigned int g = (unsigned int)(c * MAX_GRAY);
	return static_cast<uint16_t>(std::clamp<unsigned int>(g, 0, MAX_GRAY));
}

CharColor& CharColor::operator =(const Color& a) {
	r = (unsigned char)(a.r >= 1.0 ? 255.0 : a.r * 255.0);
	g = (unsigned char)(a.g >= 1.0 ? 255.0 : a.g * 255.0);
	b = (unsigned char)(a.b >= 1.0 ? 255.0 : a.b * 255.0);
	return *this;
}
CharColor& CharColor::operator =(const Color* a) {
	r = (unsigned char)(a->r >= 1.0 ? 255.0 : a->r * 255.0);
	g = (unsigned char)(a->g >= 1.0 ? 255.0 : a->g * 255.0);
	b = (unsigned char)(a->b >= 1.0 ? 255.0 : a->b * 255.0);
	return *this;
}


VertexColor VertexColor::operator -(const VertexColor& c) {

	return VertexColor(rgb - c.rgb, gray < c.gray ? 0 : gray - c.gray);
}


VertexColor VertexColor::MixColor(VertexColor& c1, VertexColor& c2) {
	unsigned int g = (unsigned int)((c1.gray + c2.gray) / 2);
	g = std::clamp<unsigned int>(g, 0, MAX_GRAY);
	return VertexColor(Color::MixColor(c1.rgb, c2.rgb), static_cast<uint16_t>(g));
}