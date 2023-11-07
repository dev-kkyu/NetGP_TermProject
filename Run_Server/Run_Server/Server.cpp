#include "protocol.h"	// ws2tcpip.h	iosteram

#include <array>
#include <thread>
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


// 전역변수
std::array<SOCKET, 3>		g_client_sockets;
std::array<std::thread, 3>	g_client_threads;
std::mutex					g_mutex;
float						g_map[100][16];
std::array<bool, 3>			g_is_accept;
std::array<bool, 3>			g_is_ready;
std::array<CPlayer, 3>		g_player;

// 송신함수
void send_sc_ready_packet(char player_id)
{
	// 현재 해당 플레이어 레디상태 체크 후 반대로 바꿔서 다시 전송(모든 클라이언트에게)
	SC_READY_PACKET p;
	p.size = sizeof(p);
	p.type = SC_READY;
	p.playerid = player_id;

	for (int i = 0; i < 3; ++i) {
		if (g_is_accept[i]) {
			p.ready = g_is_ready[player_id];

			int retval = send(g_client_sockets[i], reinterpret_cast<char*>(&p), sizeof(p), 0);
			if (retval == SOCKET_ERROR) {
				err_display("send()");
				//break;	// 차후 고민 필요....
			}
		}
	}
}

void send_sc_login_packet(char player_id)
{
	// 로그인 패킷 생성후 플레이어번호 할당및 해당 플레이어에게 전송
	{
		SC_LOGIN_PACKET p;
		p.size = sizeof(p);
		p.type = SC_LOGIN;
		p.playerid = player_id;

		int retval = send(g_client_sockets[player_id], reinterpret_cast<char*>(&p), sizeof(p), 0);
		if (retval == SOCKET_ERROR) {
			err_display("send()");
			//break;	// 차후 고민 필요....
		}
	}


	// 나를 제외한 다른 클라이언트에게 나의 준비상태를 알림으로써 나의 로그인을 알림
	for (int j = 0; j < 3; ++j) {
		if (j == player_id)
			continue;
		if (g_is_accept[j]) {
			SC_READY_PACKET p;
			p.size = sizeof(p);
			p.type = SC_READY;
			p.playerid = player_id;
			p.ready = g_is_ready[player_id];

			int retval = send(g_client_sockets[j], reinterpret_cast<char*>(&p), sizeof(p), 0);
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
		std::cout << "레디패킷 수신" << std::endl;
		CS_READY_PACKET* p = reinterpret_cast<CS_READY_PACKET*>(packet);
		g_is_ready[my_id] = not g_is_ready[my_id];
		for (int i = 0; i < 3; ++i)
			if (g_is_accept[i])
				send_sc_ready_packet(my_id);
	}
		break;
	case CS_MAP_OK: {
		CS_MAP_OK_PACKET* p = reinterpret_cast<CS_MAP_OK_PACKET*>(packet);

	}
		break;
	case CS_KEY_EVENT: {
		CS_KEY_EVENT_PACKET* p = reinterpret_cast<CS_KEY_EVENT_PACKET*>(packet);

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



			{	// 보호된 공간	// 로그인 처리
				std::lock_guard<std::mutex> l{ g_mutex };
				g_is_accept[i] = true;
				g_client_sockets[i] = client_sock;
				send_sc_login_packet(i);	// id 할당
				// 접속된 모든 클라이언트의 상태를 전송
				for (int j = 0; j < 3; ++j) {
					if (g_is_accept[j]) {
						SC_READY_PACKET p;
						p.size = sizeof(p);
						p.type = SC_READY;
						p.playerid = j;
						p.ready = g_is_ready[j];

						int retval = send(client_sock, reinterpret_cast<char*>(&p), sizeof(p), 0);
						if (retval == SOCKET_ERROR) {
							err_display("send()");
							//break;	// 차후 고민 필요....
						}
					}
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
