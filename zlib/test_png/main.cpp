#include <iostream>
#include <vector>
#include <png.h>

// PNG 저장 함수
void Save16BitGrayscalePNG(const std::string& filename, int width, int height) {
    FILE *fp = fopen(filename.c_str(), "wb");
    if(!fp) {
        std::cerr << "Error: Cannot open file " << filename << std::endl;
        return;
    }

    // PNG 구조체 생성
    png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
    if (!png) {
        std::cerr << "Error: png_create_write_struct failed" << std::endl;
        fclose(fp);
        return;
    }

    // PNG info 구조체 생성
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

    // PNG header 작성
    png_set_IHDR(
        png,
        info,
        width,
        height,
        16, // 비트 깊이
        PNG_COLOR_TYPE_GRAY,
        PNG_INTERLACE_NONE,
        PNG_COMPRESSION_TYPE_DEFAULT,
        PNG_FILTER_TYPE_DEFAULT
    );
    png_write_info(png, info);
    png_set_swap(png);    // <--- THIS LINE ADDED
    // 이미지 데이터 작성
    std::vector<uint16_t> image(width * height);
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            uint16_t grayValue = static_cast<uint16_t>((x * 65535) / width); // 그라디언트 (0 to 65535)
            image[y * width + x] = grayValue;
        }
    }

    // PNG 파일에 데이터 작성
    std::vector<png_bytep> rows(height);
    for (int y = 0; y < height; ++y) {
        rows[y] = reinterpret_cast<png_bytep>(&image[y * width]);
    }
    png_write_image(png, rows.data());

    // PNG 파일 종료
    png_write_end(png, nullptr);
    png_destroy_write_struct(&png, &info);
    fclose(fp);
    std::cout << "File " << filename << " saved successfully." << std::endl;
}

int main() {
    int width = 640;
    int height = 480;
    Save16BitGrayscalePNG("output_grayscale.png", width, height);
    return 0;
}