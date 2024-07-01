#include "Triangle.h"

#include "../Point2.h"
#include "Color.h"
#include <algorithm>


double Triangle::MinX() const {
	return std::min({ points[0].x, points[1].x, points[2].x });
}

double Triangle::MaxX() const {
	return std::max({ points[0].x, points[1].x, points[2].x });
}

double Triangle::MinY() const {
	return std::min({ points[0].y, points[1].y, points[2].y });
}

double Triangle::MaxY() const {
	return std::max({ points[0].y, points[1].y, points[2].y });
}


bool Triangle::IsInside(const Point2& p) const {
	double x0 = points[0].x, y0 = points[0].y;
	double x1 = points[1].x, y1 = points[1].y;
	double x2 = points[2].x, y2 = points[2].y;

	double alpha = ((y1 - y2) * (p.x - x2) + (x2 - x1) * (p.y - y2)) / ((y1 - y2) * (x0 - x2) + (x2 - x1) * (y0 - y2));
	double beta = ((y2 - y0) * (p.x - x2) + (x0 - x2) * (p.y - y2)) / ((y1 - y2) * (x0 - x2) + (x2 - x1) * (y0 - y2));
	double gamma = 1.0f - alpha - beta;

	return alpha >= 0 && beta >= 0 && gamma >= 0;
}

Color Triangle::InterpolateColor(const Point2& p) const {
	double x0 = points[0].x, y0 = points[0].y;
	double x1 = points[1].x, y1 = points[1].y;
	double x2 = points[2].x, y2 = points[2].y;

	double alpha = ((y1 - y2) * (p.x - x2) + (x2 - x1) * (p.y - y2)) / ((y1 - y2) * (x0 - x2) + (x2 - x1) * (y0 - y2));
	double beta = ((y2 - y0) * (p.x - x2) + (x0 - x2) * (p.y - y2)) / ((y1 - y2) * (x0 - x2) + (x2 - x1) * (y0 - y2));
	double gamma = 1.0f - alpha - beta;
	const Color& c0 = colors[0], &c1 = colors[1], &c2 = colors[2];
	return Color(
		c0.r * alpha + c1.r * beta + c2.r * gamma,
		c0.g * alpha + c1.g * beta + c2.g * gamma,
		c0.b * alpha + c1.b * beta + c2.b * gamma,
		c0.a * alpha + c1.a * beta + c2.a * gamma
	);
}

void Triangle::DrawPixel(unsigned char* pixel_data, unsigned int w, unsigned int h, unsigned int x, unsigned int y, const CharColor& c) const {
	int pos = x * 3 + w * 3 * (h - y - 1);
	//std::cout << pos << "\n";
	pixel_data[pos] = c.b;
	pixel_data[pos + 1] = c.g;
	pixel_data[pos + 2] = c.r;
	//std::cout << (int)pixel_data[pos] << ", " << (int)pixel_data[pos + 1] << ", " << (int)pixel_data[pos + 2] << "\n";
}

CharColor Triangle::GetPixelColor(unsigned char* pixel_data, unsigned int w, unsigned int h, unsigned int x, unsigned int y) const {
	int pos = x * 3 + w * 3 * (h - y - 1);
	return CharColor(pixel_data[pos + 2], pixel_data[pos + 1], pixel_data[pos]);
}


double Triangle::FindXGivenY(const Point2& p1, const Point2& p2, unsigned int y) {

	if (p1.x == p2.x) {
		return p1.x;
	}

	double m = (p2.y - p1.y) / (p2.x - p1.x);
	double b = p1.y - m * p1.x;
	return ((double)y - b) / m;
}

