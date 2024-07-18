#pragma once
#include <algorithm>

enum class MapType {
	CONTINENT,
	ISLAND
};

class GenerateSetting {
	//friend class VoronoiDiagramGenerator;
private:
	MapType type;
	int seed;
	double site_range;
	double radius;
	double lake_scale;
	double lake_size;

	unsigned int island_cnt;
	double island_radius_max;
	double island_radius_min;

	unsigned int lake_cnt;
	double lake_radius_max;
	double lake_radius_min;

	double river_radius;
	double river_power_scale;
	double river_curv_spacing;
	double river_additional_curve_chance;
	double river_additional_curve_distance;
public:
	GenerateSetting()
		: type(MapType::CONTINENT)
		, seed(0)
		, site_range(0.666)
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
		, river_additional_curve_distance(0.5) {};

	GenerateSetting(MapType _type, int _seed, double _site_range, double _radius, double _lake_scale,
		double _lake_size, unsigned int _island_cnt, double _island_radius_max,
		double _island_radius_min, unsigned int _lake_cnt, double _lake_radius_max,
		double _lake_radius_min, double _river_radius, double _river_power_scale, 
		double _river_curv_spacing, double _river_additional_curve_chance, double _additional_curve_distance);


	inline void SetMapType(MapType new_type) { type = new_type; };
	inline void SetSeed(int new_seed) { seed = new_seed; };
	inline void SetSiteRange(int new_seed) { seed = new_seed; };
	inline void SetRadius(double new_radius) { radius = new_radius; };
	inline void SetLakeScale(double new_scale) { lake_scale = new_scale; };
	inline void SetLakeSize(double new_size) { lake_size = new_size; };
	inline void SetIslandCount(unsigned int new_cnt) { island_cnt = new_cnt; };
	inline void SetIslandRadiusMax(double new_radius) { island_radius_max = new_radius; };
	inline void SetIslandRadiusMin(double new_radius) { island_radius_min = new_radius; };
	inline void SetLakeCount(unsigned int new_cnt) { lake_cnt = new_cnt; };
	inline void SetLakeRadiusMax(double new_radius) { lake_radius_max = new_radius; };
	inline void SetLakeRadiusMin(double new_radius) { lake_radius_min = new_radius; };
	inline void SetRiverRadius(double new_radius) { river_radius = new_radius; };
	inline void SetRiverPowerScale(double new_sacle) { river_power_scale = new_sacle; };
	inline void SetRiverCurvSpacing(double new_curvSpacing) { river_curv_spacing = new_curvSpacing; };
	inline void SetRiverAdditionalCurveChance(double new_curvSpacing) { river_additional_curve_chance = new_curvSpacing; };
	inline void SetRiverAdditionalCurveDistance(double new_dist) { river_additional_curve_distance = new_dist; };

	inline MapType GetMapType() { return type; };
	inline int GetSeed() { return seed; };
	inline double GetSiteRange() { return site_range; };
	inline double GetRadius() { return radius; };
	inline double GetLakeScale() { return lake_scale; };
	inline double GetLakeSize() { return lake_size; };
	inline unsigned int GetIslandCount() { return island_cnt; };
	inline double GetIslandRadiusMax() { return island_radius_max; };
	inline double GetIslandRadiusMin() { return island_radius_min; };
	inline unsigned int GetLakeCount() { return lake_cnt; };
	inline double GetLakeRadiusMax() { return lake_radius_max; };
	inline double GetLakeRadiusMin() { return lake_radius_min; };
	inline double GetRiverRadius() { return river_radius; };
	inline double GetRiverPowerScale() { return river_power_scale; };
	inline double GetRiverCurvSpacing() { return river_curv_spacing; };
	inline double GetRiverAdditionalCurveChance() { return river_additional_curve_chance; };
	inline double GetRiverAdditionalCurveDistance() { return river_additional_curve_distance; };

	static inline double GetRandom() {
		return rand() / ((double)0x7fff); // RAND_MAX 
	}

	inline void Srand() { srand(seed); }

};