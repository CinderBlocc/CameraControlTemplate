#include "CameraControlPluginTemplate.h"
#include "bakkesmod\wrappers\includes.h"
#include <sstream>
#include <filesystem>

using namespace std;

BAKKESMOD_PLUGIN(CameraControlPluginTemplate, "Template for creating camera control plugins", "1.0", PLUGINTYPE_FREEPLAY)


/******************************* INSTRUCTIONS ***********************************
	Change all the class name instances
		- i.e. CameraControlPluginTemplate --> MyCamControlPlugin
		- Easily done with Find and Replace
	In onLoad()
		- Change enable/disable notifier name strings
		- i.e. "EnableCamControlTemplatePlugin" --> "EnableMyCamControl"
	In CreateValues
		- Do the calculations to create the orientation values
		- Rotation values are in unreal units: roughly -32768 to 32768
		- Assign values to global variables: FOCUS, ROTATION, DISTANCE, FOV
		- If you don't want a value to be overriden, set its array index to false


	This plugin will work in offline gameplay.
	It will also work in private matches BUT ONLY IF TIME IS SET TO UNLIMITED! 
*********************************************************************************/

void CameraControlPluginTemplate::onLoad()
{
	Initialize();
	cvarManager->registerNotifier("EnableCamControlTemplatePlugin", [this](std::vector<string> params){Enable();}, "Enables camera control plugin", PERMISSION_ALL);
	cvarManager->registerNotifier("DisableCamControlTemplatePlugin", [this](std::vector<string> params){Disable();}, "Disables camera control plugin", PERMISSION_ALL);
}


void CameraControlPluginTemplate::CreateValues()
{
	/*
	READ-ONLY VALUES: bool isInBallCam, bool isInRearView, Rotator SWIVEL
	overrideValue array indices:
		0: Focus X
		1: Focus Y
		2: Focus Z
		3: Rotation Pitch
		4: Rotation Yaw
		5: Rotation Roll
		6: Distance
		7: FOV
	*/

	//WRITE VALUES
	FOCUS.X = -500;
	FOCUS.Y = 0;
	FOCUS.Z = 250;
	ROTATION.Pitch = -20 * 182.044449;
	ROTATION.Yaw = 0;
	ROTATION.Roll = 0;
	DISTANCE = 200;
	FOV = 90;

	//Example: leave DISTANCE and FOV unchanged
	overrideValue[6] = false;
	overrideValue[7] = false;
}









//LEAVE THESE UNCHANGED


void CameraControlPluginTemplate::onUnload(){}
void CameraControlPluginTemplate::Initialize()
{
	//Install parent plugin if it isn't already installed. Ensure parent plugin is loaded.
	if(!experimental::filesystem::exists(".\\bakkesmod\\plugins\\CameraControl.dll"))
		cvarManager->executeCommand("bpm_install 71");
	cvarManager->executeCommand("plugin load CameraControl", false);

	//Hook events
	gameWrapper->HookEvent("Function ProjectX.Camera_X.ClampPOV", std::bind(&CameraControlPluginTemplate::HandleValues, this));
	gameWrapper->HookEvent("Function TAGame.PlayerController_TA.PressRearCamera", [&](std::string eventName){isInRearCam = true;});
	gameWrapper->HookEvent("Function TAGame.PlayerController_TA.ReleaseRearCamera", [&](std::string eventName){isInRearCam = false;});
	gameWrapper->HookEvent("Function TAGame.CameraState_BallCam_TA.BeginCameraState", [&](std::string eventName){isInBallCam = true;});
	gameWrapper->HookEvent("Function TAGame.CameraState_BallCam_TA.EndCameraState", [&](std::string eventName){isInBallCam = false;});
}
bool CameraControlPluginTemplate::CanCreateValues()
{
	if(!enabled || IsCVarNull("CamControl_Swivel_READONLY") || IsCVarNull("CamControl_Focus") || IsCVarNull("CamControl_Rotation") || IsCVarNull("CamControl_Distance") || IsCVarNull("CamControl_FOV"))
		return false;
	else
		return true;
}
bool CameraControlPluginTemplate::IsCVarNull(string cvarName)
{
    struct CastStructOne
    {
        struct CastStructTwo{void* address;};
        CastStructTwo* casttwo;
    };

	CVarWrapper cvar = cvarManager->getCvar(cvarName);
    CastStructOne* castone = (CastStructOne*)&cvar;
    return castone->casttwo->address == NULL;
}
void CameraControlPluginTemplate::Enable()
{
	cvarManager->executeCommand("CamControl_Enable 1", false);
	enabled = true;
}
void CameraControlPluginTemplate::Disable()
{
	enabled = false;
	cvarManager->executeCommand("CamControl_Enable 0", false);
}
void CameraControlPluginTemplate::HandleValues()
{
	if(!CanCreateValues())
		return;
	
	//Reset values so that the game won't crash if the developer doesn't assign values to variables
	overrideValue[0] = true;//Focus X
	overrideValue[1] = true;//Focus Y
	overrideValue[2] = true;//Focus Z
	overrideValue[3] = true;//Rotation Pitch
	overrideValue[4] = true;//Rotation Yaw
	overrideValue[5] = true;//Rotation Roll
	overrideValue[6] = true;//Distance
	overrideValue[7] = true;//FOV

	SWIVEL = GetSwivel();
	FOCUS = Vector{0,0,0};
	ROTATION = Rotator{0,0,0};
	DISTANCE = 100;
	FOV = 90;

	//Get values from the developer
	CreateValues();

	//Send value requests to the parent mod
	string values[8];
	values[0] = to_string(FOCUS.X);
	values[1] = to_string(FOCUS.Y);
	values[2] = to_string(FOCUS.Z);
	values[3] = to_string(ROTATION.Pitch);
	values[4] = to_string(ROTATION.Yaw);
	values[5] = to_string(ROTATION.Roll);
	values[6] = to_string(DISTANCE);
	values[7] = to_string(FOV);
	
	for(int i=0; i<8; i++)
	{
		if(!overrideValue[i])
			values[i] = "NULL";
	}

	cvarManager->getCvar("CamControl_Focus").setValue(values[0] + "," + values[1] + "," + values[2]);
	cvarManager->getCvar("CamControl_Rotation").setValue(values[3] + "," + values[4] + "," + values[5]);
	cvarManager->getCvar("CamControl_Distance").setValue(values[6]);
	cvarManager->getCvar("CamControl_FOV").setValue(values[7]);
}
Rotator CameraControlPluginTemplate::GetSwivel()
{
	if(IsCVarNull("CamControl_Swivel_READONLY"))
		return Rotator{0,0,0};

	string readSwivel = cvarManager->getCvar("CamControl_Swivel_READONLY").getStringValue();
	string swivelInputString;
	stringstream ssSwivel(readSwivel);

	Rotator SWIVEL = {0,0,0};

	getline(ssSwivel, swivelInputString, ',');
	SWIVEL.Pitch = stof(swivelInputString);
	getline(ssSwivel, swivelInputString, ',');
	SWIVEL.Yaw = stof(swivelInputString);
	getline(ssSwivel, swivelInputString, ',');
	SWIVEL.Roll = stof(swivelInputString);

	return SWIVEL;
}