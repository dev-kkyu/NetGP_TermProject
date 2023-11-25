#include "PlayerManager.h"
#include <fstream>
#include <string>
#include <sstream>

CPlayerManager::CPlayerManager() : info{}
{
	info.z = 0.f;
	info.map_index = -10;
	info.bottom_index = 2;

	isLeft = isRight = isSpace = false;
	info.x = 0.f;
	acc_x = 0.f;

	isBottom = true;
	isDrop = false;
	info.y = 0.f;
	basic_v = 100.f;
	velocity = basic_v;

	stop_time = 0.f;

	info.is_walk = true;

	// 회전용 변수
	info.is_rotating = false;
	info.now_angle = 0.f;
	info.bef_mv_x = info.x;
	info.bef_mv_y = info.y;

	Initialize();
}

CPlayerManager::~CPlayerManager()
{
}

void CPlayerManager::Initialize()
{
	std::string filename = "./map1.txt";
	std::ifstream in{ filename };
	if (!in) {
		std::cerr << filename << " 열기 실패" << std::endl;
	}
	std::string str;
	while (std::getline(in, str)) {
		std::stringstream ss{ str };
		map_rect mati;
		for (int i = 0; i < 4; ++i) {
			map_rect::map_line line;
			ss >> line[0] >> line[1] >> line[2] >> line[3];
			mati[i] = line;
		}
		map_data.push_back(mati);
	}
	if (not map_data.empty()) {
		std::cout << filename << " 맵 로드 완료" << std::endl;
	}
}

void CPlayerManager::Update(float ElapsedTime)
{
	if (info.is_rotating)
		RotateMap(ElapsedTime);
	else if (stop_time > 0.f) {
		stop_time -= 1.f * ElapsedTime;
		if (stop_time <= 0.f) {
			isBottom = true;
			stop_time = 0.f;
			info.is_walk = true;
		}
	}
	else {
		info.z += 3.5f * ElapsedTime;
		if (info.z > 1.f) {
			info.z -= 1.f;
			++info.map_index;		// map_index의 의미. 현재 0번위치의 사각형부터 map_data[map_index]의 지형을 본다.
		}

		// 점프키
		if (isBottom and isSpace) {
			info.is_walk = false;
			isBottom = false;
		}
		// 방향키
		if (isLeft or isRight)
			acc_x = float(-(int)isLeft + (int)isRight);
		else if (acc_x != 0.f) {
			if (acc_x > 0)
				acc_x -= ElapsedTime * 5.f;
			else
				acc_x += ElapsedTime * 5.f;
			if (abs(acc_x) < 0.2f)
				acc_x = 0.f;
		}
		info.x += acc_x * 3.f * ElapsedTime;
		// 벽 제한
		if (info.x >= 1.75f or info.x <= -1.75f) {
			info.x = info.x >= 1.75f ? 1.75f : -1.75f;
			if (not isDrop) {
				SetRotate();
				RotateMap(ElapsedTime);
				info.is_walk = false;
			}
		}


		// 중력
		if (isBottom and isOffTile()) {
			isBottom = false;
			isDrop = true;
			velocity = -1.f;
		}

		if (not isBottom) {
			info.y += pow(velocity, 2) * 9.8f / 12000.f * ElapsedTime * (velocity < 0 ? -1.f : 1.f);
			if (info.y <= 0.f and isOffTile())
				isDrop = true;
			if (info.y <= -5.f or (not isDrop and info.y <= 0.f and not isOffTile())) {
				if (info.y <= -0.5f) {
					MoveBackOnTile();
					info.is_walk = false;
					stop_time = 0.5f;
				}
				else
					info.is_walk = true;
				info.y = 0.f;
				isBottom = true;
				isDrop = false;
				velocity = basic_v;
			}
			else {
				velocity -= 250 * ElapsedTime;
			}
		}
	}
}

bool CPlayerManager::isOffTile()
{
	int posIdx = round(info.x + 1.5f);
	if (info.map_index >= -2 and info.map_index < 99) {
		if (info.z <= 0.5f and info.map_index >= -1) {
			int lineIdx = info.map_index + 1;
			if (not map_data[lineIdx][info.bottom_index][posIdx])
				return true;
		}
		else if (info.z > 0.5f and info.map_index < 98) {
			int lineIdx = info.map_index + 2;
			if (not map_data[lineIdx][info.bottom_index][posIdx])
				return true;
		}
	}
	return false;
}

void CPlayerManager::MoveBackOnTile()
{
	int my_index;
	int posIdx;
	for (int i = 1;; ++i) {
		posIdx = round(info.x + 1.5f);
		my_index = info.map_index - i;
		if (my_index + 1 < 0)
			break;
		if (map_data[my_index + 1][info.bottom_index][posIdx])
			break;
	}
	info.map_index = my_index;
	info.x = posIdx - 1.5f;
	info.z = 0.f;
}

void CPlayerManager::SetRotate()
{
	info.is_rotating = true;
	info.now_angle = 0.f;
	info.bef_mv_x = info.x;
	info.bef_mv_y = info.y;
}

void CPlayerManager::RotateMap(float ElapsedTime)
{
	static const float time_sec = 0.5f;
	if (info.bef_mv_x == -1.75f) {
		float finalx = 1.75f - info.bef_mv_y;
		info.now_angle += 90.f / time_sec * ElapsedTime;
		info.y -= info.bef_mv_y / time_sec * ElapsedTime;
		info.x += (finalx - info.bef_mv_x) / time_sec * ElapsedTime;
		if (info.now_angle >= 90.f) {
			info.is_rotating = false;
			info.is_walk = true;
			info.bottom_index -= 1;
			info.now_angle = 0.f;
			info.y = 0.f;
			if (info.x >= 1.75)
				info.x = 1.7499;
			if (info.bottom_index == -1)
				info.bottom_index = 3;
		}
	}
	else if (info.bef_mv_x == 1.75f) {
		float finalx = -1.75f + info.bef_mv_y;
		info.now_angle -= 90.f / time_sec * ElapsedTime;
		info.y -= info.bef_mv_y / time_sec * ElapsedTime;
		info.x -= (info.bef_mv_x - finalx) / time_sec * ElapsedTime;
		if (info.now_angle <= -90.f) {
			info.is_rotating = false;
			info.is_walk = true;
			info.bottom_index += 1;
			info.now_angle = 0.f;
			info.y = 0.f;
			if (info.x <= -1.75)
				info.x = -1.7499;
			if (info.bottom_index == 4)
				info.bottom_index = 0;
		}
	}
}

map_rect::map_line& map_rect::operator[](int idx)
{
	return rect[idx];
}

float& map_rect::map_line::operator[](int idx)
{
	return line[idx];
}
