#pragma once

#include <iostream>
#include <WS2tcpip.h>
#pragma comment(lib, "ws2_32") // ws2_32.lib ��ũ

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

// Ŭ�� ���ӽ� �ش� Ŭ�󿡰� id �Ҵ� (�Ѹ��� ������)
struct SC_LOGIN_PACKET
{
	short size;
	char type;
	char playerid;
};

// Ŭ�� ���� �����, ���� Ŭ�󿡰� �ش� Ŭ���� �������Ḧ �˸� (n���� ������)
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
	float map[100][16];		//map�� 100�� ����....
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

// ���� �Լ� ���� ��� �� ����
void err_quit(const char* msg);

// ���� �Լ� ���� ���
void err_display(const char* msg);
