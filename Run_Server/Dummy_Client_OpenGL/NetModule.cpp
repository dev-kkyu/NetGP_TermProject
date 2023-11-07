#include "NetModule.h"

char* SERVERIP = (char*)"127.0.0.1";

void CNetModule::send_cs_ready_packet()
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

CNetModule::CNetModule() : g_map{}, g_is_accept{}, g_is_ready{}, g_player{}, sock{}, my_id{-1}
{
	int retval;

	// 윈속 초기화
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
		printf("윈속 초기화 오류\n");
		exit(EXIT_FAILURE);
	}

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
}

CNetModule::~CNetModule()
{
	// 소켓 닫기
	closesocket(sock);

	// 윈속 종료
	WSACleanup();
}

void CNetModule::process_packet(char* packet, std::mutex& m, std::unique_ptr<CNetModule>& my_Net)
{
	switch (packet[2]) {
	case SC_LOGIN: {
		SC_LOGIN_PACKET* p = reinterpret_cast<SC_LOGIN_PACKET*>(packet);
		std::cout << (int)p->playerid << "번으로 할당..." << std::endl;
		m.lock();
		my_Net->my_id = p->playerid;
		m.unlock();
	}
				 break;
	case SC_READY: {
		SC_READY_PACKET* p = reinterpret_cast<SC_READY_PACKET*>(packet);
		std::cout << (int)p->playerid << "번 플레이어 준비 상태 : " << std::boolalpha << p->ready << std::endl;
		m.lock();
		my_Net->g_is_accept[p->playerid] = true;
		my_Net->g_is_ready[p->playerid] = p->ready;
		m.unlock();
		for(int i=0;i<3;++i)
			if (my_Net->g_is_accept[i])
				std::cout << i << " : " << std::boolalpha << my_Net->g_is_ready[i] << std::endl;
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

void CNetModule::RecvThread(SOCKET s, std::mutex& m, std::unique_ptr<CNetModule>& my_Net)
{
	int remain_size = 0;
	char* remain_data = new char[BUFSIZE] {};
	while (true) {
		char buf[BUFSIZE];
		int retval = recv(s, buf, BUFSIZE, 0);

		if (retval == SOCKET_ERROR) {
			err_display("recv()");
			break;
		}
		else if (retval == 0) {
			std::cout << "서버 종료" << std::endl;
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
				process_packet(remain_data, m, my_Net);
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
}
