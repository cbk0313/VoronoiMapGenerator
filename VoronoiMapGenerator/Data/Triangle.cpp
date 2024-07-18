#include "Triangle.h"

#include <algorithm>
#include "Heightmap.h"


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
	const Color& c0 = colors[0].rgb, &c1 = colors[1].rgb, &c2 = colors[2].rgb;
	return Color(
		c0.r * alpha + c1.r * beta + c2.r * gamma,
		c0.g * alpha + c1.g * beta + c2.g * gamma,
		c0.b * alpha + c1.b * beta + c2.b * gamma,
		c0.a * alpha + c1.a * beta + c2.a * gamma
	);
}

uint16_t Triangle::InterpolateGray(const Point2& p) const {
	double x0 = points[0].x, y0 = points[0].y;
	double x1 = points[1].x, y1 = points[1].y;
	double x2 = points[2].x, y2 = points[2].y;

	double alpha = ((y1 - y2) * (p.x - x2) + (x2 - x1) * (p.y - y2)) / ((y1 - y2) * (x0 - x2) + (x2 - x1) * (y0 - y2));
	double beta = ((y2 - y0) * (p.x - x2) + (x0 - x2) * (p.y - y2)) / ((y1 - y2) * (x0 - x2) + (x2 - x1) * (y0 - y2));
	double gamma = 1.0f - alpha - beta;
	const uint16_t & c0 = colors[0].gray, & c1 = colors[1].gray, & c2 = colors[2].gray;
	unsigned int c = static_cast<unsigned int>((double)c0 * alpha + (double)c1 * beta + (double)c2 * gamma);

	
	return static_cast<uint16_t>(c);
}

void Triangle::DrawPixel(unsigned char* pixel_data, unsigned int w, unsigned int h, unsigned int x, unsigned int y, const CharColor& c) const {
	int pos = x * 3 + w * 3 * (h - y - 1);
	pixel_data[pos] = c.b;
	pixel_data[pos + 1] = c.g;
	pixel_data[pos + 2] = c.r;
}
void Triangle::DrawGrayscalePixel(Heightmap& pixel_data, unsigned int w, unsigned int h, unsigned int x, unsigned int y, const uint16_t c) const {
	int pos = x + w * (h - y - 1);
	pixel_data[pos] = c;
}

CharColor Triangle::GetPixelColor(unsigned char* pixel_data, unsigned int w, unsigned int h, unsigned int x, unsigned int y) const {
	int pos = x * 3 + w * 3 * (h - y - 1);
	return CharColor(pixel_data[pos + 2], pixel_data[pos + 1], pixel_data[pos]);
}

