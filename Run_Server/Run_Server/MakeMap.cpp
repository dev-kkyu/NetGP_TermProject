#include "MakeMap.h"
#include <random>

static std::random_device rd;
static std::default_random_engine dre{ rd() };

MapRect::MapLine& MapRect::operator[](int idx)
{
	return rect[idx];
}

MapRect::MapRect() : rect{}
{
}

MapRect::MapRect(float value)
	: rect{ {value, value, value, value},
	{value, value, value, value},
	{value, value, value, value},
	{value, value, value, value}
	}
{
}

float& MapRect::MapLine::operator[](int idx)
{
	return line[idx];
}

std::vector<MapRect> MapRect::make_map()
{
	// 맵 생성
	std::uniform_int_distribution<int> uid{ 0, 2 };		// 30퍼 확률로 비어있다.
	std::vector<MapRect> map_data;

	for (int i = 0; i < 100; ++i) {
		MapRect mati;
		for (int j = 0; j < 4; ++j) {
			MapRect::MapLine line;
			for (int k = 0; k < 4; ++k)
				line[k] = !!uid(dre);
			mati[j] = line;
		}
		map_data.push_back(mati);
	}
	return map_data;
}
