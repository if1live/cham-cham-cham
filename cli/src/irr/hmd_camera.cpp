﻿// Ŭnicode please 
#include "stdafx.h"
#include "hmd_camera.h"
#include "base/template_lib.h"
#include "base/lib.h"
#include "game/input_event.h"

#include "irr/debug_draw_manager.h"

using namespace irr;
using namespace std;

inline bool isSetBinaryFlag(const irr::u32& flags, const irr::u32 offset) {
    return (flags & offset) == offset;
}


CameraEventReceiver::CameraEventReceiver(irr::scene::ICameraSceneNode *cam)
	: cam_(cam),
	horizontalRotate_(0),
	verticalRotate_(0)
{
	std::fill(keyDownStatus_, keyDownStatus_ + getArraySize(keyDownStatus_), false);
}

CameraEventReceiver::~CameraEventReceiver()
{
}

MoveEvent CameraEventReceiver::getMoveEvent() const
{
	MoveEvent evt = keyboardMoveEvent_.merge(joystickMoveEvent_);
	return evt;
}
LookEvent CameraEventReceiver::getLookEvent() const
{
	LookEvent evt = mouseLookEvent_.merge(joystickLookEvent_);
	return evt;
}


bool CameraEventReceiver::OnEvent(const irr::SEvent &evt) 
{
	switch(evt.EventType) {
	case EET_JOYSTICK_INPUT_EVENT:
		onEvent(evt.JoystickEvent);
		break;
	case EET_KEY_INPUT_EVENT:
		onEvent(evt.KeyInput);
		break;
	case EET_MOUSE_INPUT_EVENT:
		onEvent(evt.MouseInput);
		break;
	}
	return false;
}

void CameraEventReceiver::onEvent(const irr::SEvent::SJoystickEvent &evt) 
{
	const float DEAD_ZONE = 0.20f;

	MoveEvent &moveEvent = joystickMoveEvent_;
	LookEvent &lookEvent = joystickLookEvent_;

	const JoystickDevice &joystickDev = Lib::eventReceiver->getJoystickDev();
	const SJoystickInfo &joystickInfo = joystickDev.getJoystickInfo()[0];

	float XMovement = joystickDev.getAxisFloatValue<SEvent::SJoystickEvent::AXIS_X>();
	float YMovement = -joystickDev.getAxisFloatValue<SEvent::SJoystickEvent::AXIS_Y>();

	if(fabs(XMovement) < DEAD_ZONE) {
		XMovement = 0.0f;
	}
	if(fabs(YMovement) < DEAD_ZONE) {
		YMovement = 0.0f;
	}

	const float povDegrees = ((-evt.POV / 100.0) - 90) * 180 / irr::core::PI;
	if(povDegrees < 2 * irr::core::PI && povDegrees > 0) {
        XMovement = cosf(povDegrees);
        YMovement = sinf(povDegrees);
	}

	moveEvent.forwardBackward = YMovement;
	moveEvent.leftRight = XMovement;

	float XView = joystickDev.getAxisFloatValue<SEvent::SJoystickEvent::AXIS_R>();
	float YView = joystickDev.getAxisFloatValue<SEvent::SJoystickEvent::AXIS_U>();
	
	if(fabs(XView) < DEAD_ZONE) {
		XView = 0.0f;
	}
	if(fabs(YView) < DEAD_ZONE) {
		YView = 0.0f;
	}
	lookEvent.horizontalRotation = XView;
	//y는 패드 자체에서 방향이 반대로 나온다?
	lookEvent.verticalRotation = -YView;

	//TODO move가 실제로 발생했을떄만 이동처리하는거 구현할때 살려서 쓴다
	/*
	if(!core::equals(XMovement, 0.f) || !core::equals(YMovement, 0.f)) {
		Moved = true;
	} else {
		Moved = false;
	}
	*/
    for(const KeyMapping::joystickMap_t::value_type &joystickMapValue : Lib::keyMapping->getJoystickKeyMap()) {
        buttonEvent_.setButton(joystickMapValue.second, isSetBinaryFlag(evt.ButtonStates, joystickMapValue.first));
    }
}

