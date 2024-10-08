#ifndef _POINT2_H_
#define _POINT2_H_

#include "Vector2.h"
#include <iostream>
#include <sstream>

class Point2 {
public:
	double x, y;

    Point2();
    Point2(const Point2& p);
    Point2(double _x, double _y);
    Point2(int _x, int _y);
    Point2(unsigned int _x, unsigned int _y);

    Point2& operator=(const Point2& a);  

	const double &operator[](int n) const;
	double &operator[](int n);

    Point2& operator+=(const Vector2& v);
    Point2& operator-=(const Vector2& v);
    Point2& operator*=(double s);
    Point2& operator/=(double s);

    Point2 operator-(const Point2 & p) const;
    Point2 operator+(const Vector2 & v) const;
    Point2 operator-(const Vector2 & v) const;
    Point2 operator+(const Point2 & v) const;
    //Point2 operator-(const Point2 & v) const;
    //Vector2 operator=(const Vector2D& other) {
    Point2 operator*(const double s) const;
    Point2 operator/(const double s) const;

    static bool SitesOrdered(const Point2& s1, const Point2& s2);

    static double Distance(const Point2& a, const Point2& b);
    double DistanceTo(const Point2& p) const;
    double DistanceToSquared(const Point2& p) const;
    double DistanceFromOrigin() const;
    double DistanceFromOriginSquared() const;
    void Epsilon();

    double GetAngle() const;
    Point2 Normalize() const;

    static double Dot(const Point2& a, const Point2& b);

    static double Cross(const Point2& a, const Point2& b);
    static double AngleBetween(const Point2& vec1, const Point2& vec2);
    static double AngleBetweenAbs(const Point2& vec1, const Point2& vec2);

    bool operator==( const Point2 &p ) const;
    bool operator!=( const Point2 &p ) const;

    void print() const;
};

inline Point2 lerp( const Point2& p0, const Point2& p1, double dT ){
    const double dTMinus = 1.0 - dT;
    return Point2( dTMinus * p0[0] + dT * p1[0], dTMinus * p0[1] + dT * p1[1]); 
}

inline std::ostream& operator<<(std::ostream& os, const Point2& p){
    os << "(" << p[0] << ", " << p[1] << ")";
    return os;
}

#endif
