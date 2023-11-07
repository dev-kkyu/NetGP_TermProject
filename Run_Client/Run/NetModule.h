#pragma once
#include "stdafx.h"
#include <array>
#include <mutex>

class CPlayerManager
{
public:
	PlayerData info;

public:
	CPlayerManager() : info{}
	{

	}
	~CPlayerManager() {}
};

class CNetModule
{
public:
	SOCKET							m_sock;
	char							my_id;

	std::mutex&						m_mutex;

	// ¸â¹ö º¯¼ö
	float							m_map[100][16];
	std::array<bool, 3>				m_is_accept;
	std::array<bool, 3>				m_is_ready;
	std::array<CPlayerManager, 3>	m_player;

public:
	CNetModule(std::mutex& mutex);
	~CNetModule();

	void send_cs_ready_packet();
	void send_cs_map_ok_packet();
	void send_cs_key_event_packet(MY_KEY_EVENT key, bool is_on);

public:
	static void process_packet(char* packet, std::mutex& m, std::unique_ptr<CNetModule>& my_Net);
	static void RecvThread(SOCKET s, std::mutex& m, std::unique_ptr<CNetModule>& my_Net);
};

