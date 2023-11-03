#include "Image.h"
#define STB_IMAGE_IMPLEMENTATION
#include "OpenGL/stb_image.h"

unsigned char* CImage::LoadImg(std::string filename, int* x, int* y, int* comp, int req_comp)
{
	return stbi_load(filename.c_str(), x, y, comp, req_comp);
}

void CImage::FreeImg(void* data)
{
	stbi_image_free(data);
}
