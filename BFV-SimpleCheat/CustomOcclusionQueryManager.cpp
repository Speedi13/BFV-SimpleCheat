#include "FrostbiteSDK.h"
#include "Util.h"
#include "CustomOcclusionQueryManager.h"
#include <math.h>
#include <stdio.h>

t_fb__WorldOcclusionQueryRenderModule__drawBatchQuery           fb__WorldOcclusionQueryRenderModule__drawBatchQuery =           (t_fb__WorldOcclusionQueryRenderModule__drawBatchQuery)         0x1419EDD60; //4C 8B ?? ?? 8B ?? 48 8B ?? 49 8B ?? E8 ?? ?? ?? ?? E9 ?? ?? ?? ?? CC
//t_fb__WorldOcclusionQueryRenderModule__retrieveBatchQueryData   fb__WorldOcclusionQueryRenderModule__retrieveBatchQueryData =   (t_fb__WorldOcclusionQueryRenderModule__retrieveBatchQueryData) 0x1419885C0; //got inlined
t_fb__WorldOcclusionQueryRenderModule__processBatchQueries      fb__WorldOcclusionQueryRenderModule__processBatchQueries =      (t_fb__WorldOcclusionQueryRenderModule__processBatchQueries)    0x141A10300; //00 00 C6 44 ?? ?? ?? 41 B1 01 49 8B ?? E8
t_fb__WorldOcclusionQueryRenderModule__insertBatchQuery         fb__WorldOcclusionQueryRenderModule__insertBatchQuery =         (t_fb__WorldOcclusionQueryRenderModule__insertBatchQuery)       0;//0x?????????; //not needed


unsigned __int64 __fastcall WorldOcclusionQueryRenderModule___HkProcessBatchQueries(fb::WorldOcclusionQueryRenderModule* _this, DWORD64 Qword, fb::WorldViewDesc* viewDesc, bool b )
{
    CustomOcclusionQueryManager::GetInstance()->EngineUpdate( viewDesc );
    return fb__WorldOcclusionQueryRenderModule__processBatchQueries( _this, Qword, viewDesc, b );
}

typedef DWORD64 (__fastcall* t_fb__ClientVehicleEntity__Destructor)( void* entity, bool a2 );
t_fb__ClientVehicleEntity__Destructor fb__ClientVehicleEntity__Destructor = NULL;

DWORD64 __fastcall fb__ClientVehicleEntity__hkDestructor(void* entity, bool a2)
{
#if defined(_DEBUG_ENABLED)
    printf("[fb::ClientVehicleEntity::Destructor] freeing pOcclusionQuery\n");
#endif
    CustomOcclusionQueryManager::GetInstance()->RemoveQuery( entity );
    //release hooked vtable
    VirtualFree( *(void**)entity, NULL, MEM_RELEASE );
    return fb__ClientVehicleEntity__Destructor( entity, a2 );
}

