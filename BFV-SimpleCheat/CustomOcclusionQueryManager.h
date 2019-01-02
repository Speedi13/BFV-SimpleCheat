#pragma once
#include "FrostbiteSDK.h"

class CustomOcclusionQueryManager
{
public:
	class OcclusionQuery
	{
	public:
		void* m_entity;
		class fb::WorldOcclusionQueryRenderModule::ObjectRenderInfo m_info;
		bool m_occluded;
		bool m_Initialized;
		void CustomOcclusionQueryManager::OcclusionQuery::UpdateQuery( fb::WorldOcclusionQueryRenderModule::ObjectRenderInfo* info );
		void CustomOcclusionQueryManager::OcclusionQuery::UpdateQuery( fb::LinearTransform_AABB* transformAABB );
		bool CustomOcclusionQueryManager::OcclusionQuery::IsVisible( );
	};
private:
	CustomOcclusionQueryManager::OcclusionQuery* m_querys[256];
	fb::Vec4 LocalTransform;
	float m_ObjectScreenAreaCoverage[32*8];
	fb::WorldOcclusionQueryRenderModule::BatchQuery m_Batches[8];
public:
	void __fastcall CustomOcclusionQueryManager::UpdateLocalTransform(fb::Vec4*);
	void __fastcall CustomOcclusionQueryManager::EngineUpdate();
	void __fastcall CustomOcclusionQueryManager::hook();
	bool CustomOcclusionQueryManager::IsInQuery( void* entity );
	CustomOcclusionQueryManager::OcclusionQuery* CustomOcclusionQueryManager::GetQuery( void* entity );
	CustomOcclusionQueryManager::OcclusionQuery* CustomOcclusionQueryManager::AddQuery( void* entity, fb::WorldOcclusionQueryRenderModule::ObjectRenderInfo* info );
	
	static CustomOcclusionQueryManager* GetInstance()
	{
		static class CustomOcclusionQueryManager* g_CustomOcclusionQueryManager = NULL;
		if ( g_CustomOcclusionQueryManager == NULL )
		{
			 g_CustomOcclusionQueryManager = (CustomOcclusionQueryManager*)malloc( sizeof(CustomOcclusionQueryManager) );
			 ZeroMemory( g_CustomOcclusionQueryManager, sizeof(CustomOcclusionQueryManager) );
			 g_CustomOcclusionQueryManager->hook();
		}
		return g_CustomOcclusionQueryManager;
	}	
};

//////////////////////////////////////////////////////////////////////////////////// GAME FUNCTIONS //////////////////////////////////////////////////////////////////////////////////// 
typedef void* 			 (__fastcall* t_fb__WorldOcclusionQueryRenderModule__drawBatchQuery)( 			fb::WorldOcclusionQueryRenderModule*, 
																										QWORD v4, 
																										void*, 
																										fb::WorldOcclusionQueryRenderModule::BatchQuery* );
																										
typedef void* 			 (__fastcall* t_fb__WorldOcclusionQueryRenderModule__retrieveBatchQueryData)( 	fb::WorldOcclusionQueryRenderModule*, 
																										fb::WorldOcclusionQueryRenderModule::BatchQuery*, 
																										float flScreenArea );
																										
typedef unsigned __int64 (__fastcall* t_fb__WorldOcclusionQueryRenderModule__processBatchQueries)( 		fb::WorldOcclusionQueryRenderModule*, 
																										__int64 Arg2, 
																										void*, 
																										bool );
																										
typedef unsigned __int64 (__fastcall* t_fb__WorldOcclusionQueryRenderModule__insertBatchQuery)( 		fb::WorldOcclusionQueryRenderModule*, 
																										int _dwordC, 
																										unsigned int QueryCount, 
																										fb::WorldOcclusionQueryRenderModule::ObjectRenderInfo *ObjectRenderInfo, 
																										float *outObjectScreenAreaCoverage );

extern t_fb__WorldOcclusionQueryRenderModule__drawBatchQuery 			fb__WorldOcclusionQueryRenderModule__drawBatchQuery;
extern t_fb__WorldOcclusionQueryRenderModule__retrieveBatchQueryData 	fb__WorldOcclusionQueryRenderModule__retrieveBatchQueryData;
extern t_fb__WorldOcclusionQueryRenderModule__processBatchQueries 		fb__WorldOcclusionQueryRenderModule__processBatchQueries;
extern t_fb__WorldOcclusionQueryRenderModule__insertBatchQuery 			fb__WorldOcclusionQueryRenderModule__insertBatchQuery;