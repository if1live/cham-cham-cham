﻿// Ŭnicode please 
#pragma once

#include "debug_draw_manager.h"

class DebugDrawer {
public:
	DebugDrawer() : device_(nullptr), batchSceneNode_(nullptr) {}
	~DebugDrawer() {}

	void startUp(irr::IrrlichtDevice *dev);
	void shutDown();
	void drawAll();

public:
	void drawList(const DrawAttributeList_Line2D &cmd);
	void drawList(const DrawAttributeList_Cross2D &cmd);
	void drawList(const DrawAttributeList_Circle2D &cmd);
	void drawList(const DrawAttributeList_String2D &cmd);
	void drawList(const DrawAttributeList_Line3D &cmd);
	void drawList(const DrawAttributeList_Cross3D &cmd);
	void drawList(const DrawAttributeList_Sphere3D &cmd);
	void drawList(const DrawAttributeList_Axis3D &cmd);
	void drawList(const DrawAttributeList_String3D &cmd);

private:
	irr::IrrlichtDevice *device_;

private:
	LineBatchSceneNode *batchSceneNode_;
	// thickness != 1 인 경우도 렌더링 로직은 동일하게 처리하기 위해서 도입
	std::map<float, LineBatchSceneNode *> batchSceneNodeMap_;
	LineBatchSceneNode *getBatchSceneNode(float thickness);
};