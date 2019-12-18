#pragma once
#pragma comment(lib, "BakkesMod.lib")
#include "bakkesmod/plugin/bakkesmodplugin.h"

class CameraControlPluginTemplate : public BakkesMod::Plugin::BakkesModPlugin
{
private:
	bool enabled = false;
	Vector FOCUS;
	Rotator ROTATION, SWIVEL;
	float DISTANCE, FOV;
	bool overrideValue[8];
	bool isInBallCam = false;
	bool isInRearCam = false;

public:
	virtual void onLoad();
	virtual void onUnload();

	void CreateValues();

	void Initialize();
	bool CanCreateValues();
	bool IsCVarNull(string cvarName);
	void Enable();
	void Disable();
	void HandleValues();
	Rotator GetSwivel();
};
