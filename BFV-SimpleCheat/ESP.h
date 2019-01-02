#pragma once
#include "FrostbiteSDK.h"

class ESP
{
public:
	ESP(void);
	~ESP(void);

	void Render( fb::DxRenderer* );

private:
	fb::ClientPlayer* m_pLocalPlayer;
	fb::ClientSoldierEntity* m_pLocalSoldier;

	bool ScreenProject(fb::Vec4 WorldPos, fb::Vec4* ScreenPos);

	void DrawSkeletonESP( fb::ClientSoldierEntity* pEnemySoldier, fb::Color32 Color );
		void DrawBone( fb::Vec4 from, fb::Vec4 to, fb::Color32 color);
};
extern ESP* g_pESP;