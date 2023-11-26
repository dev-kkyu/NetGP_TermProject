#pragma once

#include "protocol.h"
#include <vector>

struct map_rect
{
	struct map_line
	{
		float line[4];
		float& operator[](int idx);
	};

	map_line rect[4];
	map_line& operator[](int idx);
};

class CPlayerManager
{
public:
	PlayerData info;
	std::vector<map_rect> map_data;

	bool isLeft;
	bool isRight;
	bool isSpace;
	float acc_x;

	bool isBottom;
	bool isDrop;
	float basic_v;
	float velocity;

	float stop_time;

	bool is_rotating;
	float bef_mv_x;
	float bef_mv_y;

public:
	CPlayerManager();
	~CPlayerManager();

	void Initialize();

	void Update(float ElapsedTime);

	bool isOffTile();
	void MoveBackOnTile();

	void SetRotate();
	void RotateMap(float ElapsedTime);
};

