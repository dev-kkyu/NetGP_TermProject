#pragma once
#include "GameObject.h"
#include <vector>
#include <memory>

class CPlayer;
class CNetModule;
class CMap : public CGameObject
{
private:
	static const int MAX_Layer;

	std::unique_ptr<CNetModule>& m_NetModule;

	int& w_width;
	int& w_height;
	std::vector<glm::mat4> map_data;

	float move_z;
	int map_index;
	int bottom_index;

	std::unique_ptr<CPlayer> m_pplayer;

	bool isLeft;
	bool isRight;
	bool isSpace;
	float move_x;
	float acc_x;

	float move_y;
	bool isBottom;
	bool isDrop;
	float basic_v;
	float velocity;

	float stop_time;

	// 회전용 변수
	bool is_Rotating;
	float now_angle;
	float bef_mv_x;
	float bef_mv_y;

public:
	CMap(std::string filename, int& winWidth, int& winHeight, std::unique_ptr<CNetModule>& NetModule);
	virtual ~CMap();

	virtual void Initialize() override;					//생성될 때 할 일
	virtual void Update(float ElapsedTime) override;	//타이머에서 할 일
	virtual void FixedUpdate() override;				//충돌처리 등
	virtual void Render() override;						//드로우
	virtual void Release() override;					//소멸될 때 할 일

	GLuint InitBuffer();

	void MouseEvent(int button, int state, int x, int y);
	void KeyboardEvent(int state, unsigned char key);
	void SpecialKeyEvent(int state, int key);

	bool isOffTile();
	void MoveBackOnTile();

	void SetRotate();
	void RotateMap(float ElapsedTime);
};

