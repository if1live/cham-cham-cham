﻿// Ŭnicode please 
#pragma once

#include "scene.h"

class DebugDrawManager2D;
class DebugDrawManager3D;

class DebugDrawScene : public Scene {
public:
	DebugDrawScene();
	virtual ~DebugDrawScene();

	virtual void update(int ms);
};

