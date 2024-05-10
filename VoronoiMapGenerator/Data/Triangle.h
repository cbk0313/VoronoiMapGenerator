#pragma once

class Point2;
class Color;
class Triangle {
public:
	Point2* points[3];
	Color* colors[3];

	// 삼각형을 그리는 함수
	void draw();

private:
	// 삼각형의 최소 x값 반환
	float minX() const {
		return std::min({ points[0]->x, points[1]->x, points[2]->x });
	}

	// 삼각형의 최대 x값 반환
	float maxX() const {
		return std::max({ points[0]->x, points[1]->x, points[2]->x });
	}

	// 삼각형의 최소 y값 반환
	float minY() const {
		return std::min({ points[0]->y, points[1]->y, points[2]->y });
	}

	// 삼각형의 최대 y값 반환
	float maxY() const {
		return std::max({ points[0]->y, points[1]->y, points[2]->y });
	}

	// 특정 점이 삼각형 내부에 있는지 확인하는 함수
	bool isInside(const Point2& p) const;
	// 삼각형 내부의 색상을 선형 보간하여 반환하는 함수
	Color interpolateColor(const Point2& p) const;

	// 특정 점에 색상을 그리는 함수 (실제로는 그래픽 라이브러리의 함수를 호출해야 함)
	void drawPixel(const Point2& p, const Color& c) const;
};