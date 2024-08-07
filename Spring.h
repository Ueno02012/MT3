#pragma once
#include "Vector3.h"

struct Spring
{
	// アンカー。固定された端の位置
	Vector3 anchor;
	float naturalLength;//!<自然長
	float stiffness;//!<剛性。ばね定数k
	float dampingCoefficient; // 減衰係数
};

