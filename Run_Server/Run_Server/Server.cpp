#include "protocol.h"	// ws2tcpip.h	iosteram

#include "PlayerManager.h"
#include "Timer.h"

#include <array>
#include <thread>
#include <mutex>
#include <chrono>

//#define COMM_TEST		// 비어있는 맵으로 테스트

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

// 전역변수
std::array<SOCKET, 3>			g_client_sockets;
std::array<std::thread, 3>		g_client_threads;
std::mutex						g_mutex;
float							g_map[100][16];
// 플레이어 데이터
std::array<bool, 3>				g_is_accept;
std::array<bool, 3>				g_is_ready;
bool							g_ready_lock;
std::array<bool, 3>				g_is_map_ok;
std::array<bool, 3>				g_is_end;
std::array<CPlayerManager, 3>	g_player;

std::unique_ptr<CRecordTimer>	g_recordTimer;

CTimer							g_gameTimer;


// 게임 루프
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

// 송신함수
void send_sc_login_packet(char player_id)
{
	// 로그인 패킷 생성후 플레이어번호 할당및 해당 플레이어에게 전송

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

	// 전역 데이터 복사
	g_mutex.lock();
	std::array<SOCKET, 3>	client_sockets = g_client_sockets;
	std::array<bool, 3>		is_accept = g_is_accept;
	g_mutex.unlock();

	// 모든 클라이언트에게 전송
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
	// 인자로 받은 플레이어의 레디 상태를 모두에게 전송
	SC_READY_PACKET p;
	p.size = sizeof(p);
	p.type = SC_READY;
	p.playerid = player_id;

	// 전역 데이터 복사
	g_mutex.lock();
	std::array<SOCKET, 3>	client_sockets = g_client_sockets;
	std::array<bool, 3>		is_accept = g_is_accept;
	std::array<bool, 3>		is_ready = g_is_ready;
	g_mutex.unlock();

	p.ready = is_ready[player_id];

	// 모든 클라이언트에게 전송
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

	// 전역 데이터 복사
	g_mutex.lock();
	std::array<SOCKET, 3>	client_sockets = g_client_sockets;
	std::array<bool, 3>		is_accept = g_is_accept;
	g_mutex.unlock();

	// 모든 클라이언트에게 전송
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

	// 전역 데이터 복사
	g_mutex.lock();
	std::array<SOCKET, 3>	client_sockets = g_client_sockets;
	std::array<bool, 3>		is_accept = g_is_accept;
	g_mutex.unlock();

	// 모든 클라이언트에게 전송
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

	// 전역 데이터 복사
	g_mutex.lock();
	std::array<SOCKET, 3>	client_sockets = g_client_sockets;
	std::array<bool, 3>		is_accept = g_is_accept;
	for (int i = 0; i < 3; ++i)
		memcpy(&p.p_info[i], &g_player[i].info, sizeof(PlayerData));
	g_mutex.unlock();

	// 모든 클라이언트에게 전송
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

	// 전역 데이터 복사
	g_mutex.lock();
	std::array<SOCKET, 3>	client_sockets = g_client_sockets;
	std::array<bool, 3>		is_accept = g_is_accept;
	g_mutex.unlock();

	// 모든 클라이언트에게 전송
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

// 패킷 처리하는 함수
void process_packet(int my_id, char* packet)
{
	switch (packet[2]) {
	case CS_READY: {
		CS_READY_PACKET* p = reinterpret_cast<CS_READY_PACKET*>(packet);
		std::cout << my_id << ": player ready packet 수신" << std::endl;
		g_mutex.lock();
		if (g_ready_lock) {
			g_mutex.unlock();
			break;
		}
		g_is_ready[my_id] = not g_is_ready[my_id];	// 해당하는 플레이어의 준비 상태를 반대로 바꿔줌
		g_mutex.unlock();
		send_sc_ready_packet(my_id);				// 접속한 모든 플레이어에게 전송
	}
		break;
	case CS_MAP_OK: {
		CS_MAP_OK_PACKET* p = reinterpret_cast<CS_MAP_OK_PACKET*>(packet);
		std::cout << my_id << ": player map ok packet 수신" << std::endl;
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

// 클라이언트 스레드 함수
void RecvThread(int player_id)
{
	// 첫 2바이트가 사이즈, 3번째 바이트가 타입
	// 클라이언트와 데이터 통신
	g_mutex.lock();
	SOCKET sock = g_client_sockets[player_id];
	g_mutex.unlock();

	// 클라이언트 정보 얻기
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
			std::cout << player_id << ": 클라이언트 종료" << std::endl;
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

	// 소켓 닫기
	g_mutex.lock();
	g_is_accept[player_id] = false;
	if (not g_ready_lock)
		g_client_threads[player_id].detach();	// 시작 전에 종료가 되면 detach 해준다.
	g_mutex.unlock();
	send_sc_logout_packet(player_id);
	printf("[TCP 서버] 클라이언트 종료: IP 주소=%s, 포트 번호=%d\n",
		addr, ntohs(clientaddr.sin_port));
	closesocket(sock);
}



void game_loop()
{
	while (true) {				// ready 기다린다.
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

	send_sc_map_data_packet();	// map_data 보낸다.
	while (true) {				// map_ok 기다린다.
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
			g_is_map_ok = std::array<bool, 3>{};	// 다음게임을 위한 초기화
			break;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}

	std::cout << "3초 뒤 시작" << std::endl;
	std::this_thread::sleep_for(std::chrono::seconds(3));
	send_sc_game_start_packet();
	g_recordTimer = std::make_unique<CRecordTimer>();		// 기록 시작

	g_gameTimer.Tick(0);
	while (true) {			// 게임 진행
		float elapsedTime = g_gameTimer.Tick(60);	// 초당 60번

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

	// 윈속 초기화
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 1;

	// 소켓 생성
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


	// 데이터 통신에 사용할 변수

	bool end_flag = false;
	while (not end_flag) {
		{
			// 판 시작시 맵 생성
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
		while (get_id() >= 0) {		// id가 안 남을때 까지 accept
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

			// 접속한 클라이언트 정보 출력
			char addr[INET_ADDRSTRLEN];
			inet_ntop(AF_INET, &clientaddr.sin_addr, addr, sizeof(addr));
			printf("\n[TCP 서버] 클라이언트 접속: IP 주소=%s, 포트 번호=%d\n",
				addr, ntohs(clientaddr.sin_port));


			{	// 로그인 처리
				g_mutex.lock();
				g_is_accept[id] = true;							// 전역데이터 변경
				g_is_ready[id] = false;
				g_client_sockets[id] = client_sock;				// ''
				g_mutex.unlock();

				send_sc_login_packet(id);	// id 할당
				send_sc_ready_packet(id);	// 다른 접속된 모든 플레이어들에게 나의 준비상태(false)를 알림으로써, 나의 존재를 알린다.

				g_mutex.lock();
				if (g_is_accept[id] == false) {		// send_sc_login_packet에서 플레이어가 나가버렸으면 초기화
					g_mutex.unlock();
					send_sc_logout_packet(id);
					continue;
				}
				g_mutex.unlock();
				
				// 나에게 현재 접속된 모든 클라이언트의 상태를 나를 제외하고 전송
				// 나에게 나는 방금 전송했고, 나에게 다른 클라이언트의 준비상태를 알림으로써 다른 클라이언트가 존재함을 알린다.
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
							g_is_accept[id] = false;		// 내가 나감
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

		// 게임 종료 시, 클라이언트의 종료를 기다림
		for (auto& t : g_client_threads)
			t.join();

		// 전역 데이터 초기화
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

	// 소켓 닫기
	closesocket(listen_sock);

	// 윈속 종료
	WSACleanup();
	return 0;
}
