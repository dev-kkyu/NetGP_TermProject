#include "protocol.h"	// ws2tcpip.h	iosteram

#include "PlayerManager.h"
#include "Timer.h"

#include <array>
#include <thread>
#include <mutex>
#include <chrono>

//#define COMM_TEST		// ����ִ� ������ �׽�Ʈ

class CRecordTimer
{
	std::chrono::steady_clock::time_point start_time;
	std::chrono::steady_clock::time_point end_time[3];
public:
	CRecordTimer() : start_time{ std::chrono::steady_clock::now() }
	{
		for (auto& time : end_time)
			time = start_time;
	};

	void set_end_now(int client_id)
	{
		end_time[client_id] = std::chrono::steady_clock::now();
	}

	float get_record(int client_id)
	{
		auto record = end_time[client_id] - start_time;
		return record.count() / 1'000'000'000.;
	}
};

// ��������
std::array<SOCKET, 3>			g_client_sockets;
std::array<std::thread, 3>		g_client_threads;
std::mutex						g_mutex;
float							g_map[100][16];
// �÷��̾� ������
std::array<bool, 3>				g_is_accept;
std::array<bool, 3>				g_is_ready;
bool							g_ready_lock;
std::array<bool, 3>				g_is_map_ok;
std::array<bool, 3>				g_is_end;
std::array<CPlayerManager, 3>	g_player;

std::unique_ptr<CRecordTimer>	g_recordTimer;

CTimer							g_gameTimer;


// ���� ����
void Update(float ElapsedTime);
void SendData();


int get_id()
{
	g_mutex.lock();
	for (int i = 0; i < 3; ++i) {
		if (not g_is_accept[i]) {
			g_mutex.unlock();
			return i;
		}
	}
	g_mutex.unlock();
	return -1;
}

// �۽��Լ�
void send_sc_login_packet(char player_id)
{
	// �α��� ��Ŷ ������ �÷��̾��ȣ �Ҵ�� �ش� �÷��̾�� ����

	SC_LOGIN_PACKET p;
	p.size = sizeof(p);
	p.type = SC_LOGIN;
	p.playerid = player_id;

	g_mutex.lock();
	SOCKET sock = g_client_sockets[player_id];
	g_mutex.unlock();

	int retval = send(sock, reinterpret_cast<char*>(&p), sizeof(p), 0);
	if (retval == SOCKET_ERROR) {
		err_display("send()");
		g_mutex.lock();
		g_is_accept[player_id] = false;
		g_mutex.unlock();
		closesocket(sock);
	}
}

void send_sc_logout_packet(char player_id)
{
	SC_LOGOUT_PACKET p;
	p.size = sizeof(p);
	p.type = SC_LOGOUT;
	p.playerid = player_id;

	// ���� ������ ����
	g_mutex.lock();
	std::array<SOCKET, 3>	client_sockets = g_client_sockets;
	std::array<bool, 3>		is_accept = g_is_accept;
	g_mutex.unlock();

	// ��� Ŭ���̾�Ʈ���� ����
	for (int i = 0; i < 3; ++i) {
		if (is_accept[i]) {
			int retval = send(client_sockets[i], reinterpret_cast<char*>(&p), sizeof(p), 0);
			if (retval == SOCKET_ERROR) {
				err_display("send()");
				g_mutex.lock();
				g_is_accept[i] = false;
				g_mutex.unlock();
				send_sc_logout_packet(i);
				closesocket(client_sockets[i]);
			}
		}
	}
}

void send_sc_ready_packet(char player_id)
{
	// ���ڷ� ���� �÷��̾��� ���� ���¸� ��ο��� ����
	SC_READY_PACKET p;
	p.size = sizeof(p);
	p.type = SC_READY;
	p.playerid = player_id;

	// ���� ������ ����
	g_mutex.lock();
	std::array<SOCKET, 3>	client_sockets = g_client_sockets;
	std::array<bool, 3>		is_accept = g_is_accept;
	std::array<bool, 3>		is_ready = g_is_ready;
	g_mutex.unlock();

	p.ready = is_ready[player_id];

	// ��� Ŭ���̾�Ʈ���� ����
	for (int i = 0; i < 3; ++i) {
		if (is_accept[i]) {
			int retval = send(client_sockets[i], reinterpret_cast<char*>(&p), sizeof(p), 0);
			if (retval == SOCKET_ERROR) {
				err_display("send()");
				g_mutex.lock();
				g_is_accept[i] = false;
				g_mutex.unlock();
				send_sc_logout_packet(i);
				closesocket(client_sockets[i]);
			}
		}
	}
}

