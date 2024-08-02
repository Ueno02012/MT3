#pragma once
#include "Vector3.h"

struct Ball {
	Vector3 position;//!<ボールの位置
	Vector3 velocity;//!<ボールの速度
	Vector3 aceleration;//!<ボールの加速度
	float mass;//!<ボールの質量
	float radius;//!<ボールの半径
	unsigned int color;//!<ボールの色
};