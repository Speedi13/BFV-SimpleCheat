
#include "ESP.h"
#include "Util.h"
#include "ObfuscationMgr.h" //https://github.com/Speedi13/BFV-Decryption
#include "CustomOcclusionQueryManager.h" //https://github.com/Speedi13/BFV-OcclusionQueryManager

#include <math.h>
#include <stdio.h>
ESP* g_pESP = NULL;


ESP::ESP(void)
{
	this->m_pLocalPlayer = NULL;
	this->m_pLocalSoldier = NULL;
}


ESP::~ESP(void)
{
}


void ESP::Render( fb::DxRenderer* pDxRenderer )
{
	if (!ValidPointer(pDxRenderer)) return;

	fb::ClientGameContext* pClientGameContext = fb::ClientGameContext::GetInstance();
	if (!ValidPointer(pClientGameContext)) return;

	fb::ClientPlayerManager* pPlayerManager = pClientGameContext->m_playerManager;
	if (!ValidPointer(pPlayerManager)) return;

	fb::ClientLevel* pLevel = pClientGameContext->m_pClientLevel;
	if (!ValidPointer(pLevel)) return;

	fb::DebugRenderer2* pDbgRenderer = fb::DebugRenderer2::Singleton();
	if (!ValidPointer(pDbgRenderer)) return;

	fb::GameRenderer* pGameRenderer = fb::GameRenderer::GetInstance();
	if (!ValidPointer(pGameRenderer)) return;

	fb::RenderView* pRenderView = pGameRenderer->m_pRenderView;
	if (!ValidPointer(pRenderView)) return;
	
	/*fb::ClientPlayer*/ this->m_pLocalPlayer = GetLocalPlayer();
	if (!ValidPointer( this->m_pLocalPlayer )) return;

	pDbgRenderer->drawText( 20, 20, fb::Color32::White(), this->m_pLocalPlayer->m_pName, 2 );

	/*fb::ClientSoldierEntity*/ m_pLocalSoldier = this->m_pLocalPlayer->GetSoldier();
	if (!ValidPointer( this->m_pLocalSoldier )) return;

	int LocalTeamId = this->m_pLocalPlayer->GetTeamId();

	//Getting the transform Matrix for the local soldier:
	fb::LinearTransform LocalPlayerTransform;
	this->m_pLocalSoldier->GetTransform( &LocalPlayerTransform );

	CustomOcclusionQueryManager* COcclusionQuery = CustomOcclusionQueryManager::GetInstance();

	fb::Vec4 vLocalPos = LocalPlayerTransform.trans;
	COcclusionQuery->UpdateLocalTransform( &vLocalPos );

	//extracting the position:
	fb::Vec4 vecLocalPosition = LocalPlayerTransform.trans;

	__int32 maxPlayerCount = 70;

	for (int i = 0; i < maxPlayerCount; i++)
	{
		fb::ClientPlayer* pPlayer = GetPlayerById( i );
		if (!ValidPointer( pPlayer )) continue;
		//don't display players from the own team
		if ( pPlayer->GetTeamId() == LocalTeamId ) continue;

		//getting the soldier pointer from the player:
		fb::ClientSoldierEntity* pSoldierEntity = pPlayer->GetSoldier();
		if (!ValidPointer( pSoldierEntity )) continue;

		fb::HealthComponent* pHealthComponent = pSoldierEntity->GetHealthComponent();
		if (!ValidPointer( pHealthComponent ) || !pHealthComponent->isAlive()) continue;

		//Getting the transform Matrix for the player:
		fb::LinearTransform PlayerTransform;
		pSoldierEntity->GetTransform( &PlayerTransform );

		//extracting the position:
		fb::Vec4 vecPosition = PlayerTransform.trans, vecScreenPos;
		if ( !this->ScreenProject( vecPosition, &vecScreenPos ) ) continue; 

		fb::Color32 PlayerColor = fb::Color32::White();
		if ( pSoldierEntity->IsVisible() )
		{
			PlayerColor = fb::Color32::Blue();
		}

		float anglex = XAngle(  vecLocalPosition.x,
							    vecLocalPosition.z,
							    vecPosition.x,
							    vecPosition.z,
							    0
							  );
		float posl[4],posr[4];

		//simple usage just as demo
		RotatePointAlpha(posl, -0.5f, 0.0f, 1.8f, 0.0f, 0.0f, 0.0f, -anglex + 90.0f);

/*
		//proper usage
		if (pSoldierEntity->m_poseType == 0)
		{
			RotatePointAlpha(posl, -0.5f, 0.0f, 1.8f, 0.0f, 0.0f, 0.0f, -anglex + 90.0f);
		}
		else if (pSoldierEntity->m_poseType == 1)
		{
			RotatePointAlpha(posl, -0.5f, 0.0f, 0.9f, 0.0f, 0.0f, 0.0f, -anglex + 90.0f);
		}
		else if (pSoldierEntity->m_poseType == 2)
		{
			RotatePointAlpha(posl, -0.5f, 0.0f, 0.4f, 0.0f, 0.0f, 0.0f, -anglex + 90.0f);
		}
*/

		RotatePointAlpha(posr, 0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, -anglex + 90.0f);

		fb::Vec4 vposl,vposr;

		vposl.x = vecPosition.x + posl[0];
		vposl.y = vecPosition.y + posl[2];
		vposl.z = vecPosition.z + posl[1];
		vposl.w = 0;
		vposr.x = vecPosition.x + posr[0];
		vposr.y = vecPosition.y + posr[2];
		vposr.z = vecPosition.z + posr[1];
		vposr.w = 0;
		fb::Vec4 screenPosl, screenPosr ;

		if ( !this->ScreenProject( vposl, &screenPosl) || 
			 !this->ScreenProject( vposr, &screenPosr) 
		   )
		{
		   continue;
		}

		float flSoldierPos_x = screenPosr.x;
		float flSoldierPos_y = screenPosl.y;

		float flSoldierPos_w = (float)abs( (float)(flSoldierPos_x - screenPosl.x) );
		float flSoldierPos_h = (float)abs( (float)(flSoldierPos_y - screenPosr.y) );

		if ( pPlayer->InVehicle() )
		{
			fb::ClientVehicleEntity* pVehicle = pPlayer->GetVehicle();
			if (ValidPointer(pVehicle))
			{
				fb::VehicleEntityData* pVehicleData = pVehicle->GetEntityData();
				if (ValidPointer(pVehicleData))
				{
					fb::LinearTransform_AABB TransformAABB = {};
					pVehicle->GetTransformAABB( TransformAABB );

					COcclusionQuery->Enter();
					CustomOcclusionQueryManager::OcclusionQuery* pQuery = COcclusionQuery->GetQuery( pVehicle );
					if (pQuery == NULL)
						pQuery = COcclusionQuery->AddQuery( pVehicle, NULL );

					pQuery->UpdateQuery( &TransformAABB );

					bool bVehicleVisible = pQuery->IsVisible();
					COcclusionQuery->Leave();
					TransformAABB.m_Transform.trans.y += 3;
					fb::Vec4 VehicleScreenPos;
					if ( this->ScreenProject( TransformAABB.m_Transform.trans, &VehicleScreenPos ) )
					{
						fb::Color32 VehicleColor = bVehicleVisible ? fb::Color32::Red() : fb::Color32::Yellow();

						char text[512];
						sprintf_s( text, "[%s]\nIsVisible = %u",pVehicleData->m_ControllableType,bVehicleVisible);

						pDbgRenderer->drawText( (int)VehicleScreenPos.x ,
												(int)VehicleScreenPos.y - 8,
												VehicleColor,
												text,
												1
											  );
					}
				}
			}
		}
		//Draw the name of the Player:
		pDbgRenderer->drawText( (int)flSoldierPos_x ,
								(int)flSoldierPos_y + (int)flSoldierPos_h + 6,
								PlayerColor,
								pPlayer->m_pName,
								1
							  );

		//Draw a box around him:
		pDbgRenderer->DrawBox2D( 
								flSoldierPos_x,
								flSoldierPos_y,
								flSoldierPos_w,
								flSoldierPos_h,
								PlayerColor
							   );

		//Draws the skeleton
		this->DrawSkeletonESP( pSoldierEntity, PlayerColor );


	}
}

