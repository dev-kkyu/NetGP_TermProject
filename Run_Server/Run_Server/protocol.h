#pragma once

#include <iostream>
#include <WS2tcpip.h>
#pragma comment(lib, "ws2_32") // ws2_32.lib ��ũ

#define SERVERPORT 7777
#define BUFSIZE    8192

constexpr char SC_LOGIN = 1;
constexpr char SC_READY = 2;
constexpr char SC_MAP_DATA = 3;
constexpr char SC_POSITION = 4;
constexpr char SC_GAME_END = 5;

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
	int map_index;
	int bottom_index;
	bool is_rotating;
	float now_angle;
	float bef_mv_x;
	float bef_mv_y;
};

struct SC_LOGIN_PACKET
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
	float map[100][16];		//map�� 100�� ����....
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


// ���� �Լ� ���� ��� �� ����
void err_quit(const char* msg)
{
	LPVOID lpMsgBuf;
	FormatMessageA(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(char*)&lpMsgBuf, 0, NULL);
	MessageBoxA(NULL, (const char*)lpMsgBuf, msg, MB_ICONERROR);
	LocalFree(lpMsgBuf);
	exit(1);
}

// ���� �Լ� ���� ���
void err_display(const char* msg)
{
	LPVOID lpMsgBuf;
	FormatMessageA(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(char*)&lpMsgBuf, 0, NULL);
	printf("[%s] %s\n", msg, (char*)lpMsgBuf);
	LocalFree(lpMsgBuf);
}