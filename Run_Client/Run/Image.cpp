#include "Image.h"
#define STB_IMAGE_IMPLEMENTATION
#include "OpenGL/stb_image.h"

unsigned char* CImage::LoadImg(std::string filename, int* x, int* y, int* channels_in_file, int desired_channels)
{
	return stbi_load(filename.c_str(), x, y, channels_in_file, desired_channels);
}

void CImage::FreeImg(void* data)
{
	stbi_image_free(data);
}
