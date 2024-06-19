#include "Triangle.h"

#include "../Point2.h"
#include "Color.h"



double Triangle::minX() const {
	return std::min({ points[0].x, points[1].x, points[2].x });
}

double Triangle::maxX() const {
	return std::max({ points[0].x, points[1].x, points[2].x });
}

double Triangle::minY() const {
	return std::min({ points[0].y, points[1].y, points[2].y });
}

double Triangle::maxY() const {
	return std::max({ points[0].y, points[1].y, points[2].y });
}


bool Triangle::isInside(const Point2& p) const {
	double x0 = points[0].x, y0 = points[0].y;
	double x1 = points[1].x, y1 = points[1].y;
	double x2 = points[2].x, y2 = points[2].y;

	double alpha = ((y1 - y2) * (p.x - x2) + (x2 - x1) * (p.y - y2)) / ((y1 - y2) * (x0 - x2) + (x2 - x1) * (y0 - y2));
	double beta = ((y2 - y0) * (p.x - x2) + (x0 - x2) * (p.y - y2)) / ((y1 - y2) * (x0 - x2) + (x2 - x1) * (y0 - y2));
	double gamma = 1.0f - alpha - beta;

	return alpha >= 0 && beta >= 0 && gamma >= 0;
}

Color Triangle::interpolateColor(const Point2& p) const {
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
		c0.b * alpha + c1.b * beta + c2.b * gamma
	);
}

void Triangle::drawPixel(char* pixel_data, unsigned int w, unsigned int h, unsigned int x, unsigned int y, const CharColor& c) const {
	int pos = x * 3 + w * 3 * (h - y - 1);
	//std::cout << pos << "\n";
	pixel_data[pos] = c.b;
	pixel_data[pos + 1] = c.g;
	pixel_data[pos + 2] = c.r;
	//std::cout << (int)pixel_data[pos] << ", " << (int)pixel_data[pos + 1] << ", " << (int)pixel_data[pos + 2] << "\n";
}

int Triangle::FindStartX(int lowX, int highX, int y) {
	while (lowX <= highX) {
		int m = (lowX + highX) / 2;

		if (isInside(Point2(m, y))) {
			lowX = m + 1;
		}
		else {
			highX = m - 1;
		}
	}

	return lowX;
}

void Triangle::draw(char* pixel_data, unsigned int w, unsigned int h) {
	// 선형 보간을 사용하여 삼각형 내부를 채우기
	unsigned int m_X = (unsigned int)minX(), M_X = (unsigned int)maxX(), m_Y = (unsigned int)minY(), M_Y = (unsigned int)maxY();

	if (m_X < 0) m_X = 0;
	if (M_X >= w) M_X = w - 1;
	if (m_Y < 0) m_Y = 0;
	if (M_Y >= h) M_Y = h - 1;

	for (unsigned int y = m_Y; y <= M_Y; y += 1) {
		bool drew = false;
		//int find_x = FindStartX(m_X, M_X, y);
		//bool state = !isInside(Point2(find_x - 1, y));
		//if (state) {
		//	for (int x = find_x - 1; x <= M_X; x += 1) {
		//		//std::cout << FindStartX(m_X, M_X, y) << "\n";
		//		if (x < 0 || y < 0 || x >= w || y >= h) break;
		//		if (isInside(Point2(x, y))) {
		//			drew = true;
		//			CharColor c = interpolateColor(Point2(x, y));
		//			drawPixel(pixel_data, w, h, x, y, c);
		//		}
		//		else if (drew) break;
		//	}
		//}
		//else {
		//	for (int x = find_x; x >= m_X; x -= 1) {
		//		//std::cout << FindStartX(m_X, M_X, y) << "\n";
		//		if (x < 0 || y < 0 || x >= w || y >= h) break;
		//		if (isInside(Point2(x, y))) {
		//			drew = true;
		//			CharColor c = interpolateColor(Point2(x, y));
		//			drawPixel(pixel_data, w, h, x, y, c);
		//		}
		//		else if (drew) break;
		//	}
		//}
		

	
		for (unsigned int x = m_X; x <= M_X; x += 1) {
			if (isInside(Point2(x, y))) {
				drew = true;
				CharColor c = interpolateColor(Point2(x, y));
				drawPixel(pixel_data, w, h, x, y, c);
			}
			else if (drew) break;
		}
		
	}
}