void CameraEventReceiver::onEvent(const irr::SEvent::SKeyInput &evt) 
{
	struct KeyCompFunctor {
		KeyCompFunctor(irr::EKEY_CODE key) : key(key) {}
		irr::EKEY_CODE key;

		bool operator()(const KeyMapData &a) {
			return (a.keyCode == key);
		}
	};
    
	//processing move event
    for(const KeyMapping::keyMap_t::value_type &keyMapList : Lib::keyMapping->getKeyMap()){
        auto keyItr = keyMapList.second.begin();
        auto keyEndItr = keyMapList.second.end();
        auto keyFound = std::find_if(keyItr, keyEndItr, KeyCompFunctor(evt.Key));
        if(keyFound != keyEndItr) {
            keyDownStatus_[keyMapList.first] = evt.PressedDown;
        }
    }

	MoveEvent moveEvt;
	if(keyDownStatus_[KeyMapping::kLeftKey]) {
		moveEvt.leftRight -= 1.0f;
	}
	if(keyDownStatus_[KeyMapping::kRightKey]) {
		moveEvt.leftRight += 1.0f;
	}
	if(keyDownStatus_[KeyMapping::kForwardKey]) {
		moveEvt.forwardBackward += 1.0f;
	}
	if(keyDownStatus_[KeyMapping::kBackwardKey]) {
		moveEvt.forwardBackward -= 1.0f;
	}
	keyboardMoveEvent_ = moveEvt;
}

void CameraEventReceiver::onEvent(const irr::SEvent::SMouseInput &evt)
{
	//printf("mouse\n");
	//TODO
	//마우스구현을 집어넣으면 게임 디버깅이 심히 힘들다
}

void CameraEventReceiver::update(int ms)
{
	auto cam = this->getCamera();

	MoveEvent moveEvt = getMoveEvent();
	LookEvent lookEvt = getLookEvent();

	horizontalRotate_ += lookEvt.horizontalRotation;
	verticalRotate_ += lookEvt.verticalRotation;
	const float maxVerticalRotation = 80.0f;
	if(verticalRotate_ < -maxVerticalRotation) {
		verticalRotate_ = -maxVerticalRotation;
	} else if(verticalRotate_ > maxVerticalRotation) {
		verticalRotate_ = maxVerticalRotation;
	}

	//그냥 생각없이 카메라를 돌리자. 오큘러스 대응은 렌더리쪽에서 알아서 처리될거다
	cam->setRotation(core::vector3df(verticalRotate_, horizontalRotate_, 0));
	
	//카메라 처다보는 방향으로 로직을 구현하면 오큘러스에서 설정한 값하고 꼬인다
	//v/h 값으로 따로 계산해야될듯
	core::vector3df pos = cam->getPosition();
	core::vector3df up(0, 1, 0);
	float targetX = cos(core::degToRad(verticalRotate_)) * sin(core::degToRad(horizontalRotate_));
	float targetY = sin(core::degToRad(verticalRotate_));
	float targetZ = cos(core::degToRad(verticalRotate_)) * cos(core::degToRad(horizontalRotate_));
	core::vector3df target(targetX, targetY, targetZ);
	irr::core::vector3df side = target.crossProduct(up);

	const float moveFactor = ((irr::f32)ms) / 10.0f;
	cam->setPosition(pos + moveFactor * moveEvt.forwardBackward * target + moveFactor * moveEvt.leftRight * side);

	using boost::wformat;

	irr::video::SColor white(255, 255, 255, 255);
	auto camPosMsg = (wformat(L"Cam : h=%.2f, v=%.2f") % horizontalRotate_ % verticalRotate_).str();
	g_debugDrawMgr->addString(core::vector2di(0, 100), camPosMsg, white);

	auto evtMsg = (wformat(L"evt : fb=%.2f, lr=%.2f") % moveEvt.forwardBackward % moveEvt.leftRight).str();
	g_debugDrawMgr->addString(core::vector2di(0, 100 + 14*1), evtMsg, white);

	auto targetMsg = (wformat(L"target : %.2f, %.2f, %.2f") % targetX % targetY % targetZ).str();
	g_debugDrawMgr->addString(core::vector2di(0, 100 + 14*2), targetMsg, white);

	auto sideMsg = (wformat(L"side : %.2f, %.2f, %.2f") % side.X % side.Y % side.Z).str();
	g_debugDrawMgr->addString(core::vector2di(0, 100 + 14*3), sideMsg, white);
}