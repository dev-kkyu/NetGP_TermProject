#include "protocol.h"	// ws2tcpip.h	iosteram

#include <array>
#include <thread>
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


// 전역변수
std::array<SOCKET, 3>			g_client_sockets;
std::array<std::thread, 3>		g_client_threads;
std::mutex						g_mutex;
float							g_map[100][16];
std::array<bool, 3>				g_is_accept;
std::array<bool, 3>				g_is_ready;
std::array<CPlayerManager, 3>	g_player;

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
		//break;	// 차후 고민 필요....
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
				//break;	// 차후 고민 필요....
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
				//break;	// 차후 고민 필요....
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
				//break;	// 차후 고민 필요....
			}
		}
	}
}

// 패킷 처리하는 함수
void process_packet(int my_id, char* packet)
{
	switch (packet[2]) {
	case CS_READY: {
		std::cout << my_id << ": player ready packet 수신" << std::endl;
		CS_READY_PACKET* p = reinterpret_cast<CS_READY_PACKET*>(packet);
		g_mutex.lock();
		g_is_ready[my_id] = not g_is_ready[my_id];	// 해당하는 플레이어의 준비 상태를 반대로 바꿔줌
		g_mutex.unlock();
		send_sc_ready_packet(my_id);				// 접속한 모든 플레이어에게 전송
	}
		break;
	case CS_MAP_OK: {
		CS_MAP_OK_PACKET* p = reinterpret_cast<CS_MAP_OK_PACKET*>(packet);

	}
		break;
	case CS_KEY_EVENT: {
		CS_KEY_EVENT_PACKET* p = reinterpret_cast<CS_KEY_EVENT_PACKET*>(packet);
		switch (p->key)
		{
		case MY_KEY_EVENT::KEY_SPACE:
			printf("%d: player key packet: space", my_id);
			break;
		case MY_KEY_EVENT::KEY_LEFT:
			printf("%d: player key packet: left", my_id);
			break;
		case MY_KEY_EVENT::KEY_RIGHT:
			printf("%d: player key packet: right", my_id);
			break;
		default:
			printf("%d: player key packet: err", my_id);
			break;
		}
		if (p->is_on)
			printf(" 눌림\n");
		else
			printf(" 떼짐\n");
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
	int remain_size = 0;
	char* remain_data = new char[BUFSIZE] {};
	while (true) {
		char buf[BUFSIZE];
		int retval = recv(g_client_sockets[player_id], buf, BUFSIZE, 0);

		if (retval == SOCKET_ERROR) {
			err_display("recv()");
			break;
		}
		else if (retval == 0) {
			std::cout << player_id << ": 클라이언트 종료" << std::endl;
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
	//char addr[INET_ADDRSTRLEN];
	//inet_ntop(AF_INET, &sock_data.addr.sin_addr, addr, sizeof(addr));
	//printf("[TCP 서버] 클라이언트 종료: IP 주소=%s, 포트 번호=%d\n",
	//	addr, ntohs(clientaddr.sin_port));
	closesocket(g_client_sockets[player_id]);
}



void game_loop()
{
	while (true) {
		std::cout << "실행중..." << std::endl;
		std::this_thread::sleep_for(std::chrono::milliseconds{ 1000 });
	}
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

		for (int i = 0; i < 3; ++i) {
			// accept()
			sockaddr_in clientaddr;
			int addrlen = sizeof(clientaddr);
			SOCKET client_sock = accept(listen_sock, reinterpret_cast<sockaddr*>(&clientaddr), &addrlen);
			if (client_sock == INVALID_SOCKET) {
				err_display("accept()");
				end_flag = true;
				break;
			}

			// 접속한 클라이언트 정보 출력
			char addr[INET_ADDRSTRLEN];
			inet_ntop(AF_INET, &clientaddr.sin_addr, addr, sizeof(addr));
			printf("\n[TCP 서버] 클라이언트 접속: IP 주소=%s, 포트 번호=%d\n",
				addr, ntohs(clientaddr.sin_port));


			{	// 로그인 처리
				g_mutex.lock();
				g_is_accept[i] = true;							// 전역데이터 변경
				g_client_sockets[i] = client_sock;				// ''
				g_mutex.unlock();

				send_sc_login_packet(i);	// id 할당
				send_sc_ready_packet(i);	// 다른 접속된 모든 플레이어들에게 나의 준비상태(false)를 알림으로써, 나의 존재를 알린다.
				
				// 나에게 현재 접속된 모든 클라이언트의 상태를 나를 제외하고 전송
				// 나에게 나는 방금 전송했고, 나에게 다른 클라이언트의 준비상태를 알림으로써 다른 클라이언트가 존재함을 알린다.
				for (int j = 0; j < 3; ++j) {
					if (i == j)
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
							//break;	// 차후 고민 필요....
						}
					}
					else g_mutex.unlock();
				}
				g_client_threads[i] = std::thread{ RecvThread, i };
			}
		}

		game_loop();

		// 게임 종료 시, 클라이언트의 종료를 기다림
		for (auto& t : g_client_threads)
			t.join();
	}

	// 소켓 닫기
	closesocket(listen_sock);

	// 윈속 종료
	WSACleanup();
	return 0;
}