void send_sc_map_data_packet()
{
	SC_MAP_DATA_PACKET p;
	p.size = sizeof(p);
	p.type = SC_MAP_DATA;
	memcpy(p.map, g_map, sizeof(g_map));

	// ���� ������ ����
	g_mutex.lock();
	std::array<SOCKET, 3>	client_sockets = g_client_sockets;
	std::array<bool, 3>		is_accept = g_is_accept;
	g_mutex.unlock();

	// ��� Ŭ���̾�Ʈ���� ����
	for (int i = 0; i < 3; ++i) {
		if (is_accept[i]) {
			int retval = send(client_sockets[i], reinterpret_cast<char*>(&p), sizeof(p), 0);
			if (retval == SOCKET_ERROR) {
				err_display("send()");
				g_mutex.lock();
				g_is_accept[i] = false;
				g_mutex.unlock();
				send_sc_logout_packet(i);
				closesocket(client_sockets[i]);
			}
		}
	}
}

void send_sc_game_start_packet()
{
	SC_GAME_START_PACKET p;
	p.size = sizeof(p);
	p.type = SC_GAME_START;

	// ���� ������ ����
	g_mutex.lock();
	std::array<SOCKET, 3>	client_sockets = g_client_sockets;
	std::array<bool, 3>		is_accept = g_is_accept;
	g_mutex.unlock();

	// ��� Ŭ���̾�Ʈ���� ����
	for (int i = 0; i < 3; ++i) {
		if (is_accept[i]) {
			int retval = send(client_sockets[i], reinterpret_cast<char*>(&p), sizeof(p), 0);
			if (retval == SOCKET_ERROR) {
				err_display("send()");
				g_mutex.lock();
				g_is_accept[i] = false;
				g_mutex.unlock();
				send_sc_logout_packet(i);
				closesocket(client_sockets[i]);
			}
		}
	}
}

void send_sc_position_packet()
{
	SC_POSITION_PACKET p;
	p.size = sizeof(p);
	p.type = SC_POSITION;

	// ���� ������ ����
	g_mutex.lock();
	std::array<SOCKET, 3>	client_sockets = g_client_sockets;
	std::array<bool, 3>		is_accept = g_is_accept;
	for (int i = 0; i < 3; ++i)
		memcpy(&p.p_info[i], &g_player[i].info, sizeof(PlayerData));
	g_mutex.unlock();

	// ��� Ŭ���̾�Ʈ���� ����
	for (int i = 0; i < 3; ++i) {
		if (is_accept[i]) {
			int retval = send(client_sockets[i], reinterpret_cast<char*>(&p), sizeof(p), 0);
			if (retval == SOCKET_ERROR) {
				err_display("send()");
				g_mutex.lock();
				g_is_accept[i] = false;
				g_mutex.unlock();
				send_sc_logout_packet(i);
				closesocket(client_sockets[i]);
			}
		}
	}
}

void send_sc_game_end_packet()
{
	SC_GAME_END_PACKET p;
	p.size = sizeof(p);
	p.type = SC_GAME_END;
	for (int index = 0; index < 3; ++index)
		p.end_time[index] = g_recordTimer->get_record(index);

	// ���� ������ ����
	g_mutex.lock();
	std::array<SOCKET, 3>	client_sockets = g_client_sockets;
	std::array<bool, 3>		is_accept = g_is_accept;
	g_mutex.unlock();

	// ��� Ŭ���̾�Ʈ���� ����
	for (int i = 0; i < 3; ++i) {
		if (is_accept[i]) {
			int retval = send(client_sockets[i], reinterpret_cast<char*>(&p), sizeof(p), 0);
			if (retval == SOCKET_ERROR) {
				err_display("send()");
				g_mutex.lock();
				g_is_accept[i] = false;
				g_mutex.unlock();
				send_sc_logout_packet(i);
				closesocket(client_sockets[i]);
			}
		}
	}
}

