#pragma once
#include "GameObject.h"

class CPlayer : public CGameObject
{
private:
	float time_sec;

	glm::mat4 cameraMat;
	glm::mat4 projMat;
	glm::mat4 modelMat;

	GLuint Textures[4];
	int TexFrame;

	bool isWalk;

	float move_x;
	float move_y;
	float move_z;

	float rotate_angle;

	int my_id;

public:
	CPlayer(int id);
	virtual ~CPlayer();

	virtual void Initialize() override;					//생성될 때 할 일
	virtual void Update(float ElapsedTime) override;	//타이머에서 할 일
	virtual void FixedUpdate() override;				//충돌처리 등
	virtual void Render() override;						//드로우
	virtual void Release() override;					//소멸될 때 할 일

	void SetCamera(glm::mat4 cameraMat);
	void SetProjection(glm::mat4 projMat);

	GLuint InitBuffer();

	void SetWalk(bool isWalk);

	void SetMoveX(float move_x);
	void SetMoveY(float move_y);
	void SetMoveZ(float move_z);

	void SetRotate(float angle);
};

