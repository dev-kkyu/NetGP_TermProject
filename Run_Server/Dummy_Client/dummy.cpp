#include "../Run_Server/protocol.h"	// ws2tcpip.h	iosteram

#include <array>
#include <thread>
#include <mutex>

char* SERVERIP = (char*)"127.0.0.1";

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

// 전역 변수
float						g_map[100][16];
SOCKET sock;
std::array<bool, 3>			g_is_accept;
std::array<bool, 3>			g_is_ready;
std::array<CPlayer, 3>		g_player;

void send_cs_ready_packet()
{
	CS_READY_PACKET p;
	p.size = sizeof(p);
	p.type = CS_READY;

	int retval = send(sock, reinterpret_cast<char*>(&p), sizeof(p), 0);
	if (retval == SOCKET_ERROR) {
		err_display("send()");
		//break;	// 차후 고민 필요....
	}

}

void send_cs_map_ok_packet()
{
	CS_MAP_OK_PACKET p;
	p.size = sizeof(p);
	p.type = CS_MAP_OK;

	int retval = send(sock, reinterpret_cast<char*>(&p), sizeof(p), 0);
	if (retval == SOCKET_ERROR) {
		err_display("send()");
		//break;	// 차후 고민 필요....
	}
}

void send_cs_key_event_packet(MY_KEY_EVENT key, bool is_on)
{
	CS_KEY_EVENT_PACKET p;
	p.size = sizeof(p);
	p.type = CS_KEY_EVENT;
	p.is_on = is_on;
	p.key = key;

	int retval = send(sock, reinterpret_cast<char*>(&p), sizeof(p), 0);
	if (retval == SOCKET_ERROR) {
		err_display("send()");
		//break;	// 차후 고민 필요....
	}
}

// 패킷 처리하는 함수
void process_packet(char* packet)
{
	switch (packet[2]) {
	case SC_LOGIN: {
		SC_LOGIN_PACKET* p = reinterpret_cast<SC_LOGIN_PACKET*>(packet);
		std::cout << (int)p->playerid << "번으로 할당..." << std::endl;
	}
		break;
	case SC_READY: {
		SC_READY_PACKET* p = reinterpret_cast<SC_READY_PACKET*>(packet);
		std::cout << (int)p->playerid << "번 플레이어 준비 상태 : " << std::boolalpha << p->ready << std::endl;
	}
		break;
	case SC_MAP_DATA: {
		SC_MAP_DATA_PACKET* p = reinterpret_cast<SC_MAP_DATA_PACKET*>(packet);

	}
		break;
	case SC_POSITION: {
		SC_POSITION_PACKET* p = reinterpret_cast<SC_POSITION_PACKET*>(packet);

	}
		break;
	case SC_GAME_END: {
		SC_GAME_END_PACKET* p = reinterpret_cast<SC_GAME_END_PACKET*>(packet);

	}
		break;
	default:
		std::cout << "invalid packet" << std::endl;
		break;
	}
}


int main(int argc, char* argv[])
{
	int retval;

	// 명령행 인수가 있으면 IP 주소로 사용
	if (argc > 1) SERVERIP = argv[1];

	// 윈속 초기화
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 1;

	// 소켓 생성
	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == INVALID_SOCKET) err_quit("socket()");

	// connect()
	sockaddr_in serveraddr;
	memset(&serveraddr, 0, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	inet_pton(AF_INET, SERVERIP, &serveraddr.sin_addr);
	serveraddr.sin_port = htons(SERVERPORT);
	retval = connect(sock, reinterpret_cast<sockaddr*>(&serveraddr), sizeof(serveraddr));
	if (retval == SOCKET_ERROR) err_quit("connect()");

	int remain_size = 0;
	char* remain_data = new char[BUFSIZE] {};
	while (true) {
		char buf[BUFSIZE];
		int retval = recv(sock, buf, BUFSIZE, 0);

		if (retval == SOCKET_ERROR) {
			err_display("recv()");
			break;
		}
		else if (retval == 0) {
			std::cout  << "서버 종료" << std::endl;
			break;
		}
		std::cout << retval << "수신" << std::endl;

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
				process_packet(remain_data);
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
	closesocket(sock);

	// 윈속 종료
	WSACleanup();
	return 0;
}