// ��Ŷ ó���ϴ� �Լ�
void process_packet(int my_id, char* packet)
{
	switch (packet[2]) {
	case CS_READY: {
		CS_READY_PACKET* p = reinterpret_cast<CS_READY_PACKET*>(packet);
		std::cout << my_id << ": player ready packet ����" << std::endl;
		g_mutex.lock();
		if (g_ready_lock) {
			g_mutex.unlock();
			break;
		}
		g_is_ready[my_id] = not g_is_ready[my_id];	// �ش��ϴ� �÷��̾��� �غ� ���¸� �ݴ�� �ٲ���
		g_mutex.unlock();
		send_sc_ready_packet(my_id);				// ������ ��� �÷��̾�� ����
	}
		break;
	case CS_MAP_OK: {
		CS_MAP_OK_PACKET* p = reinterpret_cast<CS_MAP_OK_PACKET*>(packet);
		std::cout << my_id << ": player map ok packet ����" << std::endl;
		g_mutex.lock();
		g_is_map_ok[my_id] = true;
		g_mutex.unlock();
	}
		break;
	case CS_KEY_EVENT: {
		CS_KEY_EVENT_PACKET* p = reinterpret_cast<CS_KEY_EVENT_PACKET*>(packet);
		switch (p->key)
		{
		case MY_KEY_EVENT::KEY_SPACE:
			g_mutex.lock();
			g_player[my_id].isSpace = p->is_on;
			g_mutex.unlock();
			break;
		case MY_KEY_EVENT::KEY_LEFT:
			g_mutex.lock();
			g_player[my_id].isLeft = p->is_on;
			g_mutex.unlock();
			break;
		case MY_KEY_EVENT::KEY_RIGHT:
			g_mutex.lock();
			g_player[my_id].isRight = p->is_on;
			g_mutex.unlock();
			break;
		default:
			printf("%d: player key packet: err\n", my_id);
			break;
		}
	}
		break;
	default:
		std::cout << "invalid packet" << std::endl;
		break;
	}
}

// Ŭ���̾�Ʈ ������ �Լ�
void RecvThread(int player_id)
{
	// ù 2����Ʈ�� ������, 3��° ����Ʈ�� Ÿ��
	// Ŭ���̾�Ʈ�� ������ ���
	g_mutex.lock();
	SOCKET sock = g_client_sockets[player_id];
	g_mutex.unlock();

	// Ŭ���̾�Ʈ ���� ���
	struct sockaddr_in clientaddr;
	int addrlen = sizeof(clientaddr);
	char addr[INET_ADDRSTRLEN];
	getpeername(sock, (struct sockaddr*)&clientaddr, &addrlen);
	inet_ntop(AF_INET, &clientaddr.sin_addr, addr, sizeof(addr));

	int remain_size = 0;
	char* remain_data = new char[BUFSIZE] {};
	while (true) {
		char buf[BUFSIZE];
		int retval = recv(sock, buf, BUFSIZE, 0);

		if (retval <= 0) {
			std::cout << player_id << ": Ŭ���̾�Ʈ ����" << std::endl;
			if (retval == SOCKET_ERROR)
				err_display("recv()");
			break;
		}

		int received = retval;
		char* ptr = buf;
		while (received > 0)
		{
			if (received < 2) {
				memcpy(remain_data + remain_size, ptr, received);
				remain_size += received;
				received = 0;
				if (remain_size < 2)
					break;
			}
			int packet_size = reinterpret_cast<short*>(ptr)[0];
			if (remain_size > 1)
				packet_size = reinterpret_cast<short*>(remain_data)[0];
			if (remain_size + received >= packet_size) {
				memcpy(remain_data + remain_size, ptr, packet_size - remain_size);
				ptr += packet_size - remain_size;
				received -= packet_size - remain_size;
				process_packet(player_id, remain_data);
				remain_size = 0;
			}
			else {
				memcpy(remain_data + remain_size, ptr, received);
				remain_size += received;
				received = 0;
			}
		}
	}
	delete[] remain_data;

	// ���� �ݱ�
	g_mutex.lock();
	g_is_accept[player_id] = false;
	if (not g_ready_lock)
		g_client_threads[player_id].detach();	// ���� ���� ���ᰡ �Ǹ� detach ���ش�.
	g_mutex.unlock();
	send_sc_logout_packet(player_id);
	printf("[TCP ����] Ŭ���̾�Ʈ ����: IP �ּ�=%s, ��Ʈ ��ȣ=%d\n",
		addr, ntohs(clientaddr.sin_port));
	closesocket(sock);
}



