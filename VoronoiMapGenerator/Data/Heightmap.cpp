#pragma once
#include "Heightmap.h"
#include <algorithm>
#include "png.h"

#define M_PI 3.14159265358979323846

Heightmap::Heightmap(unsigned int w, unsigned int h) : Width(w), Height(h) {
	//data = new uint16_t[w * h];
	Data = std::vector<uint16_t>(w * h);
}

Heightmap::~Heightmap() {
	//Dispose();
}

//void Heightmap::Dispose() {
//	//delete[] data;
//}

uint16_t& Heightmap::operator[](size_t index) {
	return Data[index];
}

uint16_t& Heightmap::Get(unsigned int x, unsigned int y) {
	unsigned int pos = x + Width * (Height - y - 1);
	return Data[pos];

}

uint16_t* Heightmap::GetArray() {
	return Data.data();
}

std::vector<uint16_t>& Heightmap::GetData() {
	return Data;
}




void Heightmap::BoxBlur(int blurSize) {

	const std::vector<uint16_t> inputImage = Data;

	long sum;
	int count;
	int halfSize = blurSize / 2;
	const int width = (int)Width;
	const int height = (int)Width;
	for (int y = 0; y < height; ++y) {
		for (int x = 0; x < width; ++x) {
			sum = 0;
			count = 0;

			for (int ky = -halfSize; ky <= halfSize; ++ky) {
				for (int kx = -halfSize; kx <= halfSize; ++kx) {
					int nx = std::clamp(x + kx, 0, width - 1);
					int ny = std::clamp(y + ky, 0, height - 1);
					sum += inputImage[ny * width + nx];
					count++;
				}
			}

			Data[y * width + x] = sum / count;
		}
	}
}


void Heightmap::Sharpen() {

	int kernel[3][3] = {
		{0, -1, 0},
		{-1, 5, -1},
		{0, -1, 0}
	};

	const std::vector<uint16_t> inputImage = Data;
	const int width = (int)Width;
	const int height = (int)Width;

	for (int y = 1; y < height - 1; ++y) {
		for (int x = 1; x < width - 1; ++x) {
			int sum = 0;

			for (int ky = -1; ky <= 1; ++ky) {
				for (int kx = -1; kx <= 1; ++kx) {
					sum += inputImage[(y + ky) * width + (x + kx)] * kernel[ky + 1][kx + 1];
				}
			}

			Data[y * width + x] = std::clamp(sum, 0, 65535);  // 16-bit uint의 범위를 유지
		}
	}
}


//void Heightmap::EdgeDetection() {
//    int sobelX[3][3] = {
//        {-1, 0, 1},
//        {-2, 0, 2},
//        {-1, 0, 1}
//    };
//
//    int sobelY[3][3] = {
//        {-1, -2, -1},
//        {0, 0, 0},
//        {1, 2, 1}
//    };
//
//    for (int y = 1; y < height - 1; ++y) {
//        for (int x = 1; x < width - 1; ++x) {
//            int sumX = 0;
//            int sumY = 0;
//
//            for (int ky = -1; ky <= 1; ++ky) {
//                for (int kx = -1; kx <= 1; ++kx) {
//                    sumX += inputImage[(y + ky) * width + (x + kx)] * sobelX[ky + 1][kx + 1];
//                    sumY += inputImage[(y + ky) * width + (x + kx)] * sobelY[ky + 1][kx + 1];
//                }
//            }
//
//            int magnitude = std::clamp(static_cast<int>(std::sqrt(sumX * sumX + sumY * sumY)), 0, 65535);
//            outputImage[y * width + x] = magnitude;
//        }
//    }
//}

void Heightmap::GaussianBlur(int size, float sigma) {

	int halfSize = size / 2;
	std::vector<std::vector<float>> kernel(size, std::vector<float>(size));
	float sum = 0.0;

	for (int y = -halfSize; y <= halfSize; ++y) {
		for (int x = -halfSize; x <= halfSize; ++x) {
			float value = (1.0 / (2.0 * M_PI * sigma * sigma)) * std::exp(-(x * x + y * y) / (2 * sigma * sigma));
			kernel[y + halfSize][x + halfSize] = value;
			sum += value;
		}
	}

	for (int y = 0; y < size; ++y) {
		for (int x = 0; x < size; ++x) {
			kernel[y][x] /= sum;
		}
	}

	const std::vector<uint16_t> inputImage = Data;
	const int width = (int)Width;
	const int height = (int)Width;


	for (int y = 1; y < height - 1; ++y) {
		for (int x = 1; x < width - 1; ++x) {
			float sum = 0.0;

			for (int ky = -1; ky <= 1; ++ky) {
				for (int kx = -1; kx <= 1; ++kx) {
					sum += inputImage[(y + ky) * width + (x + kx)] * kernel[ky + 1][kx + 1];
				}
			}

			Data[y * width + x] = std::clamp(static_cast<int>(sum), 0, 65535);
		}
	}
}


void Heightmap::ClearImage() {
	std::fill(Data.begin(), Data.end(), 0);
}

void Heightmap::SaveImage(const char* filename) {
	auto start = std::clock();
	FILE* fp = nullptr;
	errno_t err = fopen_s(&fp, filename, "wb");
	if (err != 0) {
		std::cout << "File write error!!\n";
		return;
	}
	if (!fp) {
		std::cerr << "Error: Cannot open file " << filename << std::endl;
		return;
	}


	png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
	if (!png) {
		std::cerr << "Error: png_create_write_struct failed" << std::endl;
		fclose(fp);
		return;
	}

	// PNG info struct
	png_infop info = png_create_info_struct(png);
	if (!info) {
		std::cerr << "Error: png_create_info_struct failed" << std::endl;
		png_destroy_write_struct(&png, nullptr);
		fclose(fp);
		return;
	}

	if (setjmp(png_jmpbuf(png))) {
		std::cerr << "Error: png error during init_io" << std::endl;
		png_destroy_write_struct(&png, &info);
		fclose(fp);
		return;
	}

	png_init_io(png, fp);

	// PNG header
	png_set_IHDR(
		png,
		info,
		Width,
		Height,
		16, // bit depth
		PNG_COLOR_TYPE_GRAY,
		PNG_INTERLACE_NONE,
		PNG_COMPRESSION_TYPE_DEFAULT,
		PNG_FILTER_TYPE_DEFAULT
	);
	png_write_info(png, info);
	png_set_swap(png);


	std::vector<png_bytep> rows(Height);
	for (int y = 0; y < Height; ++y) {
		rows[y] = reinterpret_cast<png_bytep>(&Data[(Height - y - 1) * Width]);
	}
	
	png_write_image(png, rows.data());
	

	// PNG 파일 종료
	png_write_end(png, nullptr);
	png_destroy_write_struct(&png, &info);
	fclose(fp);



	//auto duration2 = 1000 * (std::clock() - start) / (double)CLOCKS_PER_SEC;
	auto duration = 1000 * (std::clock() - start) / (double)CLOCKS_PER_SEC;
	std::cout << "file saved: " << filename << ", save time: " << duration << "ms\n";

}