//calculates the 2D screen position (x, y) from a 3D world position (x, y, z)
//if its visible on screen, it returns true
bool ESP::ScreenProject(fb::Vec4 WorldPos, fb::Vec4* ScreenPos)
{
	fb::GameRenderer* pGameRenderer = fb::GameRenderer::GetInstance();
	if (!ValidPointer(pGameRenderer)) return false;

	fb::RenderView* pRenderView = pGameRenderer->m_pRenderView;
	if (!ValidPointer(pRenderView)) return false;

	fb::DxRenderer* pDxRenderer = fb::DxRenderer::GetInstance();
	if (!ValidPointer(pDxRenderer)) return false;

	fb::RenderScreenInfo* pScreenInfo = pDxRenderer->getScreenInfo(0);
	if (!ValidPointer(pScreenInfo)) return false;

	fb::LinearTransform viewProj = pRenderView->m_ViewProj;

	float mX = (float)pScreenInfo->width * 0.5f;
	float mY = (float)pScreenInfo->height * 0.5f;

	float w =
		viewProj.right.w	* WorldPos.x +
		viewProj.up.w		* WorldPos.y +
		viewProj.forward.w	* WorldPos.z +
		viewProj.trans.w;

	if (w < 0.65f)
	{
		ScreenPos->z = w;
		return false;
	}
 
	float x =
		viewProj.right.x	* WorldPos.x +
		viewProj.up.x		* WorldPos.y +
		viewProj.forward.x	* WorldPos.z +
		viewProj.trans.x;
 
	float y =
		viewProj.right.y	* WorldPos.x +
		viewProj.up.y		* WorldPos.y +
		viewProj.forward.y	* WorldPos.z +
		viewProj.trans.y;
 
	ScreenPos->x = mX + mX * x / w;
	ScreenPos->y = mY - mY * y / w;
	ScreenPos->z = w;
 
	return true;
}