void game_loop()
{
	while (true) {				// ready ��ٸ���.
		bool is_start = true;
		for (int i = 0; i < 3; ++i) {
			std::lock_guard<std::mutex> l{ g_mutex };
			if (g_is_ready[i] == false) {
				is_start = false;
				break;
			}
		}
		if (is_start) {
			std::lock_guard<std::mutex> l{ g_mutex };
			g_ready_lock = true;
			break;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}

	send_sc_map_data_packet();	// map_data ������.
	while (true) {				// map_ok ��ٸ���.
		bool is_start = true;
		for (int i = 0; i < 3; ++i) {
			std::lock_guard<std::mutex> l{ g_mutex };
			if (g_is_map_ok[i] == false) {
				is_start = false;
				break;
			}
		}
		if (is_start) {
			std::lock_guard<std::mutex> l{ g_mutex };
			g_is_map_ok = std::array<bool, 3>{};	// ���������� ���� �ʱ�ȭ
			break;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}

	std::cout << "3�� �� ����" << std::endl;
	std::this_thread::sleep_for(std::chrono::seconds(3));
	send_sc_game_start_packet();
	g_recordTimer = std::make_unique<CRecordTimer>();		// ��� ����

	g_gameTimer.Tick(0);
	while (true) {			// ���� ����
		float elapsedTime = g_gameTimer.Tick(60);	// �ʴ� 60��

		Update(elapsedTime);
		SendData();

		std::lock_guard<std::mutex> l{ g_mutex };
		if (g_is_end[0] and g_is_end[1] and g_is_end[2])
			break;
	}
	send_sc_game_end_packet();
}

void Update(float ElapsedTime)
{
	std::lock_guard<std::mutex> l{ g_mutex };
	for (int i = 0; i < 3; ++i) {
		g_player[i].Update(ElapsedTime);
		if (not g_is_end[i]) {
			if (g_player[i].info.map_index >= 100 - 1) {
				g_is_end[i] = true;
				if (g_recordTimer)
					g_recordTimer->set_end_now(i);
				else
					std::cerr << "RecordTimer alloc Error" << std::endl;
			}
		}
	}
}

void SendData()
{
	send_sc_position_packet();
}

int main(int argc, char *argv[])
{
	int retval;

	// ���� �ʱ�ȭ
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 1;

	// ���� ����
	SOCKET listen_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (listen_sock == INVALID_SOCKET) err_quit("socket()");

	// bind()
	sockaddr_in serveraddr;
	memset(&serveraddr, 0, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons(SERVERPORT);
	retval = bind(listen_sock, reinterpret_cast<sockaddr*>(&serveraddr), sizeof(serveraddr));
	if (retval == SOCKET_ERROR) err_quit("bind()");

	// listen()
	retval = listen(listen_sock, SOMAXCONN);
	if (retval == SOCKET_ERROR) err_quit("listen()");


	// ������ ��ſ� ����� ����

	bool end_flag = false;
	while (not end_flag) {
		{
			// �� ���۽� �� ����
#ifndef COMM_TEST
			std::vector<MapRect> map_data = MapRect::make_map();
			for (int i = 0; i < 3; ++i)
				g_player[i].SetMap(map_data);
			memcpy(&g_map, map_data.data(), map_data.size() * sizeof(MapRect));
#else
			std::vector<MapRect> map_data;
			for (int i = 0; i < 100; ++i)
				map_data.push_back(MapRect{ 1.f });
			for (int i = 0; i < 3; ++i)
				g_player[i].SetMap(map_data);
			memcpy(&g_map, map_data.data(), map_data.size() * sizeof(MapRect));
#endif
		}
		while (get_id() >= 0) {		// id�� �� ������ ���� accept
			// accept()
			sockaddr_in clientaddr;
			int addrlen = sizeof(clientaddr);
			SOCKET client_sock = accept(listen_sock, reinterpret_cast<sockaddr*>(&clientaddr), &addrlen);
			if (client_sock == INVALID_SOCKET) {
				err_display("accept()");
				end_flag = true;
				break;
			}
			int id = get_id();

			// ������ Ŭ���̾�Ʈ ���� ���
			char addr[INET_ADDRSTRLEN];
			inet_ntop(AF_INET, &clientaddr.sin_addr, addr, sizeof(addr));
			printf("\n[TCP ����] Ŭ���̾�Ʈ ����: IP �ּ�=%s, ��Ʈ ��ȣ=%d\n",
				addr, ntohs(clientaddr.sin_port));


			{	// �α��� ó��
				g_mutex.lock();
				g_is_accept[id] = true;							// ���������� ����
				g_is_ready[id] = false;
				g_client_sockets[id] = client_sock;				// ''
				g_mutex.unlock();

				send_sc_login_packet(id);	// id �Ҵ�
				send_sc_ready_packet(id);	// �ٸ� ���ӵ� ��� �÷��̾�鿡�� ���� �غ����(false)�� �˸����ν�, ���� ���縦 �˸���.

				g_mutex.lock();
				if (g_is_accept[id] == false) {		// send_sc_login_packet���� �÷��̾ ������������ �ʱ�ȭ
					g_mutex.unlock();
					send_sc_logout_packet(id);
					continue;
				}
				g_mutex.unlock();
				
				// ������ ���� ���ӵ� ��� Ŭ���̾�Ʈ�� ���¸� ���� �����ϰ� ����
				// ������ ���� ��� �����߰�, ������ �ٸ� Ŭ���̾�Ʈ�� �غ���¸� �˸����ν� �ٸ� Ŭ���̾�Ʈ�� �������� �˸���.
				for (int j = 0; j < 3; ++j) {
					if (id == j)
						continue;
					g_mutex.lock();
					if (g_is_accept[j]) {
						SC_READY_PACKET p;
						p.ready = g_is_ready[j];
						g_mutex.unlock();

						p.playerid = j;
						p.size = sizeof(p);
						p.type = SC_READY;

						int retval = send(client_sock, reinterpret_cast<char*>(&p), sizeof(p), 0);
						if (retval == SOCKET_ERROR) {
							err_display("send()");
							g_mutex.lock();
							g_is_accept[id] = false;		// ���� ����
							g_mutex.unlock();
							send_sc_logout_packet(id);
							closesocket(client_sock);
							break;
						}
					}
					else g_mutex.unlock();
				}
				g_mutex.lock();
				if (g_is_accept[id] == true)
					g_client_threads[id] = std::thread{ RecvThread, id };
				g_mutex.unlock();
			}
		}

		if (not end_flag)
			game_loop();

		// ���� ���� ��, Ŭ���̾�Ʈ�� ���Ḧ ��ٸ�
		for (auto& t : g_client_threads)
			t.join();

		// ���� ������ �ʱ�ȭ
		g_mutex.lock();
		g_is_accept[0] = g_is_accept[1] = g_is_accept[2] = false;
		g_is_ready[0] = g_is_ready[1] = g_is_ready[2] = false;
		g_is_ready[0] = g_is_ready[1] = g_is_ready[2] = false;
		g_ready_lock = false;
		g_is_map_ok[0] = g_is_map_ok[1] = g_is_map_ok[2] = false;
		g_is_end[0] = g_is_end[1] = g_is_end[2] = false;
		g_player[0] = g_player[1] = g_player[2] = CPlayerManager{};
		g_recordTimer.reset();
		g_mutex.unlock();
	}

	// ���� �ݱ�
	closesocket(listen_sock);

	// ���� ����
	WSACleanup();
	return 0;
}
