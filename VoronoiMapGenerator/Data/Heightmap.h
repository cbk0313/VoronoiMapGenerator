#pragma once

#include <iostream>
#include <vector>


class Heightmap {
	unsigned int Width;
	unsigned int Height;
    std::vector<uint16_t> Data;


public:
	Heightmap(unsigned int w, unsigned int h);
	~Heightmap();

	void Dispose();

	uint16_t& operator[](size_t index);
	uint16_t& Get(unsigned int x, unsigned int y);
	uint16_t* GetArray();
	std::vector<uint16_t>& GetData();

	void BoxBlur(int blurSize);
	void Sharpen();
	//void EdgeDetection();
	void GaussianBlur(int size, float sigma);

	void ClearImage();
	void SaveImage(const char* filename);
};