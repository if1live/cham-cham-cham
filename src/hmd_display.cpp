﻿// Ŭnicode please 
#include "hmd_display.h"
#include "HMDStereoRender.h"
#include "ISceneManager.h"

CHMDDisplay::CHMDDisplay()
{
}
CHMDDisplay::~CHMDDisplay()
{
	shutDown();
}

void CHMDDisplay::setUp(irr::IrrlichtDevice *device)
{
	HMDDescriptor HMD;
	// Parameters from the Oculus Rift DK1
	HMD.hResolution = 1280;
	HMD.vResolution = 800;
	HMD.hScreenSize = 0.14976;
	HMD.vScreenSize = 0.0936;
	HMD.interpupillaryDistance = 0.064;
	HMD.lensSeparationDistance = 0.064;
	HMD.eyeToScreenDistance = 0.041;
	HMD.distortionK[0] = 1.0;
	HMD.distortionK[1] = 0.22;
	HMD.distortionK[2] = 0.24;
	HMD.distortionK[3] = 0.0;

	Renderer.reset(new HMDStereoRender(device, HMD, 10));
}
void CHMDDisplay::shutDown()
{
	Renderer.reset(nullptr);
}

void CHMDDisplay::NormalDrawAll(irr::scene::ISceneManager *smgr)
{
	smgr->drawAll();
}
void CHMDDisplay::StereoDrawAll(irr::scene::ISceneManager *smgr)
{
	Renderer->drawAll(smgr);
}