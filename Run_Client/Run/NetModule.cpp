#include "NetModule.h"
#include "Scene.h"

void CNetModule::send_cs_ready_packet()
{
	CS_READY_PACKET p;
	p.size = sizeof(p);
	p.type = CS_READY;

	int retval = send(m_sock, reinterpret_cast<char*>(&p), sizeof(p), 0);
	if (retval == SOCKET_ERROR) {
		err_display("send()");
		//break;	// 차후 고민 필요....
	}
}

void CNetModule::send_cs_map_ok_packet()
{
	CS_MAP_OK_PACKET p;
	p.size = sizeof(p);
	p.type = CS_MAP_OK;

	int retval = send(m_sock, reinterpret_cast<char*>(&p), sizeof(p), 0);
	if (retval == SOCKET_ERROR) {
		err_display("send()");
		//break;	// 차후 고민 필요....
	}
}

void CNetModule::send_cs_key_event_packet(MY_KEY_EVENT key, bool is_on)
{
	CS_KEY_EVENT_PACKET p;
	p.size = sizeof(p);
	p.type = CS_KEY_EVENT;
	p.is_on = is_on;
	p.key = key;

	int retval = send(m_sock, reinterpret_cast<char*>(&p), sizeof(p), 0);
	if (retval == SOCKET_ERROR) {
		err_display("send()");
		//break;	// 차후 고민 필요....
	}
}

CNetModule::CNetModule(std::mutex& mutex, char* SERVERIP) : m_is_accept{}, m_is_ready{}, m_player{}, m_sock{}, my_id{ -1 }, m_mutex{ mutex }
{
	if (not SERVERIP)
		SERVERIP = const_cast<char*>("127.0.0.1");

	int retval;

	// 윈속 초기화
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
		printf("윈속 초기화 오류\n");
		exit(EXIT_FAILURE);
	}

	// 소켓 생성
	m_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (m_sock == INVALID_SOCKET) err_quit("socket()");

	// connect()
	sockaddr_in serveraddr;
	memset(&serveraddr, 0, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	inet_pton(AF_INET, SERVERIP, &serveraddr.sin_addr);
	serveraddr.sin_port = htons(SERVERPORT);
	retval = connect(m_sock, reinterpret_cast<sockaddr*>(&serveraddr), sizeof(serveraddr));
	if (retval == SOCKET_ERROR) err_quit("connect()");
}

CNetModule::~CNetModule()
{
	// 소켓 닫기
	closesocket(m_sock);

	// 윈속 종료
	WSACleanup();
}

void CNetModule::SetScene(std::shared_ptr<CScene> pscene)
{
	m_pscene = pscene;
}

void CNetModule::process_packet(char* packet, std::mutex& m, std::shared_ptr<CNetModule> my_Net)
{
	switch (packet[2]) {
	case SC_LOGIN: {
		SC_LOGIN_PACKET* p = reinterpret_cast<SC_LOGIN_PACKET*>(packet);
		std::cout << (int)p->playerid << "번으로 할당..." << std::endl;
		m.lock();
		my_Net->my_id = p->playerid;
		if (my_Net->m_pscene)
			my_Net->m_pscene->SetID(p->playerid);
		m.unlock();
	}
				 break;
	case SC_LOGOUT: {
		SC_LOGOUT_PACKET* p = reinterpret_cast<SC_LOGOUT_PACKET*>(packet);
		m.lock();
		my_Net->m_is_accept[p->playerid] = false;
		if (my_Net->m_pscene)
			my_Net->m_pscene->SetIsReady(p->playerid, false);
		m.unlock();
		std::cout << "로그아웃 패킷 수신 - " << (int)p->playerid << "번 플레이어" << std::endl;
	}
		break;
	case SC_READY: {
		SC_READY_PACKET* p = reinterpret_cast<SC_READY_PACKET*>(packet);
		std::cout << (int)p->playerid << "번 플레이어 준비 상태 : " << std::boolalpha << p->ready << ", 나는 : " << (int)my_Net->my_id << std::endl;
		m.lock();
		my_Net->m_is_accept[p->playerid] = true;
		my_Net->m_is_ready[p->playerid] = p->ready;
		if (my_Net->m_pscene)
			my_Net->m_pscene->SetIsReady(p->playerid, p->ready);
		m.unlock();
	}
				 break;
	case SC_MAP_DATA: {
		SC_MAP_DATA_PACKET* p = reinterpret_cast<SC_MAP_DATA_PACKET*>(packet);

		std::cout << "맵 패킷 수신" << std::endl;
		m.lock();
		if (my_Net->m_pscene)
			my_Net->m_pscene->SetMap(p->map);
		m.unlock();
		my_Net->send_cs_map_ok_packet();
	}
					break;
	case SC_GAME_START: {
		SC_GAME_START_PACKET* p = reinterpret_cast<SC_GAME_START_PACKET*>(packet);

		std::cout << "게임 시작 패킷 수신" << std::endl;
		m.lock();
		if (my_Net->m_pscene)
			my_Net->m_pscene->SetGameStart();
		m.unlock();
	}
		break;
	case SC_POSITION: {
		SC_POSITION_PACKET* p = reinterpret_cast<SC_POSITION_PACKET*>(packet);

		m.lock();
		for (int i = 0; i < 3; ++i) {
			my_Net->m_player[i].info = p->p_info[i];
		}
		m.unlock();

		//std::cout << "플레이어 POS 패킷 수신" << std::endl;
	}
					break;
	case SC_GAME_END: {
		SC_GAME_END_PACKET* p = reinterpret_cast<SC_GAME_END_PACKET*>(packet);

		std::cout << "GAME_END 패킷 수신" << std::endl;
	}
					break;
	default:
		std::cout << "invalid packet" << std::endl;
		break;
	}
}

void CNetModule::RecvThread(SOCKET s, std::mutex& m, std::shared_ptr<CNetModule> my_Net)
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
		//std::cout << retval << "수신" << std::endl;

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

// 소켓 함수 오류 출력 후 종료
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

// 소켓 함수 오류 출력
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
