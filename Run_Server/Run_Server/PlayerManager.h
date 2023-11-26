#pragma once

#include "protocol.h"
#include "MakeMap.h"
#include <vector>

class CPlayerManager
{
public:
	PlayerData info;

	bool isLeft;
	bool isRight;
	bool isSpace;

private:
	std::vector<MapRect> map_data;

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

	void SetMap(std::vector<MapRect> map_data);

	void Update(float ElapsedTime);

	bool isOffTile();
	void MoveBackOnTile();

	void SetRotate();
	void RotateMap(float ElapsedTime);
};

