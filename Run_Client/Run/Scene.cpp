#include "Scene.h"
#include "Map.h"
#include <iostream>

CScene::CScene(int& width, int& height) : w_width{ width }, w_height{ height }
{
	Initialize();
}

CScene::~CScene()
{
	Release();
}

void CScene::Initialize()
{
	m_map = std::make_unique<CMap>("./Map/map1.txt", w_width, w_height);

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
	static const int WHEEL_UP = 3, WHEEL_DOWN = 4;
	switch (state) {
	case GLUT_DOWN:
		switch (button) {
		case GLUT_LEFT_BUTTON:
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
