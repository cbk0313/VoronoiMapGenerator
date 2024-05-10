#pragma once

class Point2;
class Color;
class Triangle {
public:
	Point2* points[3];
	Color* colors[3];

	// �ﰢ���� �׸��� �Լ�
	void draw();

private:
	// �ﰢ���� �ּ� x�� ��ȯ
	float minX() const {
		return std::min({ points[0]->x, points[1]->x, points[2]->x });
	}

	// �ﰢ���� �ִ� x�� ��ȯ
	float maxX() const {
		return std::max({ points[0]->x, points[1]->x, points[2]->x });
	}

	// �ﰢ���� �ּ� y�� ��ȯ
	float minY() const {
		return std::min({ points[0]->y, points[1]->y, points[2]->y });
	}

	// �ﰢ���� �ִ� y�� ��ȯ
	float maxY() const {
		return std::max({ points[0]->y, points[1]->y, points[2]->y });
	}

	// Ư�� ���� �ﰢ�� ���ο� �ִ��� Ȯ���ϴ� �Լ�
	bool isInside(const Point2& p) const;
	// �ﰢ�� ������ ������ ���� �����Ͽ� ��ȯ�ϴ� �Լ�
	Color interpolateColor(const Point2& p) const;

	// Ư�� ���� ������ �׸��� �Լ� (�����δ� �׷��� ���̺귯���� �Լ��� ȣ���ؾ� ��)
	void drawPixel(const Point2& p, const Color& c) const;
};