bool Triangle::IsYBetweenPoints(const Point2& p1, const Point2& p2, double y) {
	return y >= std::min(p1.y, p2.y) && y <= std::max(p1.y, p2.y);
}
unsigned int Triangle::FindLeftmostXGivenY(unsigned int y, unsigned int min_x) {
	const Point2& p1 = points[0], &p2 = points[1], &p3 = points[2];

	double mostX = std::numeric_limits<double>::max();
	bool check = false;

	if (IsYBetweenPoints(p1, p2, y)) {
		mostX = std::min(mostX, FindXGivenY(p1, p2, y));
		check = true;
	}
	if (IsYBetweenPoints(p2, p3, y)) {
		mostX = std::min(mostX, FindXGivenY(p2, p3, y));
		check = true;
	}
	if (IsYBetweenPoints(p1, p3, y)) {
		mostX = std::min(mostX, FindXGivenY(p1, p3, y));
		check = true;
	}

	if (!check) return min_x;
	else {
		return (unsigned int)mostX + 1;
	}

	
}


unsigned int Triangle::FindRightmostXGivenY(unsigned int y, unsigned int max_x) {

	const Point2& p1 = points[0], & p2 = points[1], & p3 = points[2];

	double mostX = -1;
	bool check = false;

	if (IsYBetweenPoints(p1, p2, y)) {
		mostX = std::max(mostX, FindXGivenY(p1, p2, y));
		check = true;
	}
	if (IsYBetweenPoints(p2, p3, y)) {
		mostX = std::max(mostX, FindXGivenY(p2, p3, y));
		check = true;
	}
	if (IsYBetweenPoints(p1, p3, y)) {
		mostX = std::max(mostX, FindXGivenY(p1, p3, y));
		check = true;
	}

	if (!check) return max_x;
	else {
		return (unsigned int)mostX;
	}



}


void Triangle::Draw(unsigned char* pixel_data, unsigned int w, unsigned int h) {

	unsigned int m_X = (unsigned int)MinX(), M_X = (unsigned int)MaxX(), m_Y = (unsigned int)MinY(), M_Y = (unsigned int)MaxY();

	if (m_X < 0) m_X = 0;
	if (M_X >= w) M_X = w - 1;
	if (m_Y < 0) m_Y = 0;
	if (M_Y >= h) M_Y = h - 1;

	for (unsigned int y = m_Y; y <= M_Y; y += 1) {
		for (unsigned int x = FindLeftmostXGivenY(y, m_X); x <= M_X; x += 1) {
			if (!IsInside(Point2(x, y))) break;
			CharColor c = InterpolateColor(Point2(x, y));
			DrawPixel(pixel_data, w, h, x, y, c);
			
		}
	}
}


void Triangle::DrawTransparent(unsigned char* pixel_data, unsigned int w, unsigned int h) {

	unsigned int m_X = (unsigned int)MinX(), M_X = (unsigned int)MaxX(), m_Y = (unsigned int)MinY(), M_Y = (unsigned int)MaxY();

	if (m_X < 0) m_X = 0;
	if (M_X >= w) M_X = w - 1;
	if (m_Y < 0) m_Y = 0;
	if (M_Y >= h) M_Y = h - 1;

	for (unsigned int y = m_Y; y <= M_Y; y += 1) {
		for (unsigned int x = FindLeftmostXGivenY(y, m_X); x <= M_X; x += 1) {
			if (!IsInside(Point2(x, y))) break;
			Color c = InterpolateColor(Point2(x, y));
			CharColor pixel_char_c = GetPixelColor(pixel_data, w, h, x, y);
			Color pixel_c = Color((double)pixel_char_c.r / 255, (double)pixel_char_c.g / 255, (double)pixel_char_c.b / 255, 1);
			
			double alpha = c.a;
			if (alpha > 1) alpha = 1;
			else if (alpha < 0) alpha = 0;
			double pixel_alpha = 1. - alpha;
			
			Color mix_c = Color(
				(pixel_c.r * pixel_alpha + c.r * alpha),
				(pixel_c.g * pixel_alpha + c.g * alpha),
				(pixel_c.b * pixel_alpha + c.b * alpha),
				1);

			DrawPixel(pixel_data, w, h, x, y, mix_c);
		}
	}
}



void Triangle::AdjustSize(unsigned int w, unsigned int h, double dimension) {
	for (int i = 0; i < 3; i++) {
		points[i] /= dimension;
		points[i].x *= w;
		points[i].y *= h;
	}
}