#pragma once
#include "GameObject.h"
#include <array>
#include <vector>
#include <memory>

class CPlayer;
class CNetModule;
class CMap : public CGameObject
{
private:
	static const int MAX_Layer;

	std::shared_ptr<CNetModule> m_NetModule;

	int& w_width;
	int& w_height;
	std::vector<glm::mat4> map_data;

	std::array<std::unique_ptr<CPlayer>, 3> m_pplayers;

public:
	CMap(std::string filename, int& winWidth, int& winHeight, std::shared_ptr<CNetModule> NetModule);
	virtual ~CMap();

	virtual void Initialize() override;					//생성될 때 할 일
	virtual void Update(float ElapsedTime) override;	//타이머에서 할 일
	virtual void FixedUpdate() override;				//충돌처리 등
	virtual void Render() override;						//드로우
	virtual void Release() override;					//소멸될 때 할 일

	GLuint InitBuffer();

	void KeyboardEvent(int state, unsigned char key);
	void SpecialKeyEvent(int state, int key);
};

