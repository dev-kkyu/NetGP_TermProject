#include "Scene.h"
#include "Map.h"
#include "NetModule.h"
#include <iostream>

CScene::CScene(int& width, int& height, std::unique_ptr<CNetModule>& NetModule)
	: w_width{ width }, w_height{ height }, m_NetModule{ NetModule }
{
	m_map = std::make_unique<CMap>("./Map/map1.txt", w_width, w_height, std::ref(NetModule));
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
	if (m_map) {
		m_map->Update(ElapsedTime);
	}
}

void CScene::FixedUpdate()
{
}

void CScene::Render()
{
	if (m_map) {
		m_map->Render();
	}
}

void CScene::Release()
{
}

void CScene::MouseEvent(int button, int state, int x, int y)
{
	if (m_map)
		m_map->MouseEvent(button, state, x, y);
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
