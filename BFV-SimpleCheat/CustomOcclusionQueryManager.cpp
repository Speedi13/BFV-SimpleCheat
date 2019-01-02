#include "CustomOcclusionQueryManager.h"
#include "Util.h"
#include <math.h>

t_fb__WorldOcclusionQueryRenderModule__drawBatchQuery 			fb__WorldOcclusionQueryRenderModule__drawBatchQuery = 			(t_fb__WorldOcclusionQueryRenderModule__drawBatchQuery)			0x141DDAFC0; //4C 8B CD 4C 8B C7 48 8B D6 49 8B CE E8 ?? ?? ?? ?? EB 10
t_fb__WorldOcclusionQueryRenderModule__retrieveBatchQueryData 	fb__WorldOcclusionQueryRenderModule__retrieveBatchQueryData = 	(t_fb__WorldOcclusionQueryRenderModule__retrieveBatchQueryData)	0x1491CB270; //pattern from above minus two
t_fb__WorldOcclusionQueryRenderModule__processBatchQueries 		fb__WorldOcclusionQueryRenderModule__processBatchQueries = 		(t_fb__WorldOcclusionQueryRenderModule__processBatchQueries)	0x141DEF5E0; //00 00 C6 44 ?? ?? ?? 41 B1 01 49 8B D4 E8
t_fb__WorldOcclusionQueryRenderModule__insertBatchQuery 		fb__WorldOcclusionQueryRenderModule__insertBatchQuery = 		(t_fb__WorldOcclusionQueryRenderModule__insertBatchQuery)		0x141DE9720;

unsigned __int64 __fastcall WorldOcclusionQueryRenderModule___HkProcessBatchQueries(fb::WorldOcclusionQueryRenderModule* _this, DWORD64 Qword, fb::WorldViewDesc* viewDesc, bool b )
{
	CustomOcclusionQueryManager::GetInstance()->EngineUpdate();
	return fb__WorldOcclusionQueryRenderModule__processBatchQueries( _this, Qword, viewDesc, b );
}

void __fastcall CustomOcclusionQueryManager::hook()
{
	static bool bHooked = false;
	if (bHooked) return;
#if defined(_DEBUG_ENABLED)	
	printf(""__FUNCTION__"();\n");
#endif
	DWORD64 hookAddress = (DWORD64)fb__WorldOcclusionQueryRenderModule__processBatchQueries;
/*
//bytes that get overwritten by the hook:
//------------------------------------------------------------------
.srdata:0000000141DEF5E0  48 8B C4      mov     rax, rsp
.srdata:0000000141DEF5E3  4C 89 40 18   mov     [rax+18h], r8
.srdata:0000000141DEF5E7  48 89 50 10   mov     [rax+10h], rdx
.srdata:0000000141DEF5EB  55            push    rbp
.srdata:0000000141DEF5EC  56            push    rsi
.srdata:0000000141DEF5ED  57            push    rdi
//------------------------------------------------------------------
*/

	BYTE* jmpBackTrampoline = (BYTE*)VirtualAlloc( NULL, 0x100, (MEM_COMMIT | MEM_RESERVE), PAGE_EXECUTE_READWRITE );
	//copy to be overwritten bytes to the trampoline
	memcpy( (void*)jmpBackTrampoline, (void*)hookAddress, 14 );
	//place jump back in the trampoline
	PlaceJmp( (DWORD64)&jmpBackTrampoline[14], hookAddress+14 );
	
	//set the trampoline as new function address
	fb__WorldOcclusionQueryRenderModule__processBatchQueries = (t_fb__WorldOcclusionQueryRenderModule__processBatchQueries)jmpBackTrampoline;
	//overwrite the first function bytes with a jump to the hook function
	PlaceJmp( hookAddress, (DWORD64)WorldOcclusionQueryRenderModule___HkProcessBatchQueries );
	bHooked = true;
}

