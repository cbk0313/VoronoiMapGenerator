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
	//std::cout << alpha + beta + gamma << "\n";
	const Color& c0 = colors[0], &c1 = colors[1], &c2 = colors[2];
	//return Color::lerp(c0, c1, alpha) + Color::lerp(c1, c2, beta) + Color::lerp(c2, c0, gamma);
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
	/*if (p1.y == p2.y) {
		return std::min(p1.x, p2.x);
	}*/
	if (p1.x == p2.x) {
		return p1.x;
	}

	double m = (p2.y - p1.y) / (p2.x - p1.x);
	double b = p1.y - m * p1.x;
	//std::cout << ((double)y - b) / m << "\n";
	return ((double)y - b) / m;
}

bool Triangle::IsYBetweenPoints(const Point2& p1, const Point2& p2, double y) {
	return y >= std::min(p1.y, p2.y) && y <= std::max(p1.y, p2.y);
}
unsigned int Triangle::FindLeftmostXGivenY(unsigned int y, unsigned int min_x) {
	// 주어진 y 값이 삼각형의 범위 내에 있는지 확인
	const Point2& p1 = points[0], &p2 = points[1], &p3 = points[2];

	/*double MinY = std::min({ p1.y, p2.y, p3.y });
	double MaxY = std::max({ p1.y, p2.y, p3.y });
	if (y < MinY || y > MaxY) {
		throw std::out_of_range("주어진 y 값이 삼각형의 범위를 벗어납니다.");
	}*/

	// 가장 좌측에 있는 x 값을 초기화
	double mostX = std::numeric_limits<double>::max();
	bool check = false;

	if (IsYBetweenPoints(p1, p2, y)) {
		mostX = std::min(mostX, FindXGivenY(p1, p2, y));
		check = true;
		//std::cout << (unsigned int)mostX << " A\n";
	}
	if (IsYBetweenPoints(p2, p3, y)) {
		mostX = std::min(mostX, FindXGivenY(p2, p3, y));
		check = true;
		//std::cout << (unsigned int)mostX << " B\n";
	}
	if (IsYBetweenPoints(p1, p3, y)) {
		mostX = std::min(mostX, FindXGivenY(p1, p3, y));
		check = true;
		//std::cout << (unsigned int)mostX << " C\n";
	}

	if (!check) return min_x;
	else {
		return (unsigned int)mostX + 1;
	}


	
}


unsigned int Triangle::FindRightmostXGivenY(unsigned int y, unsigned int max_x) {
	// 주어진 y 값이 삼각형의 범위 내에 있는지 확인
	const Point2& p1 = points[0], & p2 = points[1], & p3 = points[2];

	/*double MinY = std::min({ p1.y, p2.y, p3.y });
	double MaxY = std::max({ p1.y, p2.y, p3.y });
	if (y < MinY || y > MaxY) {
		throw std::out_of_range("주어진 y 값이 삼각형의 범위를 벗어납니다.");
	}*/

	// 가장 좌측에 있는 x 값을 초기화
	double mostX = -1;
	bool check = false;

	if (IsYBetweenPoints(p1, p2, y)) {
		mostX = std::max(mostX, FindXGivenY(p1, p2, y));
		check = true;
		//std::cout << (unsigned int)mostX << " A\n";
	}
	if (IsYBetweenPoints(p2, p3, y)) {
		mostX = std::max(mostX, FindXGivenY(p2, p3, y));
		check = true;
		//std::cout << (unsigned int)mostX << " B\n";
	}
	if (IsYBetweenPoints(p1, p3, y)) {
		mostX = std::max(mostX, FindXGivenY(p1, p3, y));
		check = true;
		//std::cout << (unsigned int)mostX << " C\n";
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
			//std::cout << pixel_alpha << "\n";
			/*std::cout << pixel_c.r << "\n";
			std::cout << pixel_c.g << "\n";
			std::cout << pixel_c.b << "\n";*/
			
			
			Color mix_c = Color(
				(pixel_c.r * pixel_alpha + c.r * alpha),
				(pixel_c.g * pixel_alpha + c.g * alpha),
				(pixel_c.b * pixel_alpha + c.b * alpha),
				1);
			
			//mix_c.a = std::clamp<double>(mix_c.a, 0, 1.);
			//mix_c.g = std::clamp<double>(mix_c.g, 0, 1.);
			//mix_c.b = std::clamp<double>(mix_c.b, 0, 1.);
			
		/*	std::cout << (unsigned int)test.r << "\n";
			std::cout << (unsigned int)test.g << "\n";
			std::cout << (unsigned int)test.b << "\n";
			std::cout << mix_c.a << "!\n";
			*/
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