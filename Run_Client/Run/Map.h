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

	virtual void Initialize() override;					//������ �� �� ��
	virtual void Update(float ElapsedTime) override;	//Ÿ�̸ӿ��� �� ��
	virtual void FixedUpdate() override;				//�浹ó�� ��
	virtual void Render() override;						//��ο�
	virtual void Release() override;					//�Ҹ�� �� �� ��

	GLuint InitBuffer();

	void KeyboardEvent(int state, unsigned char key);
	void SpecialKeyEvent(int state, int key);
};