void __fastcall CustomOcclusionQueryManager::EngineUpdate()
{
	//printf("function: "__FUNCTION__"\n");
	fb::DxRenderer* pDxRenderer = fb::DxRenderer::GetInstance();
	if (!ValidPointer(pDxRenderer)) return;
	
	fb::GameRenderer* pGameRenderer = fb::GameRenderer::GetInstance();
	if (!ValidPointer(pGameRenderer)) return;
	
	fb::ClientGameContext* pClientGameContext = fb::ClientGameContext::GetInstance();
	if (!ValidPointer(pClientGameContext)) return;
	
	fb::ClientLevel* pLevel = pClientGameContext->m_pClientLevel;
	if (!ValidPointer(pLevel)) return;
	
	fb::WorldRenderModule* pWorldRenderModule = pLevel->m_worldRenderModule;
	if (!ValidPointer(pWorldRenderModule)) return;
	
	fb::WorldRenderer* pWorldRenderer = pWorldRenderModule->m_worldRenderer;
	if (!ValidPointer(pWorldRenderer)) return;
	
	fb::WorldOcclusionQueryRenderModule* pWorldOcclusionQueryRenderModule = pWorldRenderer->m_WorldOcclusionQueriesRenderModule;
	if (!ValidPointer(pWorldOcclusionQueryRenderModule)) return;
	
	fb::WorldRenderer::RootView* RootView = &pWorldRenderer->m_rootviews;
	if (!ValidPointer(RootView)) return;
	
	fb::WorldViewDesc* RootViewDesc = &RootView->m_rootView; //same as viewDesc
	if (!ValidPointer(RootViewDesc)) return;
	
	float flScreenArea = RootView->m_rootView.viewport.width * RootView->m_rootView.viewport.height;

	//first xref of "waitMeshStream" function one of the first instructions of the function
	//48 8B 0D ?? ?? ?? ?? 48 8B 01 49 8B D7 FF 50 28 48 8B + 3
	//trackDis = *(_QWORD *)(GameRenderer + 0x12458);

	QWORD Qword = *(_QWORD *)( (DWORD_PTR)pGameRenderer + 0x12458); //same as qword
	if (!ValidPointer(Qword)) return;

	static DWORD idMap[256];
	static DWORD idMapCount = NULL;

	static bool g_InitBatchQuery = false;
	if ( g_InitBatchQuery == false )
	{
		for (int i = 0; i < ARRAYSIZE(this->m_Batches); i++)
		{
			ZeroMemory( &this->m_Batches[i], sizeof(fb::WorldOcclusionQueryRenderModule::BatchQuery) );
			this->m_Batches[i].status = fb::WorldOcclusionQueryRenderModule::BatchQuery::QS_Inactive;
		}
		g_InitBatchQuery = true;
	}
	
	static bool bQueryActive = false;
	if ( bQueryActive == false )
	{
		ZeroMemory( &idMap[0], ARRAYSIZE(idMap)*4 );
		idMapCount = 0;

		DWORD CurrentBatch = 0;
		DWORD CurrentQuery = 0;
		for (DWORD i = 0; i < ARRAYSIZE(this->m_querys); i++)
		{
			CustomOcclusionQueryManager::OcclusionQuery* pOcclusionQuery = this->m_querys[i];
			if (!ValidPointer(pOcclusionQuery)) continue;
			if ( pOcclusionQuery->m_Initialized != true ) continue;
			if (ValidPointer(pOcclusionQuery->m_entity))
			{
				DWORD64 vtable = NULL;
TRY;
				vtable = *(DWORD64*)pOcclusionQuery->m_entity;
TRY_END;
				if (vtable == NULL || !ValidPointer(vtable) || vtable < 0x140001000 || vtable > 0x14FFFFFFF)
				{
#if defined(_DEBUG_ENABLED)
					printf("freeing pOcclusionQuery\n");
#endif
					this->m_querys[i] = NULL;
					free( pOcclusionQuery );
					continue;
				}
			}
			else
			{
#if defined(_DEBUG_ENABLED)
				printf("freeing pOcclusionQuery\n");
#endif
				this->m_querys[i] = NULL;
				free( pOcclusionQuery );
				continue;
			}

			if (CurrentQuery >= 32)
			{
				CurrentQuery = NULL;
				CurrentBatch++;
			}
			if (CurrentBatch >= 8)
			{
#if defined(_DEBUG_ENABLED)
				printf("ERROR: trying to use more than available batches\n");
#endif	
				CurrentBatch = 0;
			}
			fb::WorldOcclusionQueryRenderModule::BatchQuery* pBatch = &this->m_Batches[CurrentBatch];
			if ( pBatch->status != fb::WorldOcclusionQueryRenderModule::BatchQuery::QS_Init )
				pBatch->renderInfos.clear();

			pBatch->count = CurrentQuery+1;
			pBatch->doneCounter = 0;
			pBatch->queriesInitialized = 0;
			pBatch->dwordC = 0;
			pBatch->status = fb::WorldOcclusionQueryRenderModule::BatchQuery::QS_Init;
			pBatch->outObjectScreenAreaCoverage = &this->m_ObjectScreenAreaCoverage[CurrentBatch*32];
			this->m_ObjectScreenAreaCoverage[CurrentBatch*32 + CurrentQuery ] = 0;
			pBatch->renderInfos.push_back( &pOcclusionQuery->m_info );

			CurrentQuery++;
			idMap[idMapCount] = i;
			idMapCount++;
		}
		int x = CurrentBatch+1;
		if ( idMapCount > 0 )
		{
			for (int j = 0; j < CurrentBatch+1; j++)
			{
				fb::WorldOcclusionQueryRenderModule::BatchQuery* pBatch = &this->m_Batches[j];
				fb::DxRenderQuery* pDxRenderQuery = (fb::DxRenderQuery*)pDxRenderer->createQuery( pWorldOcclusionQueryRenderModule->m_Allocator, fb::RenderQueryType_Occlusion, 32, "worldRenderOcclusion" );
				pBatch->m_dxQuerys = pDxRenderQuery;
				(*(void (__fastcall **)(fb::DxRenderQuery *))pDxRenderQuery->vtable)(pDxRenderQuery); //<< just _InterlockedIncrement
				pBatch->queriesInitialized = 1;
				fb__WorldOcclusionQueryRenderModule__drawBatchQuery( pWorldOcclusionQueryRenderModule, Qword, RootViewDesc, pBatch );
			}
		}
		else
			x = 0;

		for (; x < ARRAYSIZE(this->m_Batches); x++)
		{
			fb::WorldOcclusionQueryRenderModule::BatchQuery* pBatch = &this->m_Batches[x];
			pBatch->count = 0;
			pBatch->queriesInitialized = 0;
			pBatch->status = fb::WorldOcclusionQueryRenderModule::BatchQuery::QS_Inactive;
		}
		bQueryActive = true;
	}
	else
	{
		for (int i = 0; i < ARRAYSIZE(this->m_Batches); i++)
		{
			fb::WorldOcclusionQueryRenderModule::BatchQuery* pBatch = &this->m_Batches[i];
			if (   pBatch->count == NULL 
				|| pBatch->queriesInitialized == 0 
				|| pBatch->status == fb::WorldOcclusionQueryRenderModule::BatchQuery::QS_Inactive
				|| pBatch->outObjectScreenAreaCoverage == 0 
				)
				continue;
			fb__WorldOcclusionQueryRenderModule__retrieveBatchQueryData( pWorldOcclusionQueryRenderModule, pBatch, flScreenArea); //not a problem
			for (int u = 0; u < pBatch->count; u++)
			{
				float fl0X200 = .0f; //mulss   xmm13, dword ptr [rdi+rbx+200h]
				*(DWORD*)&fl0X200 = 0x4002F531; //2.046215296

				//inside fb::SoldierOcclusionQueryManager::updateBatchedOcclusionQueries
				//fl0X200 = sub_141792B90(off_1443A9830, LocalPlayerId, 0);
			
				float flVisibility = this->m_ObjectScreenAreaCoverage[i*32 + u ];
				fb::Vec4 PositionTransform = pBatch->renderInfos.At(u).worldTransform.trans;
				float flDistance = (float)Distance3D( &this->LocalTransform, &PositionTransform );

				float flScreenAreaCoverage = (float)( flDistance * (float)fl0X200 ) * (float)sqrtf( (float)flVisibility * (float)921600.0 );
				
				bool isOccluded = flScreenAreaCoverage == 0;
				
				DWORD qID = idMap[ i*32 + u ];
				CustomOcclusionQueryManager::OcclusionQuery* pOcclusionQuery = this->m_querys[ qID ];
				if (ValidPointer(pOcclusionQuery))
					pOcclusionQuery->m_occluded = isOccluded;
#if defined(_DEBUG_ENABLED)
				else
				{
					consolelog("["__FUNCTION__"] Invalid OcclusionQuery ptr on result write back!");
				}
#endif
			}
			pBatch->count = NULL;
			pBatch->status = fb::WorldOcclusionQueryRenderModule::BatchQuery::QS_Inactive;
			pBatch->queriesInitialized = 0;
		}
		bQueryActive = false;
	};
}

