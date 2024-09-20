#pragma once

#include <math.h>

#define ROUND_DECIMAL_3 1000
#define ROUND_DECIMAL_2 100
#define ROUND_DECIMAL_1 10

namespace voronoi_map_generator {
	inline double Round3(double num) {
		return round(num * ROUND_DECIMAL_3) / ROUND_DECIMAL_3;
	}

	inline double Round2(double num) {
		return round(num * ROUND_DECIMAL_2) / ROUND_DECIMAL_2;
	}

	inline double Round1(double num) {
		return round(num * ROUND_DECIMAL_1) / ROUND_DECIMAL_1;
	}

	inline double ConvertRange(double value, double oldMin, double oldMax, double newMin, double newMax) {
		return (value - oldMin) / (oldMax - oldMin) * (newMax - newMin) + newMin;
	}
}


#undef ROUND_DECIMAL_3
#undef ROUND_DECIMAL_2
#undef ROUND_DECIMAL_1