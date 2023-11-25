#pragma once
#include <string>

class CImage
{
public:
	//stbi_set_flip_vertically_on_load(true); //--- �̹����� �Ųٷ� �����ٸ� �߰�
	static unsigned char* LoadImg(std::string filename, int* x, int* y, int* channels_in_file, int desired_channels);
	static void FreeImg(void* data);
};