bool CustomOcclusionQueryManager::OcclusionQuery::IsVisible( )
{
	return !this->m_occluded;
}
void CustomOcclusionQueryManager::OcclusionQuery::UpdateQuery( fb::WorldOcclusionQueryRenderModule::ObjectRenderInfo* info )
{
	memcpy( &this->m_info, info, sizeof(fb::WorldOcclusionQueryRenderModule::ObjectRenderInfo) );
	
	if ( this->m_Initialized != true )
		 this->m_Initialized = true;
};

void CustomOcclusionQueryManager::OcclusionQuery::UpdateQuery( fb::LinearTransform_AABB* transformAABB )
{
	fb::Vec4 NewMin = transformAABB->m_Box.min ;//* g_soldierOcclusionBoxScale;
	fb::Vec4 NewMax = transformAABB->m_Box.max ;//* g_soldierOcclusionBoxScale;
	
	this->m_info.vertices[0] = NewMin; //0x0050												 NewMin.x | NewMin.y | NewMin.z | NewMax.w
	this->m_info.vertices[1] = fb::asVec4V(NewMin.x, NewMin.y, NewMax.z,NewMax.w); //0x0060  NewMin.x | NewMin.y | NewMax.z | NewMax.w
	this->m_info.vertices[2] = fb::asVec4V(NewMin.x, NewMax.y, NewMin.z,NewMax.w); //0x0070  NewMin.x | NewMax.y | NewMin.z | NewMax.w
	this->m_info.vertices[3] = fb::asVec4V(NewMin.x, NewMax.y, NewMax.z,NewMax.w); //0x0080  NewMin.x | NewMax.y | NewMax.z | NewMax.w
	this->m_info.vertices[4] = fb::asVec4V(NewMax.x, NewMin.y, NewMin.z,NewMin.w); //0x0090  NewMax.x | NewMin.y | NewMin.z | NewMin.w
	this->m_info.vertices[5] = fb::asVec4V(NewMax.x, NewMin.y, NewMax.z,NewMin.w); //0x00A0  NewMax.x | NewMin.y | NewMax.z | NewMin.w
	this->m_info.vertices[6] = fb::asVec4V(NewMax.x, NewMax.y, NewMin.z,NewMin.w); //0x00B0  NewMax.x | NewMax.y | NewMin.z | NewMin.w
	this->m_info.vertices[7] = NewMax ;//0x00C0												 NewMax.x | NewMax.y | NewMax.z | NewMin.w
	this->m_info.worldTransform = transformAABB->m_Transform;
	
	if ( this->m_Initialized != true )
		 this->m_Initialized = true;
};


