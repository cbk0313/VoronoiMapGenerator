#include "Triangle.h"

#include "../Point2.h"
#include "Color.h"

void Triangle::draw() {
    // ���� ������ ����Ͽ� �ﰢ�� ���θ� ä���
    for (float x = minX(); x <= maxX(); x += 1.0f) {
        for (float y = minY(); y <= maxY(); y += 1.0f) {
            if (isInside(Point2(x, y))) {
                Color c = interpolateColor(Point2(x, y));
                // ���⼭�� �� �ȼ��� ����ϴ� ��� ������ �׸� �� ����� �Լ��� ȣ���ϴ� ������ ����
                // �����δ� OpenGL �Ǵ� DirectX�� ���� �׷��� ���̺귯���� ����ؾ� ��
                drawPixel(Point2(x, y), c);
            }
        }
    }
}


bool Triangle::isInside(const Point2& p) const {
    float x0 = points[0]->x, y0 = points[0]->y;
    float x1 = points[1]->x, y1 = points[1]->y;
    float x2 = points[2]->x, y2 = points[2]->y;

    float alpha = ((y1 - y2) * (p.x - x2) + (x2 - x1) * (p.y - y2)) / ((y1 - y2) * (x0 - x2) + (x2 - x1) * (y0 - y2));
    float beta = ((y2 - y0) * (p.x - x2) + (x0 - x2) * (p.y - y2)) / ((y1 - y2) * (x0 - x2) + (x2 - x1) * (y0 - y2));
    float gamma = 1.0f - alpha - beta;

    return alpha >= 0 && beta >= 0 && gamma >= 0;
}

Color Triangle::interpolateColor(const Point2& p) const {
    float x0 = points[0]->x, y0 = points[0]->y;
    float x1 = points[1]->x, y1 = points[1]->y;
    float x2 = points[2]->x, y2 = points[2]->y;

    float alpha = ((y1 - y2) * (p.x - x2) + (x2 - x1) * (p.y - y2)) / ((y1 - y2) * (x0 - x2) + (x2 - x1) * (y0 - y2));
    float beta = ((y2 - y0) * (p.x - x2) + (x0 - x2) * (p.y - y2)) / ((y1 - y2) * (x0 - x2) + (x2 - x1) * (y0 - y2));
    float gamma = 1.0f - alpha - beta;

    Color& c0 = *colors[0], & c1 = *colors[1], & c2 = *colors[2];
    return Color::lerp(c0, c1, alpha) + Color::lerp(c1, c2, beta) + Color::lerp(c2, c0, gamma);
}

void Triangle::drawPixel(const Point2& p, const Color& c) const {
    std::cout << "Drawing pixel at (" << p.x << ", " << p.y << ") with color (" << c.r << ", " << c.g << ", " << c.b << ")" << std::endl;
}