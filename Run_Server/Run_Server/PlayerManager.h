#pragma once

#include "protocol.h"

class CPlayerManager
{
public:
	PlayerData info;

public:
	CPlayerManager();
	~CPlayerManager();

	void Update(float ElapsedTime);

};