void __fastcall CustomOcclusionQueryManager::UpdateLocalTransform(fb::Vec4* LocalTransform)
{
	this->LocalTransform = *LocalTransform;
}

bool CustomOcclusionQueryManager::IsInQuery( void* entity )
{
	for (int i = 0; i < ARRAYSIZE(this->m_querys); i++)
	{
		CustomOcclusionQueryManager::OcclusionQuery* pOcclusionQuery = this->m_querys[i];
		if (!ValidPointer(pOcclusionQuery)) continue;
		if ( entity != NULL && pOcclusionQuery->m_entity == entity)
			return true;
	}
	return false;
}

CustomOcclusionQueryManager::OcclusionQuery* CustomOcclusionQueryManager::GetQuery( void* entity )
{
	for (int i = 0; i < ARRAYSIZE(this->m_querys); i++)
	{
		CustomOcclusionQueryManager::OcclusionQuery* pOcclusionQuery = this->m_querys[i];
		if (!ValidPointer(pOcclusionQuery)) continue;
		if ( entity != NULL && pOcclusionQuery->m_entity == entity)
			return pOcclusionQuery;
	}
	return NULL;
}

CustomOcclusionQueryManager::OcclusionQuery* CustomOcclusionQueryManager::AddQuery( void* entity, fb::WorldOcclusionQueryRenderModule::ObjectRenderInfo* info )
{
	CustomOcclusionQueryManager::OcclusionQuery* oldQuery = this->GetQuery( entity );
	if ( oldQuery != NULL )
		return oldQuery;

	CustomOcclusionQueryManager::OcclusionQuery* pQuery = (CustomOcclusionQueryManager::OcclusionQuery*)malloc( sizeof(CustomOcclusionQueryManager::OcclusionQuery) );
	ZeroMemory( pQuery, sizeof(CustomOcclusionQueryManager::OcclusionQuery) );

	pQuery->m_entity = entity;

	if (info != NULL)
	{
		memcpy( &pQuery->m_info, info, sizeof(fb::WorldOcclusionQueryRenderModule::ObjectRenderInfo) );
		pQuery->m_Initialized = true;
	}

	pQuery->m_occluded = true;

	for (int i = 0; i < ARRAYSIZE(this->m_querys); i++)
	{
		if ( this->m_querys[i] == NULL )
		{
			this->m_querys[i] = pQuery;
			break;
		}
	}

	return pQuery;
}