uint16_t Triangle::GetGrayscalePixelColor(Heightmap& pixel_data, unsigned int w, unsigned int h, unsigned int x, unsigned int y) const {
	int pos = x + w * (h - y - 1);
	return pixel_data[pos];
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
unsigned int Triangle::FindLeftmostXGivenY(unsigned int y, unsigned int max_x) {
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

	if (!check) return max_x;
	else {
		return (unsigned int)mostX;
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

	if (M_X >= w) M_X = w - 1;
	if (M_Y >= h) M_Y = h - 1;

	for (unsigned int y = m_Y; y <= M_Y; y += 1) {
		bool begin_draw = false;
		for (unsigned int x = FindLeftmostXGivenY(y, M_X); x <= M_X; x += 1) {
			if (IsInside(Point2(x, y))) {
				begin_draw = true;
				CharColor c = InterpolateColor(Point2(x, y));
				DrawPixel(pixel_data, w, h, x, y, c);
			}
			else if (begin_draw) {
				break;
			}
			else if (x == 0 || y == 0) {
				CharColor pixel_char_c = GetPixelColor(pixel_data, w, h, x, y);
				if (pixel_char_c.r == 0 && pixel_char_c.g == 0 && pixel_char_c.b == 0) {
					CharColor c = InterpolateColor(Point2(x, y));
					DrawPixel(pixel_data, w, h, x, y, c);
				}
			}

		}
	}
}


void Triangle::DrawGrayscale(Heightmap& pixel_data, unsigned int w, unsigned int h) {

	unsigned int m_X = (unsigned int)MinX(), M_X = (unsigned int)MaxX(), m_Y = (unsigned int)MinY(), M_Y = (unsigned int)MaxY();

	if (M_X >= w) M_X = w - 1;
	if (M_Y >= h) M_Y = h - 1;

	for (unsigned int y = m_Y; y <= M_Y; y += 1) {
		bool begin_draw = false;
		for (unsigned int x = FindLeftmostXGivenY(y, M_X); x <= M_X; x += 1) {
			if (IsInside(Point2(x, y))) {
				begin_draw = true;
				uint16_t c = InterpolateGray(Point2(x, y));
				DrawGrayscalePixel(pixel_data, w, h, x, y, c);
			}
			else if (begin_draw) {
				break;
			}
		}
	}
}


void Triangle::DrawTransparent(unsigned char* pixel_data, unsigned int w, unsigned int h) {

	unsigned int m_X = (unsigned int)MinX(), M_X = (unsigned int)MaxX(), m_Y = (unsigned int)MinY(), M_Y = (unsigned int)MaxY();

	if (M_X >= w) M_X = w - 1;
	if (M_Y >= h) M_Y = h - 1;


	for (unsigned int y = m_Y; y <= M_Y; y += 1) {
		bool begin_draw = false;
		for (unsigned int x = FindLeftmostXGivenY(y, M_X); x <= M_X; x += 1) {
			if (IsInside(Point2(x, y))) {
				begin_draw = true;
				Color c = InterpolateColor(Point2(x, y));
				CharColor pixel_char_c = GetPixelColor(pixel_data, w, h, x, y);
				Color pixel_c = Color((double)pixel_char_c.r / 255, (double)pixel_char_c.g / 255, (double)pixel_char_c.b / 255, 1);

				double alpha = std::clamp<double>(c.a, 0.0, 1.0);
				double pixel_alpha = 1. - alpha;

				Color mix_c = Color(
					(pixel_c.r * pixel_alpha + c.r * alpha),
					(pixel_c.g * pixel_alpha + c.g * alpha),
					(pixel_c.b * pixel_alpha + c.b * alpha),
					1);

				DrawPixel(pixel_data, w, h, x, y, mix_c);
			}
			else if (begin_draw) {
				break;
			}
			else if (x == 0 || y == 0) {
				CharColor pixel_char_c = GetPixelColor(pixel_data, w, h, x, y);
				if (pixel_char_c.r == 0 && pixel_char_c.g == 0 && pixel_char_c.b == 0) {
					Color c = InterpolateColor(Point2(x, y));
					Color pixel_c = Color((double)pixel_char_c.r / 255, (double)pixel_char_c.g / 255, (double)pixel_char_c.b / 255, 1);

					double alpha = std::clamp<double>(c.a, 0.0, 1.0);
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
	}
}


void Triangle::DrawTransparentGrayscale(Heightmap& pixel_data, unsigned int w, unsigned int h) {

	unsigned int m_X = (unsigned int)MinX(), M_X = (unsigned int)MaxX(), m_Y = (unsigned int)MinY(), M_Y = (unsigned int)MaxY();

	if (M_X >= w) M_X = w - 1;
	if (M_Y >= h) M_Y = h - 1;
	
	

	for (unsigned int y = m_Y; y <= M_Y; y += 1) {
		bool begin_draw = false;
		for (unsigned int x = FindLeftmostXGivenY(y, M_X); x <= M_X; x += 1) {
			if (IsInside(Point2(x, y))) {
				begin_draw = true;
				Color c = InterpolateColor(Point2(x, y));
				uint16_t c_gray = InterpolateGray(Point2(x, y));

				uint16_t pixel_c = GetGrayscalePixelColor(pixel_data, w, h, x, y);

				double alpha = std::clamp<double>(c.a, 0.0, 1.0);
				double pixel_alpha = 1. - alpha;

				//unsigned int mix_c = (unsigned int)(pixel_c * pixel_alpha + c_gray * alpha);
				unsigned int mix_c = std::max((unsigned int)pixel_c, (unsigned int)(c_gray * alpha));
				uint16_t cast_c = (uint16_t)std::clamp<unsigned int>(mix_c, 0, MAX_GRAY);
				//std::cout << alpha << ", " << cast_c << "\n";
				DrawGrayscalePixel(pixel_data, w, h, x, y, cast_c);
			}
			else if (begin_draw) {
				break;
			}
			
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