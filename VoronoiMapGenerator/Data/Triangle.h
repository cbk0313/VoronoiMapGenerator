#pragma once

#include "../Point2.h"
#include "Color.h"

//class Point2;
//class Color;
//class CharColor;

class Triangle;
using Triangles = std::vector<Triangle>;
class Heightmap;

class Triangle {
public:
	Point2 points[3];
	VertexColor colors[3];

	//Triangle(Point2* p1, Point2* p2, Point2* p3, Color* c1, Color* c2, Color* c3);

	void Draw(unsigned char* pixel_data, unsigned int w, unsigned int h);
	void DrawGrayscale(Heightmap& pixel_data, unsigned int w, unsigned int h);
	void DrawTransparent(unsigned char* pixel_data, unsigned int w, unsigned int h);
	void DrawTransparentGrayscale(Heightmap& pixel_data, unsigned int w, unsigned int h);

	void AdjustSize(unsigned int w, unsigned int h, double dimension);

private:
	double MinX() const;
	double MaxX() const;
	double MinY() const;
	double MaxY() const;

	double FindXGivenY(const Point2& p1, const Point2& p2, unsigned int y);
	bool IsYBetweenPoints(const Point2& p1, const Point2& p2, double y);
	unsigned int FindLeftmostXGivenY(unsigned int y, unsigned int max_x);
	unsigned int FindRightmostXGivenY(unsigned int y, unsigned int max_x);

	bool IsInside(const Point2& p) const;
	Color InterpolateColor(const Point2& p) const;
	uint16_t InterpolateGray(const Point2& p) const;
	void DrawPixel(unsigned char* pixel_data, unsigned int w, unsigned int h, unsigned int x, unsigned int y, const CharColor& c) const;
	void DrawGrayscalePixel(Heightmap& pixel_data, unsigned int w, unsigned int h, unsigned int x, unsigned int y, const uint16_t c) const;
	CharColor GetPixelColor(unsigned char* pixel_data, unsigned int w, unsigned int h, unsigned int x, unsigned int y) const;
	uint16_t GetGrayscalePixelColor(Heightmap& pixel_data, unsigned int w, unsigned int h, unsigned int x, unsigned int y) const;
};