void __fastcall CustomOcclusionQueryManager::hook()
{
    static bool bHooked = false;
    if (bHooked) return;
#if defined(_DEBUG_ENABLED) 
    printf(""__FUNCTION__"();\n");
#endif
	//return;//SSSSSSSSSKKKKIIIPPP for now!
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

void __fastcall CustomOcclusionQueryManager::EngineUpdate( void* viewDesc )
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

	/*
    fb::WorldRenderer::RootView* RootView = &pWorldRenderer->m_rootviews;
    if (!ValidPointer(RootView)) return;

    fb::WorldViewDesc* RootViewDesc = &RootView->m_rootView; //same as viewDesc
    if (!ValidPointer(RootViewDesc)) return;
	*/
	fb::WorldViewDesc* RootViewDesc = (fb::WorldViewDesc*)viewDesc;

    float flScreenArea = (float)RootViewDesc->viewport.width * RootViewDesc->viewport.height;

    //first xref of "waitMeshStream" function one of the first instructions of the function
    //48 8B 01 FF 50 38 C6 86 ?? ?? ?? 00 - after this pattern "mov r14, [rsi+12458h]"
    //trackDis = *(_QWORD *)(GameRenderer + 0x12458);

    QWORD Qword = *(_QWORD *)( (DWORD_PTR)pGameRenderer + 0x12458); //same as qword
    if (!ValidPointer(Qword)) return;

    this->Enter();

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
            /* //replaced by hooking the destructor
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
            */
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
            for (unsigned int j = 0; j < CurrentBatch+1; j++)
            {
                fb::WorldOcclusionQueryRenderModule::BatchQuery* pBatch = &this->m_Batches[j];
                if ( pBatch->queriesInitialized != 1 || !ValidPointer(pBatch->m_dxQueries))
                {
                    fb::DxRenderQuery* pDxRenderQuery = (fb::DxRenderQuery*)pDxRenderer->createQuery( pWorldOcclusionQueryRenderModule->m_Allocator, fb::RenderQueryType_Occlusion, 32, "worldRenderOcclusion" );
                    pBatch->m_dxQueries = pDxRenderQuery;
                    (*(void (__fastcall **)(fb::DxRenderQuery *))pDxRenderQuery->vtable)(pDxRenderQuery); //<< just _InterlockedIncrement
                    pBatch->queriesInitialized = 1;
                }
                fb__WorldOcclusionQueryRenderModule__drawBatchQuery( pWorldOcclusionQueryRenderModule, Qword, RootViewDesc, pBatch );
            }
        }
        else
            x = 0;

        for (; x < ARRAYSIZE(this->m_Batches); x++)
        {
            fb::WorldOcclusionQueryRenderModule::BatchQuery* pBatch = &this->m_Batches[x];
            pBatch->count = 0;
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
            for (unsigned int u = 0; u < pBatch->count; u++)
            {
                float fl0X200 = .0f; //mulss   xmm13, dword ptr [rdi+rbx+200h]
                *(DWORD*)&fl0X200 = 0x4002F531; //2.046215296

                //https://github.com/Speedi13/BFV-OcclusionQueryManager/blob/master/Reversed_Occlusion_functions.h#L78
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
        }
        bQueryActive = false;
    };
    this->Leave();
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
    
    this->m_info.vertices[0] = NewMin; //0x0050                                              NewMin.x | NewMin.y | NewMin.z | NewMax.w
    this->m_info.vertices[1] = fb::asVec4V(NewMin.x, NewMin.y, NewMax.z,NewMax.w); //0x0060  NewMin.x | NewMin.y | NewMax.z | NewMax.w
    this->m_info.vertices[2] = fb::asVec4V(NewMin.x, NewMax.y, NewMin.z,NewMax.w); //0x0070  NewMin.x | NewMax.y | NewMin.z | NewMax.w
    this->m_info.vertices[3] = fb::asVec4V(NewMin.x, NewMax.y, NewMax.z,NewMax.w); //0x0080  NewMin.x | NewMax.y | NewMax.z | NewMax.w
    this->m_info.vertices[4] = fb::asVec4V(NewMax.x, NewMin.y, NewMin.z,NewMin.w); //0x0090  NewMax.x | NewMin.y | NewMin.z | NewMin.w
    this->m_info.vertices[5] = fb::asVec4V(NewMax.x, NewMin.y, NewMax.z,NewMin.w); //0x00A0  NewMax.x | NewMin.y | NewMax.z | NewMin.w
    this->m_info.vertices[6] = fb::asVec4V(NewMax.x, NewMax.y, NewMin.z,NewMin.w); //0x00B0  NewMax.x | NewMax.y | NewMin.z | NewMin.w
    this->m_info.vertices[7] = NewMax ;//0x00C0                                              NewMax.x | NewMax.y | NewMax.z | NewMin.w
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
    this->Enter();
    for (int i = 0; i < ARRAYSIZE(this->m_querys); i++)
    {
        CustomOcclusionQueryManager::OcclusionQuery* pOcclusionQuery = this->m_querys[i];
        if (!ValidPointer(pOcclusionQuery)) continue;
        if ( entity != NULL && pOcclusionQuery->m_entity == entity)
        {
            this->Leave();
            return true;
        }
    }
    this->Leave();
    return false;
}

CustomOcclusionQueryManager::OcclusionQuery* CustomOcclusionQueryManager::GetQuery( void* entity )
{
    this->Enter();
    for (int i = 0; i < ARRAYSIZE(this->m_querys); i++)
    {
        CustomOcclusionQueryManager::OcclusionQuery* pOcclusionQuery = this->m_querys[i];
        if (!ValidPointer(pOcclusionQuery)) continue;
        if ( entity != NULL && pOcclusionQuery->m_entity == entity)
        {
            this->Leave();
            return pOcclusionQuery;
        }
    }
    this->Leave();
    return NULL;
}

