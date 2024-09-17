#pragma once
#include <algorithm>
#include <random>

enum class MapType {
	CONTINENT,
	ISLAND
};

class RandomGenerator {
public:
	RandomGenerator(int seed) : engine((unsigned int)seed) {}

	inline double GetRandom() const {
		std::uniform_real_distribution<double> dist(0.0, 1.0);
		return dist(engine);
	}

	inline int GetRandomInt() const {
		std::uniform_int_distribution<int> dist(0, 0x7fff);
		return dist(engine);
	}

private:
	mutable std::mt19937 engine;
};


class GenerateSetting {
	friend class VoronoiDiagramGenerator;
private:
	MapType type;
	int seed;
	signed int nPoints;
	signed int dimension;
	double site_range;

	double sea_level;
	int coast_area;

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

	double cell_size;
private:
	RandomGenerator randgen;
public:
	GenerateSetting();

	GenerateSetting(MapType _type, int _seed, signed int _nPoints, signed int _dimension, double _site_range, double _sea_level, int _coast_area, double _radius, double _lake_scale,
		double _lake_size, unsigned int _island_cnt, double _island_radius_max,
		double _island_radius_min, unsigned int _lake_cnt, double _lake_radius_max,
		double _lake_radius_min, double _river_radius, double _river_power_scale, 
		double _river_curv_spacing, double _river_additional_curve_chance, double _additional_curve_distance);


	inline void SetMapType(MapType new_type) { type = new_type; };
	inline void SetSeed(int new_seed) { seed = new_seed; };
	inline void SetPoints(int new_points) { nPoints = new_points; };
	inline void SetDimension(int new_dimension) { dimension = new_dimension; };
	inline void SetSiteRange(double new_site_range) { site_range = new_site_range; };

	inline void SetSeaLevel(double new_sea_level) { sea_level = std::clamp<double>(new_sea_level, 0, 1.0); };
	inline void SetCoastArea(int new_sea_level) { coast_area = std::max<int>(new_sea_level, 1); };
	
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

	inline int GetPoints() const { return nPoints; };
	inline int GetDimension() const { return dimension; };
	inline MapType GetMapType() const { return type; };
	inline int GetSeed() const { return seed; };
	inline double GetSiteRange() const { return site_range; };
	inline double GetSeaLevel() const { return sea_level; };
	inline int GetCoastArea() const { return coast_area; };
	inline double GetRadius() const { return radius; };
	inline double GetLakeScale() const { return lake_scale; };
	inline double GetLakeSize() const { return lake_size; };
	inline unsigned int GetIslandCount() const { return island_cnt; };
	inline double GetIslandRadiusMax() const { return island_radius_max; };
	inline double GetIslandRadiusMin() const { return island_radius_min; };
	inline unsigned int GetLakeCount() const { return lake_cnt; };
	inline double GetLakeRadiusMax() const { return lake_radius_max; };
	inline double GetLakeRadiusMin() const { return lake_radius_min; };
	inline double GetRiverRadius() const { return river_radius; };
	inline double GetRiverPowerScale() const { return river_power_scale; };
	inline double GetRiverCurvSpacing() const { return river_curv_spacing; };
	inline double GetRiverAdditionalCurveChance() const { return river_additional_curve_chance; };
	inline double GetRiverAdditionalCurveDistance() const { return river_additional_curve_distance; };
	inline double GetCellSize() const { return cell_size; };

	inline double GetRandom() const {
		return randgen.GetRandom(); // RAND_MAX 
	}

	inline double GetRandomRound() const {
		return round(randgen.GetRandom() * 100) / 100;
	}

	inline int GetRandomInt() const {
		return randgen.GetRandomInt(); // RAND_MAX 
	}

	inline void Srand() { randgen = RandomGenerator(seed); }


};