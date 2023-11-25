#include "Scene.h"
#include "Lobby.h"
#include "Map.h"
#include "NetModule.h"
#include <iostream>

CScene::CScene(int& width, int& height, std::shared_ptr<CNetModule> NetModule)
	: w_width{ width }, w_height{ height }, m_NetModule{ NetModule }, is_lobby{ true }
{
	m_lobby = std::make_unique<CLobby>();
	m_map = std::make_unique<CMap>("./Map/map1.txt", w_width, w_height, NetModule);

	m_NetModule->m_mutex.lock();
	for (int i = 0; i < 3; ++i)
		m_lobby->is_ready[i] = m_NetModule->m_is_ready[i];
	m_lobby->my_id = m_NetModule->my_id;
	m_NetModule->m_mutex.unlock();

	Initialize();
}

CScene::~CScene()
{
	Release();
}

void CScene::Initialize()
{

}

void CScene::Update(float ElapsedTime)
{
	if (not is_lobby)
		if (m_map)
			m_map->Update(ElapsedTime);
}

void CScene::FixedUpdate()
{
}

void CScene::Render()
{
	if (m_lobby)
		m_lobby->Render();
	if (not is_lobby)
		if (m_map)
			m_map->Render();
}

void CScene::Release()
{
}

void CScene::SetID(char player_id)
{
	if (m_lobby)
		m_lobby->my_id = player_id;
}

void CScene::SetIsReady(char player_id, bool is_ready)
{
	if (m_lobby)
		m_lobby->is_ready[player_id] = is_ready;
}

void CScene::SetGameStart()
{
	is_lobby = false;
	if (m_lobby)
		m_lobby->is_lobby = false;
}

void CScene::MouseEvent(int button, int state, int x, int y)
{
	static const int WHEEL_UP = 3, WHEEL_DOWN = 4;
	switch (state) {
	case GLUT_DOWN:
		switch (button) {
		case GLUT_LEFT_BUTTON:
			m_NetModule->send_cs_ready_packet();
			break;
		case GLUT_RIGHT_BUTTON:
			break;
		case GLUT_MIDDLE_BUTTON:
			break;
		case WHEEL_DOWN:
			break;
		case WHEEL_UP:
			break;
		}
		break;
	case GLUT_UP:
		break;
	default:
		break;
	}
}

void CScene::KeyboardEvent(int state, unsigned char key)
{
	if (m_map)
		m_map->KeyboardEvent(state, key);
}

void CScene::SpecialKeyEvent(int state, int key)
{
	if (m_map)
		m_map->SpecialKeyEvent(state, key);
}