CustomOcclusionQueryManager::OcclusionQuery* CustomOcclusionQueryManager::AddQuery( void* entity, fb::WorldOcclusionQueryRenderModule::ObjectRenderInfo* info )
{
    CustomOcclusionQueryManager::OcclusionQuery* oldQuery = this->GetQuery( entity );
    if ( oldQuery != NULL )
        return oldQuery;
    this->Enter();
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
    //hook the entity destructor:
    DWORD64* vtable = *(DWORD64**)entity;
    
    DWORD64* newVtable = (DWORD64*)VirtualAlloc( NULL, 0x1000, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE );
    memcpy( newVtable, vtable, 0x1000 );
    *(DWORD64**)entity = vtable;
    //HookVTableFunction function code: https://github.com/Speedi13/BFV-SimpleCheat/blob/master/BFV-SimpleCheat/Util.cpp#L5
    void* oriFnc = HookVTableFunction( (DWORD64**)entity, (BYTE*)&fb__ClientVehicleEntity__hkDestructor, 10 );
    if ( fb__ClientVehicleEntity__Destructor == NULL )
         fb__ClientVehicleEntity__Destructor = (t_fb__ClientVehicleEntity__Destructor)oriFnc;
    this->Leave();
    return pQuery;
}

bool CustomOcclusionQueryManager::RemoveQuery( void* entity )
{
    this->Enter();
    CustomOcclusionQueryManager::OcclusionQuery* pQuery = this->GetQuery( entity );
    if (ValidPointer(pQuery))
    {
        for (int i = 0; i < ARRAYSIZE(this->m_querys); i++)
        {
            CustomOcclusionQueryManager::OcclusionQuery* pOcclusionQuery = this->m_querys[i];
            if (!ValidPointer(pOcclusionQuery)) continue;
            if ( pOcclusionQuery->m_entity == entity )
            {
                free( this->m_querys[i] );
                this->m_querys[i] = NULL;
                this->Leave();
                return true;
            }
        }
    }
    this->Leave();
    return false;
}

void CustomOcclusionQueryManager::InitCriticalSection()
{
    InitializeCriticalSection(&this->m_CriticalSection);
}
void CustomOcclusionQueryManager::Enter()
{
    EnterCriticalSection(&this->m_CriticalSection);
}
void CustomOcclusionQueryManager::Leave()
{
    LeaveCriticalSection(&this->m_CriticalSection);
}

void __fastcall fb__WorldOcclusionQueryRenderModule__retrieveBatchQueryData( fb::WorldOcclusionQueryRenderModule *_this, fb::WorldOcclusionQueryRenderModule::BatchQuery* BatchQueries, float flScreenArea)
{
    fb::DxRenderer* DxRenderer = fb::DxRenderer::GetInstance();

    bool (__fastcall* fb__DxRenderer__getQueryData)( fb::DxRenderer*, fb::DxRenderQuery*, unsigned int RenderQueryIndex, void *outData, unsigned int dataSize );
    fb__DxRenderer__getQueryData = *( decltype(fb__DxRenderer__getQueryData) *)( *(DWORD_PTR*)DxRenderer + (DWORD_PTR)( 45 * sizeof(void*) ) );

    int TotalProcessedQueries = 0;
    unsigned int RenderQueryIndex = 0;

    UINT64 OutData = 0;
    if ( BatchQueries->count )
    {
        do
        {
            if ( 
                fb__DxRenderer__getQueryData(
                                                DxRenderer,
                                                BatchQueries->m_dxQueries,
                                                RenderQueryIndex,
                                                &OutData,
                                                sizeof(OutData)
                                            )
                )
            {
                float v7 = (float)( OutData );
                ++TotalProcessedQueries;
                BatchQueries->outObjectScreenAreaCoverage[ RenderQueryIndex ] = min( v7 / flScreenArea, (float)1.0 );
            }
            ++RenderQueryIndex;
        }
        while ( RenderQueryIndex < BatchQueries->count );
    }
    if ( TotalProcessedQueries == BatchQueries->count )
    {
        BatchQueries->status = fb::WorldOcclusionQueryRenderModule::BatchQuery::QS_Done;
    }
    else
    {
        //lol wtf dice:
        BatchQueries->status = BatchQueries->status;
    }
}
