#pragma once


class Heightmap {
	bool is_grayscale;
	unsigned int width;
	unsigned int height;
	unsigned char* data;
public:
	Heightmap(unsigned int w, unsigned int h, bool grayscale = false);
	~Heightmap();

	void Dispose();

	unsigned char& operator[](size_t index);
	unsigned char& Get(unsigned int x, unsigned int y);
	unsigned char* GetArray();
};