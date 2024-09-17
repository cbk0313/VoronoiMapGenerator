#include "Setting.h"

GenerateSetting::GenerateSetting()
	: type(MapType::CONTINENT)
	, seed(0)
	, nPoints(10000)
	, dimension(1000000)
	, site_range(0.666)
	, sea_level(0.5)
	, coast_area(2)
	, radius(500000) // radius, 0, 0.7, 10, radius / 3, radius / 5, 10, radius / 5, radius / 7)
	, lake_scale(0)
	, lake_size(0.7)
	, island_cnt(10)
	, island_radius_max(333333)
	, island_radius_min(200000)
	, lake_cnt(10)
	, lake_radius_max(200000)
	, lake_radius_min(142857.1428571429)
	, river_radius(500)
	, river_power_scale(0.2)
	, river_curv_spacing(0.02f)
	, river_additional_curve_chance(0.5)
	, river_additional_curve_distance(0.5)
	, randgen(RandomGenerator(0))
	, cell_size(0)
{
	cell_size = (dimension / sqrt(nPoints)) / 2;
};




GenerateSetting::GenerateSetting(MapType _type, int _seed, signed int _nPoints,
	signed int _dimension, double _site_range, double _sea_level, int _coast_area,
	double _radius, double _lake_scale, double _lake_size, unsigned int _island_cnt,
	double _island_radius_max, double _island_radius_min, unsigned int _lake_cnt,
	double _lake_radius_max, double _lake_radius_min, double _river_radius, double _river_power_scale,
	double _river_curv_spacing, double _river_additional_curve_chance, double _additional_curve_distance)
	: randgen(RandomGenerator(_seed))
	, cell_size(0)
{

	SetMapType(_type);
	SetSeed(_seed);
	SetPoints(_nPoints);
	SetDimension(_dimension);
	SetSiteRange(_site_range);

	SetSeaLevel(_sea_level);
	SetCoastArea(_coast_area);

	SetRadius(_radius);
	SetLakeScale(_lake_scale);
	SetLakeSize(_lake_size);
	SetIslandCount(_island_cnt);
	SetIslandRadiusMax(_island_radius_max);
	SetIslandRadiusMin(_island_radius_min);
	SetLakeCount(_lake_cnt);
	SetLakeRadiusMax(_lake_radius_max);
	SetLakeRadiusMin(_lake_radius_min);
	SetRiverRadius(_river_radius);
	SetRiverPowerScale(_river_power_scale);
	SetRiverCurvSpacing(_river_curv_spacing);
	SetRiverAdditionalCurveChance(_river_additional_curve_chance);
	SetRiverAdditionalCurveDistance(_additional_curve_distance);

	cell_size = (dimension / sqrt(nPoints)) / 2;
};

