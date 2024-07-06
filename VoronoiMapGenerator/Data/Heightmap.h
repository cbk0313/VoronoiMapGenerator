#pragma once

#include <iostream>

class Heightmap {
	bool is_grayscale;
	unsigned int width;
	unsigned int height;
	uint16_t* data;
public:
	Heightmap(unsigned int w, unsigned int h, bool grayscale = false);
	~Heightmap();

	void Dispose();

	uint16_t& operator[](size_t index);
	uint16_t& Get(unsigned int x, unsigned int y);
	uint16_t* GetArray();
};