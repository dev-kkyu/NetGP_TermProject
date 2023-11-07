#pragma once
#include "stdafx.h"
#include <memory>

class CMap;
class CNetModule;
class CScene
{
private:
	int& w_width;
	int& w_height;

	std::unique_ptr<CNetModule>& m_NetModule;
	std::unique_ptr<CMap> m_map;

public:
	CScene(int& width, int& height, std::unique_ptr<CNetModule>& NetModule);
	~CScene();

	void Initialize();				//생성될 때 할 일
	void Update(float ElapsedTime);	//타이머에서 할 일
	void FixedUpdate();				//충돌처리 등
	void Render();					//드로우
	void Release();					//소멸될 때 할 일

	void MouseEvent(int button, int state, int x, int y);
	void KeyboardEvent(int state, unsigned char key);
	void SpecialKeyEvent(int state, int key);
};

