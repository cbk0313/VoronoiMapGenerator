#include "Setting.h"

GenerateSetting::GenerateSetting(MapType _type, int _seed, signed int _nPoints,
	signed int _dimension, double _site_range, double _sea_level, int _coast_area,
	double _radius, double _lake_scale, double _lake_size, unsigned int _island_cnt,
	double _island_radius_max, double _island_radius_min, unsigned int _lake_cnt,
	double _lake_radius_max, double _lake_radius_min, double _river_radius, double _river_power_scale,
	double _river_curv_spacing, double _river_additional_curve_chance, double _additional_curve_distance)
	: type(_type)
	, seed(_seed)
	, nPoints(_nPoints)
	, dimension(_dimension)
	, site_range(std::clamp(_site_range, 0.0, 1.0))
	, sea_level(_sea_level)
	, coast_area(_coast_area)
	, radius(_radius)
	, lake_scale(_lake_scale)
	, lake_size(_lake_size)
	, island_cnt(_island_cnt)
	, island_radius_max(_island_radius_max)
	, island_radius_min(_island_radius_min)
	, lake_cnt(_lake_cnt)
	, lake_radius_max(_lake_radius_max)
	, lake_radius_min(_lake_radius_min)
	, river_radius(_river_radius)
	, river_power_scale(_river_power_scale)
	, river_curv_spacing(_river_curv_spacing)
	, river_additional_curve_chance(_river_additional_curve_chance)
	, river_additional_curve_distance(_additional_curve_distance)
	, randgen(RandomGenerator(_seed))
{};

