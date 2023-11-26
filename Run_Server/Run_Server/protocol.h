#pragma once

#include <iostream>
#include <WS2tcpip.h>
#pragma comment(lib, "ws2_32") // ws2_32.lib 링크

#define SERVERPORT 7777
#define BUFSIZE    8192

constexpr char SC_LOGIN = 1;
constexpr char SC_LOGOUT = 2;
constexpr char SC_READY = 3;
constexpr char SC_MAP_DATA = 4;
constexpr char SC_GAME_START = 5;
constexpr char SC_POSITION = 6;
constexpr char SC_GAME_END = 7;

constexpr char CS_READY = 1;
constexpr char CS_MAP_OK = 2;
constexpr char CS_KEY_EVENT = 3;

enum class MY_KEY_EVENT : char
{
	KEY_SPACE = 0,
	KEY_LEFT = 1,
	KEY_RIGHT = 2
};

#pragma pack(push, 1)

struct PlayerData
{
	float x;
	float y;
	float z;
	bool is_walk;
	int map_index;
	int bottom_index;
	float now_angle;
};

// 클라가 접속시 해당 클라에게 id 할당 (한명에게 보낸다)
struct SC_LOGIN_PACKET
{
	short size;
	char type;
	char playerid;
};

// 클라가 접속 종료시, 남은 클라에게 해당 클라의 접속종료를 알림 (n명에게 보낸다)
struct SC_LOGOUT_PACKET
{
	short size;
	char type;
	char playerid;
};

struct SC_READY_PACKET
{
	short size;
	char type;
	char playerid;
	bool ready;
};

struct SC_MAP_DATA_PACKET
{
	short size;
	char type;
	float map[100][16];		//map은 100줄 고정....
};

struct SC_GAME_START_PACKET
{
	short size;
	char type;
};

struct SC_POSITION_PACKET
{
	short size;
	char type;
	PlayerData p_info[3];
};

struct SC_GAME_END_PACKET
{
	short size;
	char type;
	float end_time[3];
};

struct CS_READY_PACKET
{
	short size;
	char type;
};

struct CS_MAP_OK_PACKET
{
	short size;
	char type;
};

struct CS_KEY_EVENT_PACKET
{
	short size;
	char type;
	bool is_on;
	MY_KEY_EVENT key;
};

#pragma pack(pop)

// 소켓 함수 오류 출력 후 종료
void err_quit(const char* msg);

// 소켓 함수 오류 출력
void err_display(const char* msg);
