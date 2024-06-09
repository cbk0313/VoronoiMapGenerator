#pragma once

class Point2;
class Color;
class CharColor;


class Triangle {
public:
	Point2* points[3];
	Color* colors[3];

	//Triangle(Point2* p1, Point2* p2, Point2* p3, Color* c1, Color* c2, Color* c3);

	void draw(char* pixel_data, unsigned int w, unsigned int h);

private:
	double minX() const;
	double maxX() const;
	double minY() const;
	double maxY() const;

	bool isInside(const Point2& p) const;
	Color interpolateColor(const Point2& p) const;
	void drawPixel(char* pixel_data, unsigned int w, unsigned int h, unsigned int x, unsigned int y, const CharColor& c) const;

	int FindStartX(int lowX, int highX, int y);
};