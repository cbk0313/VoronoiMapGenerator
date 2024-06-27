#include "Point2.h"

#include <math.h>

Point2::Point2() : x(0), y(0) {}

Point2::Point2(const Point2& p) : x(p[0]), y(p[1]) {}

Point2::Point2(double _x, double _y) : x(_x), y(_y) {}
Point2::Point2(int _x, int _y) : x((double)_x), y((double)_y) {}
Point2::Point2(unsigned int _x, unsigned int _y) : x((double)_x), y((double)_y) {}

Point2& Point2::operator=(const Point2& a) {
    x = a[0];
    y = a[1];
    return *this;
}

const double& Point2::operator[](int n) const {
	return (&x)[n];
}

double& Point2::operator[](int n) {
	return (&x)[n];
}

Point2& Point2::operator+=(const Vector2& v) {
    x += v[0];
    y += v[1];
    return *this;
}

Point2& Point2::operator-=(const Vector2& v) {
    x -= v[0];
    y -= v[1];
    return *this;
}

Point2& Point2::operator*=(double s) {
    x *= s;
    y *= s;
    return *this;
}

Point2& Point2::operator/=(double s) {
    x /= s;
    y /= s;
    return *this;
}

Point2 Point2::operator-(const Point2 & p) const {
    return Point2(x - p.x, y - p.y);
}

Point2 Point2::operator+(const Vector2 & v) const {
    return Point2(x + v[0], y + v[1]);
}

Point2 Point2::operator-(const Vector2 & v) const {
    return Point2(x - v[0], y - v[1]);
}

Point2 Point2::operator+(const Point2& v) const {
    return Point2(x + v[0], y + v[1]);
}

//Point2 Point2::operator -(const Point2& v) {
//    return Point2(x - v[0], y - v[1]);
//}


Point2 Point2::operator*(const double s) const {
    return Point2(x * s, y * s);
}

Point2 Point2::operator/(const double s) const {
    return Point2(x / s, y / s);
}

bool Point2::SitesOrdered(const Point2& s1, const Point2& s2) {
    if (s1.y < s2.y)
        return true;
    if (s1.y == s2.y && s1.x < s2.x)
        return true;

    return false;
}

double Point2::Distance(const Point2& a, const Point2& b) {
    return sqrt((a.x - b.x) * (a.x - b.x) +
        (a.y - b.y) * (a.y - b.y));
}

double Point2::DistanceTo(const Point2& p) const {
    return sqrt((p[0] - x) * (p[0] - x) + 
                (p[1] - y) * (p[1] - y));
}

double Point2::DistanceToSquared(const Point2& p) const {
    return ((p[0] - x) * (p[0] - x) +
            (p[1] - y) * (p[1] - y));
}

double Point2::DistanceFromOrigin() const {
    return sqrt(x * x + y * y);
}

double Point2::DistanceFromOriginSquared() const {
    return x * x + y * y;
}

double Point2::GetAngle() const {
    return atan2(y, x) * 180.0 / 3.1415926f;
}

Point2 Point2::Normailize() const {
    double dist = DistanceFromOrigin();
    return Point2(x / dist, y / dist);
}

bool Point2::operator==( const Point2 &p ) const {
    return x == p.x && y == p.y;
}

bool Point2::operator!=( const Point2 &p ) const {
    return x != p.x || y != p.y;
}

void Point2::print() const {
    std::cout << x << " " << y << "\n";
}