void ESP::DrawSkeletonESP( fb::ClientSoldierEntity* pEnemySoldier, fb::Color32 Color )
{
	fb::Vec4 BonePosition[12];
	int BoneID[12] = { 
					 fb::BONE_Head,				//00
					 fb::BONE_LeftHand,			//01
					 fb::BONE_RightHand,		//02
					 fb::BONE_RightFoot,		//03
					 fb::BONE_RightKneeRoll,	//04
					 fb::BONE_LeftKneeRoll,		//05
					 fb::BONE_LeftFoot,			//06
					 fb::BONE_RightShoulder,	//07
					 fb::BONE_LeftShoulder,		//08
					 fb::BONE_Spine,			//09
					 fb::BONE_RightElbowRoll,	//10
					 fb::BONE_LeftElbowRoll		//11
				  };

	for (int i = 0; i < 12; i++)
	{
		if ( !pEnemySoldier->GetBonePos( BoneID[i], &BonePosition[i] ) ) return;
	}

	// Head -> Hips
	DrawBone( BonePosition[0], BonePosition[9], Color );
	
	// Hips -> left knee
	DrawBone( BonePosition[9], BonePosition[5], Color );
	// left knee -> left food
	DrawBone( BonePosition[5] ,BonePosition[6], Color );

	// Hips -> right knee
	DrawBone( BonePosition[9], BonePosition[4], Color );
	// right knee -> right food
	DrawBone( BonePosition[4], BonePosition[3], Color );

	//right shoulder -> left shoulder
	DrawBone( BonePosition[7], BonePosition[8], Color );

	// right shoulder -> right elbow -> right hand
	DrawBone( BonePosition[7], BonePosition[10], Color );
		DrawBone(BonePosition[10], BonePosition[2], Color );

	// left shoulder -> left elbow -> left hand
	DrawBone( BonePosition[8], BonePosition[11], Color );
		DrawBone(BonePosition[11], BonePosition[1], Color );
}


void ESP::DrawBone( fb::Vec4 from, fb::Vec4 to, fb::Color32 color)
{
	fb::Vec4 W2S_from;
	if ( !this->ScreenProject(from,& W2S_from) ) return;

	fb::Vec4 W2S_to;
	if ( !this->ScreenProject(to,& W2S_to) ) return;

	fb::DebugRenderer2::Singleton()->DrawVecLine( W2S_from, W2S_to, color );
}
