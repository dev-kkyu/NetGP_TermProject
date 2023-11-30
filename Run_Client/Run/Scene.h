#pragma once
#include "stdafx.h"
#include <memory>

class CLobby;
class CMap;
class CNetModule;
class CScene
{
private:
	int& w_width;
	int& w_height;

	bool is_lobby;

	std::shared_ptr<CNetModule> m_NetModule;
	std::unique_ptr<CMap> m_map;

public:
	std::unique_ptr<CLobby> m_lobby;

public:
	CScene(int& width, int& height, std::shared_ptr<CNetModule> NetModule);
	~CScene();

	void Initialize();				//생성될 때 할 일
	void Update(float ElapsedTime);	//타이머에서 할 일
	void FixedUpdate();				//충돌처리 등
	void Render();					//드로우
	void Release();					//소멸될 때 할 일

	void SetID(char player_id);
	void SetIsReady(char player_id, bool is_ready);
	void SetIsAccept(char player_id, bool is_accept);
	void SetMap(float map_data[100][16]);
	void SetGameStart();

	void MouseEvent(int button, int state, int x, int y);
	void KeyboardEvent(int state, unsigned char key);
	void SpecialKeyEvent(int state, int key);
};

