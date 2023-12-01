#pragma once
#include <vector>

struct MapRect
{
	struct MapLine
	{
		float line[4];
		float& operator[](int idx);
	};

	MapLine rect[4];
	MapLine& operator[](int idx);

	MapRect();
	MapRect(float value);

	static std::vector<MapRect> make_map();
};
