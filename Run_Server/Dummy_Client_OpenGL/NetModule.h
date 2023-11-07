#pragma once
#include "../Run_Server/protocol.h"
#include <array>
#include <mutex>

class CPlayer
{
public:
	PlayerData info;

public:
	CPlayer() : info{}
	{
	}
	~CPlayer() {}
};

class CNetModule
{
public:
	SOCKET	m_sock;
	char	my_id;

	// ¸â¹ö º¯¼ö
	float						m_map[100][16];
	std::array<bool, 3>			m_is_accept;
	std::array<bool, 3>			m_is_ready;
	std::array<CPlayer, 3>		m_player;

	void send_cs_ready_packet();

public:
	CNetModule();
	~CNetModule();

	static void process_packet(char* packet, std::mutex& m, std::unique_ptr<CNetModule>& my_Net);
	static void RecvThread(SOCKET s, std::mutex& m, std::unique_ptr<CNetModule>& my_Net);
};

