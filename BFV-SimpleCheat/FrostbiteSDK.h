#pragma once
#include <Windows.h>

//for latest offsets check the last page of this thread:
//https://www.unknowncheats.me/forum/battlefield-v/288137-battlefield-reversing-struct-offsets-thread.html

#define OFFSET_CLIENTGAMECONTEXT 0x144A54220
#define OFFSET_DXRENDERER 0x144B00CE0
#define OFFSET_GAMERENDERER 0x144B00EB0
extern void* OFFSET_ObfuscationMgr;

#define OFFSET_DEBUGRENDERER     0x145C59270 //48 8B 0D ?? ?? ?? ?? 48 85 C9 75 03 31 C0 C3 E9]
#define OFFSET_DrawText          0x14042C7A0 //4C 8B C8 44 8B C6 BA 14 00 00 00 49 8B CC E8] + 15
#define OFFSET_DrawLine          0x145C5C9C0 //49 8B CC E8 ?? ?? ?? ?? FF ?? 8B 0D] + 4
#define OFFSET_DrawRect2D        0x145C5CF30 //C7 44 24 3C 00 00 F0 41 41 83 C9 FF 4C 8D] + 25

#define ValidPointer( pointer ) ( pointer != NULL && (DWORD_PTR)pointer >= 0x10000 && (DWORD_PTR)pointer < 0x00007FFFFFFEFFFF /*&& some other checks*/ )

extern DWORD OFFSET_Soldier;
extern DWORD OFFSET_Vehicle;
#define OFFSET_BoneCollisionComponent 0x0670
#define OFFSET_HealthComponent 0x278
#define OFFSET_occluded 0xA13
#define OFFSET_PredictedController 0x800 //https://github.com/Speedi13/BFV-Decryption/blob/master/BfvDecryptionDemo/DllMain.cpp#L22
#define OFFSET_Player_TeamId 0x1C48

#define D3DX_PI    (3.14159265358979323846)
#define D3DX_1BYPI ( 1.0 / D3DX_PI )

#define D3DXToRadian( degree ) ((degree) * (D3DX_PI / 180.0))
#define D3DXToDegree( radian ) ((radian) * (180.0 / D3DX_PI))

#define ResolveRelativePtr(Address) ((ULONG_PTR)(Address) + *(__int32*)(Address) + sizeof(__int32))

void* DecryptPointer( DWORD64 EncryptedPtr, DWORD64 PointerKey );

typedef unsigned __int64	uint64_t;
typedef unsigned int		uint32_t;
typedef unsigned __int16	uint16_t;
typedef unsigned __int8		uint8_t;

typedef DWORD64 QWORD;

typedef QWORD _QWORD;
typedef BYTE _BYTE;
typedef DWORD _DWORD;
typedef WORD _WORD;

#define TRY __try{
#define TRY_END }__except(1){;};

namespace fb
{	
	template <class T, INT Count, INT PadSize>
	class fixed_vector
	{
	public:
		T* m_firstElement;
		T* m_lastElement;
		T* m_arrayBound;
		LPVOID m_pad[PadSize];
		T m_data[Count];

	public:
		fixed_vector() {
			m_firstElement = (T *)m_data;
			m_lastElement = (T *)m_data;
			m_arrayBound = (T *)&m_data[Count];
		}

		void push_back(T *const value) {
			if (m_lastElement > m_arrayBound) {

				return;
			}
			*m_lastElement = *value;
			m_lastElement = m_lastElement + 1;
		};

		void clear() {
			m_firstElement = m_data;
			m_lastElement = m_data;
			m_arrayBound = (T *)&m_data[Count];
		}

		UINT Size() {
			return ((DWORD_PTR)m_lastElement - (DWORD_PTR)m_firstElement) / sizeof(T);
		}

		T At(INT nIndex) {
			return m_firstElement[nIndex];
		}

		T operator [](INT index) { return At(index); }
	};

	template <class T>
	class vector
	{
	private:
		T* m_firstElement;
		T* m_lastElement;
		T* m_arrayBound;
		void* vftable;
	public:
		size_t size()
		{
			return (((intptr_t)this->m_lastElement - (intptr_t)this->m_firstElement) / sizeof(T));
		}
		T at(int nIndex)
		{
			return *(T*)((intptr_t)m_firstElement + (nIndex * sizeof(T)));
		}
		T operator [](int index)
		{
			return at(index);
		}
	};
	template <typename T>
	class Array
	{
	private:
		T* m_firstElement;

	public:
		T At(INT nIndex)
		{
			if(!ValidPointer(m_firstElement))
				return (T)NULL;

			return *(T*)((DWORD64)m_firstElement + (nIndex * sizeof(T)));
		};

		T operator [](INT index) { return m_firstElement[index]; }

		int GetSize()
        {
            if (m_firstElement)
            {
                return *(int*)((DWORD_PTR)m_firstElement - sizeof(__int32));
            }
            return 0;
        }
	};
	
	template <typename T>
	class Tuple2
	{
	public:
		T Element1;
		T Element2;

	public:
		Tuple2(T _Element1, T _Element2)
		{
			Element1=_Element1;
			Element2=_Element2;
		}
	};
	template<typename T1, typename T2>
	struct pair
	{
		T1 first;
		T2 second;
	};
	template<typename T>
	struct hash_node
	{
		pair<_QWORD,T*> mValue;
		hash_node<T>* mpNext;
	};
 
	template<typename T>
	struct hashtable
	{
		_QWORD vtable;
		hash_node<T>** mpBucketArray;
		unsigned int mnBucketCount;
		unsigned int mnElementCount;
		//... some other stuff
	};
 
	template<typename T>
	struct hashtable_iterator
	{
		hash_node<T>* mpNode;
		hash_node<T>** mpBucket;
	};
	class Color32
	{
	public:

		Color32( DWORD dwColor )
		{
			this->a = (dwColor >> 24) & 0xff; 
			this->r = (dwColor >> 16) & 0xff; 
			this->g = (dwColor >> 8) & 0xff; 
			this->b = (dwColor) & 0xff; 
		}

		Color32( unsigned char red, unsigned char green, unsigned char blue, unsigned char alpha ) :
		r( red ), g( green ), b( blue ), a( alpha )
		{
		}

		static Color32 Yellow() { return Color32( 255, 255, 000, 255 );	}
		static Color32 White()  { return Color32( 255, 255, 255, 255 ); }
		static Color32 Black()  { return Color32( 0, 0, 0, 255 ); }
		static Color32 Red()    { return Color32( 255, 0, 0, 255 ); }
		static Color32 Green()  { return Color32( 0, 255, 0, 255 ); }
		static Color32 Blue()   { return Color32( 0, 0, 255, 255 ); }
	private:
		union
		{
			struct
			{
				unsigned char r;
				unsigned char g;
				unsigned char b;
				unsigned char a;
			};

			unsigned long color;
		};
	};
	template< class T >
	struct WeakToken
	{
		T* m_realptr; //its value is: realptr + 8
		DWORD32 m_refcount;
	};
	template< class T > 
	class WeakPtr
	{
	private:
		WeakToken<T>* m_ptr;
	public:
		T* GetData()
		{
			if (!ValidPointer( m_ptr ))
    				return NULL;

			if (!ValidPointer( &m_ptr->m_realptr ))
    				return NULL;
     
			T* ptr = m_ptr->m_realptr;
			if (!ValidPointer(ptr) )
    				return NULL;
     
			return (T*)((DWORD_PTR)(ptr) - 0x8);
		}
	};

	template< class T >
	class EncryptedPtr
	{
	public:
		DWORD64 m_encryptedPtr;
		DWORD64 m_pointerKey;

	public:
		T* GetPtr( )
		{
			return (T*)( DecryptPointer( this->m_encryptedPtr, (DWORD64)(this->m_pointerKey) ) );
		}
	};

	struct Vec2 { union {float v[2]; struct {float x;float y;}; }; };
	struct Vec3 { union {float v[3]; struct {float x;float y;float z;}; }; };
	struct Vec4 { public: union {float v[4]; struct {float x;float y;float z;float w;}; }; };
	static Vec4 asVec4V(float x,float y,float z,float w){ Vec4 out; out.x = x; out.y = y; out.z = z; out.w = w; return out; }
	struct Matrix4x4 { union {Vec4 v[4]; float m[4][4]; struct {Vec4 right;Vec4 up;Vec4 forward;Vec4 trans;}; }; };
	typedef Matrix4x4 LinearTransform;

	struct AxisAlignedBox
	{
		Vec4 min;
		Vec4 max;
	};

	struct LinearTransform_AABB
	{
	public:
		LinearTransform m_Transform;
		AxisAlignedBox m_Box;
		char _pad[0x200];
	};

	class RenderView
	{
	public:
		char _0x0000[312];
		float m_FovY; //0x0138 
		float defaultFovY; //0x013C 
		char _0x0140[48];
		DWORD m_dirtyFlags; //0x0170 
		char _0x0174[364];
		float m_FovX; //0x02E0 
		float m_depthToWidthRatio; //0x02E4 
		float m_fovScale; //0x02E8 
		float m_fovScaleSqr; //0x02EC 
		LinearTransform m_viewMatrix; //0x02F0 
		LinearTransform m_viewMatrixTranspose; //0x0330 
		LinearTransform m_viewMatrixInverse; //0x0370 
		LinearTransform m_ProjectionMatrix; //0x03B0 
		LinearTransform N09CECBCA; //0x03F0 
		LinearTransform N09CECBD2; //0x0430 
		LinearTransform N09CF2713; //0x0470 
		LinearTransform N05AA0ABA; //0x04B0 
		LinearTransform m_ViewProj; //0x04F0 
	};

	class GameRenderer
	{
	public:
		char _0x0000[72];
		class GameRenderSettings* m_GameRenderSettings; //0x0048 
		char _0x0050[16];
		RenderView* m_pRenderView;        // 0x60

		static GameRenderer* GetInstance()
		{
			GameRenderer** ppGameRenderer = (GameRenderer**)(OFFSET_GAMERENDERER);
			if (!ValidPointer(ppGameRenderer)) return NULL;

			return *(GameRenderer**)ppGameRenderer;
		}
	};

	struct RenderScreenInfo
	{
		unsigned int width; //0x0000 
		unsigned int height; //0x0004 
		unsigned int windowWidth; //0x0008 
		unsigned int windowHeight; //0x000C 
	};//Size=0x0010

	class SwapChainClass
	{
	public:
		char _0x0000[16];
		class IDXGISwapChain* m_pSwapChain; //0x0010 
	};//Size=0x0018

	class Screen
	{
	public:
		char _0x0000[0x68];
		RenderScreenInfo m_ScreenInfo; //0x0068 
		char _0x0078[0x288];
		SwapChainClass* m_ptrSwapChain; //0x0300
		char _0x0308[0x30];
		class Dx11RenderTargetView * m_pDefaultRenderView; //0x0338 [=> https://www.unknowncheats.me/forum/1817135-post1239.html ]
	};
	
	enum RendererType
	{
		RendererType_Dx9,	//=> 0
		RendererType_Dx11,	//=> 1
		RendererType_Dx12,	//=> 2
		RendererType_Gl,	//=> 3
		RendererType_Orbis,	//=> 4
		RendererType_Mantle,//=> 5
		RendererType_Balsa,	//=> 6
		RendererType_Null	//=> 7
	};
	class DxRenderer
	{
	public:
		virtual void Function0( );
		virtual RendererType fb::DxRenderer::getRendererType();
		virtual void Function2(); //
		virtual void Function3(); //
		virtual void Function4(); //
		virtual void Function5(); //
		virtual void Function6(); //
		virtual void Function7(); //
		virtual void beginFrame(); // fb::DxRenderer::beginFrame
		virtual void endFrame(); // fb::DxRenderer::endFrame
		virtual void acquireThreadOwnership(); //
		virtual void releaseThreadOwnership(); //
		virtual void getAdapterInfo(); //
		virtual unsigned int GetNumberOfScreens(); //
		virtual RenderScreenInfo* getScreenInfo( unsigned int ScreenNbr ); //fb::DxRenderer::getScreenInfo;
		Screen* GetScreenByNumber( unsigned int ScreenNbr ){ return  (Screen*)( (DWORD_PTR)( (DWORD_PTR)this->getScreenInfo( ScreenNbr ) - (DWORD_PTR)0x68 ) ); }
		virtual void Function15(); //
		virtual void Function16(); //
		virtual void Function17(); //
		virtual void Function18(); //
		virtual void Function19(); //
		virtual void Function20(); //
		virtual void Function21(); //
		virtual void Function22(); //
		virtual void getFrameCounter(); //
		virtual void Function24(); //
		virtual void Function25(); //
		virtual void Function26(); //
		virtual void Function27(); //
		virtual void Function28(); //
		virtual void Function29(); //
		virtual class DxRenderQuery * createQuery(void* Allocator, enum RenderQueryType type, uint32_t size, char* name); // [fb::DxRenderer::createQuery] [ 148F50960 ]
		virtual void Function31(); //
		virtual void Function32(); //
		virtual void Function33(); //
		virtual void Function34(); //
		virtual void Function35(); //
		virtual void Function36(); //
		virtual void Function37(); //
		virtual void Function38(); //
		virtual void Function39(); //
		virtual void Function40(); //
		virtual void Function41(); //
		virtual void Function42(); //
		virtual void Function43(); //
		virtual void Function44(); //
		virtual bool getQueryData( class DxRenderQuery* pDxRenderQuery, unsigned int RenderQueryIndex, __int64 OutData, unsigned int OutDataSize); // [fb::DxRenderer::getQueryData] [ 141BD9840 ]
		virtual void Function46(); //
		virtual void Function47(); //
		virtual void Function48(); //
		virtual void Function49(); //
		virtual void Function50(); //
		virtual bool Function51(); //
		virtual void Function52(); //
		virtual class DxDisplaySettings* getDxDisplaySettings(); //
		virtual void Function54(); //
		virtual void Function55(); //
		virtual void Function56(); //
		virtual void Function57(); //
		virtual void Function58(); //
		virtual void Function59(); //
		virtual void Function60(); //
		virtual void Function61(); //
		virtual void Function62(); //
		virtual void Function63(); //
		virtual void Function64(); //
		virtual void Function65(); //
		virtual void Function66(); //
		virtual void Function67(); //
		virtual void Function68(); //
		virtual void createTexture(); // fb::DxRenderer::createTexture


		static DxRenderer* GetInstance( )
		{
			DxRenderer** ppDxRenderer = reinterpret_cast<DxRenderer**>( OFFSET_DXRENDERER );
			if (!ValidPointer(ppDxRenderer)) return NULL;

			return *(DxRenderer**)(ppDxRenderer);
		}
	};

	enum RenderQueryType
	{
		RenderQueryType_Occlusion  = 2
	};

	class DxRenderQuery
	{
	public:
		void* vtable; //0x0000
		DWORD dword8; //0x0008
		DWORD dwordC; //0x000C
		QWORD qword10; //0x0010 pointer to the allocator i guess
		RenderQueryType m_type; //0x0018
		int m_dwQueryCount; //0x001C
		class ID3D11Query** m_queries; //0x0020
	};

	class WorldOcclusionQueryRenderModule
	{
	public:
		class ObjectRenderInfo
		{
		public:
			LinearTransform worldTransform;
			Vec4 vertices[8]; //bounding box corners
		};
		class BatchQuery
		{
		public:
			enum QueryStatus
			{
				QS_Init,				// constant 0x0
				QS_WaitingForResult,	// constant 0x1
				QS_Done,				// constant 0x2
				QS_Inactive,			// constant 0x3
			}; // QueryStatus

			unsigned int count; //0x0000
			enum QueryStatus status; //0x0004
			bool queriesInitialized; //0x0008
			char _0x0009[3];
			DWORD dwordC; //0x000C
			float* outObjectScreenAreaCoverage; //0x0010
			DWORD doneCounter; //0x0018
			DWORD DWORD1C; //0x001C
			fixed_vector< ObjectRenderInfo, 32, 1> renderInfos; //0x0020
			DxRenderQuery* m_dxQuerys; //0x1840
			char _0x01848[8];
		};// Size = 0x1850
	
		_QWORD m_callback__mpBegin;	 //00000000 //vector
		_QWORD m_callback__mpEnd;	 //00000008
		_QWORD m_callback__mpCapacity;//00000010
		_QWORD m_callback__mAllocator;//00000018
		char _0x0020[16]; //0x0020
		CRITICAL_SECTION m_ciricalSection; //0x0030
		char _0x0058[8];//0x0058
		void* m_Allocator;//0x0060
		char _0x0068[8];//0x0068
		_QWORD m_usedbatches__mpBegin;	 //00000070 //vector
		_QWORD m_usedbatches__mpEnd;	 //00000078
		_QWORD m_usedbatches__mpCapacity;//00000080
		_QWORD m_usedbatches__mAllocator;//00000088
		_QWORD m_freeBatchesIndices__mpBegin;	//00000090 //vector
		_QWORD m_freeBatchesIndices__mpEnd;		//00000098
		_QWORD m_freeBatchesIndices__mCapacity;	//000000A0
		_QWORD m_freeBatchesIndices__mAllocator;//000000A8

		WorldOcclusionQueryRenderModule::BatchQuery* GetBatchById( unsigned int handle )
		{
			WorldOcclusionQueryRenderModule::BatchQuery* usedbatches = (WorldOcclusionQueryRenderModule::BatchQuery*)this->m_usedbatches__mpBegin;
			return &usedbatches[handle];
		}
	};
	class UpdatePoseResultData
	{
	public:
		class QuatTransform
		{
		public:
			Vec4 m_TransAndScale; //0x0000 
			Vec4 m_Rotation; //0x0010 
		};//Size=0x0020

		char _0x0000[0x20];

		class UpdatePoseResultData::QuatTransform* m_ActiveWorldTransforms; //0x0020 
		class UpdatePoseResultData::QuatTransform* m_ActiveLocalTransforms; //0x0028
		int m_Slot; //0x0030 
		int m_ReaderIndex; //0x0034 
		bool m_ValidTransforms; //0x0038 
		bool m_PoseUpdateEnabled; //0x0039 
		bool m_PoseNeeded; //0x003A 
	};
	class BoneCollisionComponent
	{
	public:
		UpdatePoseResultData m_ragdollTransforms; //0x0000
	};
	class ClientSoldierPrediction
	{
	public:
		void* vtable;
		void* m_characterPhyEntity; //0x0008 
		char _0x0010[128];
		Vec4 m_Position; //0x0090 
	};
	
	class HealthComponent
	{
	public:
		virtual class TypeInfo* GetType();
		virtual void Function1(); //
		virtual void Function2(); //
		virtual void Function3(); //
		virtual void Function4(); //
		virtual void Function5(); //
		virtual void Function6(); //
		virtual void Function7(); //
		virtual void Function8(); //
		virtual void Function9(); //
		virtual void Function10(); //
		virtual void Function11(); //
		virtual void Function12(); //
		virtual void Function13(); //
		virtual void Function14(); //
		virtual void Function15(); //
		virtual void Function16(); //
		virtual void Function17(); //
		virtual void Function18(); //
		virtual void Function19(); //
		virtual void Function20(); //
		virtual void Function21(); //
		virtual void Function22(); //
		virtual void Function23(); //
		virtual void Function24(); //
		virtual void Function25(); //
		virtual void Function26(); //
		virtual void Function27(); //
		virtual void Function28(); //
		virtual void Function29(); //
		virtual void Function30(); //
		virtual void Function31(); //
		virtual void Function32(); //
		virtual void Function33(); //
		virtual void Function34(); //
		virtual void Function35(); //
		virtual bool isAlive();

		char _0x0008[24];
		float m_Health; //0x0020 
		float m_MaxHealth; //0x0024 
		char _0x0028[24];
		float m_vehicleHealth; //0x0040 
	};
	class ClientSoldierEntity
	{
	public:
		virtual class TypeInfo* GetType();
		virtual void Function1(); //
		virtual void Function2(); //
		virtual void Function3(); //
		virtual void Function4(); //
		virtual void Function5(); //
		virtual void Function6(); //
		virtual void Function7(); //
		virtual void Function8(); //
		virtual void Function9(); //
		virtual void Function10(); //
		virtual void Function11(); //
		virtual void Function12(); //
		virtual void Function13(); //
		virtual void Function14(); //
		virtual void Function15(); //
		virtual void Function16(); //
		virtual void Function17(); //
		virtual void Function18(); //
		virtual void Function19(); //
		virtual void Function20(); //
		virtual void Function21(); //
		virtual void Function22(); //
		virtual void Function23(); //
		virtual void Function24(); //
		virtual void Function25(); //
		virtual void GetTransformAABB(LinearTransform_AABB& mTransform);// 26
		virtual void Function27(); //
		virtual void Function28(); //
		virtual void Function29(); //
		virtual void Function30(); //
		virtual void Function31(); //
		virtual void Function32(); //
		virtual void Function33(); //
		virtual void Function34(); //
		virtual void Function35(); //
		virtual void Function36(); //
		virtual void Function37(); //
		virtual void Function38(); //
		virtual void Function39(); //
		virtual void Function40(); //
		virtual void Function41(); //
		virtual void Function42(); //
		virtual void Function43(); //
		virtual void Function44(); //
		virtual void Function45(); //
		virtual void Function46(); //
		virtual void Function47(); //
		virtual void Function48(); //
		virtual void Function49(); //
		virtual void Function50(); //
		virtual void Function51(); //
		virtual void Function52(); //
		virtual void Function53(); //
		virtual void Function54(); //
		virtual void Function55(); //
		virtual void Function56(); //
		virtual void Function57(); //
		virtual void Function58(); //
		virtual void Function59(); //
		virtual void Function60(); //
		virtual void Function61(); //
		virtual fb::Vec4* GetVelocity( fb::Vec4 & ); // fb::ClientSoldierEntity::GetVelocity

		//https://github.com/Speedi13/BFV-Decryption/blob/master/BfvDecryptionDemo/DllMain.cpp#L22
		EncryptedPtr<ClientSoldierPrediction> GetPredictedController()
		{
			return *(EncryptedPtr<ClientSoldierPrediction>*)( (DWORD_PTR)this + OFFSET_PredictedController );
		};

		void GetTransform( LinearTransform* OutMatrix )
		{
			DWORD_PTR m_collection = *(DWORD_PTR *)( (DWORD_PTR)this  + 0x40);
			unsigned __int8 _9 = *(unsigned __int8 *)(m_collection + 9);
			unsigned __int8 _10 = *(unsigned __int8 *)(m_collection + 10);

			DWORD_PTR ComponentCollectionOffset = 0x20 * (_10 + (2 * _9) );

			*(LinearTransform *)OutMatrix = *(LinearTransform *)(m_collection + ComponentCollectionOffset + 0x10);
		}

		BoneCollisionComponent* GetBoneCollisionComponent()
		{
			return *(BoneCollisionComponent**)( (BYTE*)this + OFFSET_BoneCollisionComponent );
		}
		
		HealthComponent* GetHealthComponent()
		{
			return *(HealthComponent**)( (BYTE*)this + OFFSET_HealthComponent );
		};

		bool IsOccluded()
		{
			return *(bool*)( (BYTE*)this + OFFSET_occluded );
		}

		bool IsVisible()
		{
			return this->IsOccluded( ) == false;
		}
		bool GetBonePos( int BoneId, fb::Vec4 *vOut );
	};
	class VehicleEntityData;
	class ClientVehicleEntity
	{
	public:
		virtual class TypeInfo* GetType();
		virtual void Function1(); //
		virtual void Function2(); //
		virtual void Function3(); //
		virtual void Function4(); //
		virtual void Function5(); //
		virtual void Function6(); //
		virtual void Function7(); //
		virtual void Function8(); //
		virtual void Function9(); //
		virtual void Function10(); //
		virtual void Function11(); //
		virtual void Function12(); //
		virtual void Function13(); //
		virtual void Function14(); //
		virtual void Function15(); //
		virtual void Function16(); //
		virtual void Function17(); //
		virtual void Function18(); //
		virtual void Function19(); //
		virtual void Function20(); //
		virtual void Function21(); //
		virtual void Function22(); //
		virtual void Function23(); //
		virtual void Function24(); //
		virtual void Function25(); //
		virtual void GetTransformAABB(LinearTransform_AABB& mTransform);// 26

		HealthComponent* GetHealthComponent()
		{
			return *(HealthComponent**)( (BYTE*)this + OFFSET_HealthComponent );
		};

		VehicleEntityData* GetEntityData()
		{
			return *(VehicleEntityData**)( (BYTE*)this + 0x38 );
		};
	};
	class ClientPlayer
	{
	public:
		virtual ~ClientPlayer();
		virtual DWORD_PTR GetCharacterEntity(); //=> ClientSoldierEntity + 0x268 
		virtual DWORD_PTR GetCharacterUserData(); //=> PlayerCharacterUserData
		virtual class EntryComponent* GetEntryComponent();
		virtual bool InVehicle();
		virtual unsigned int getId();

		ClientSoldierEntity* GetSoldier();
		ClientVehicleEntity* GetVehicle();

		char _0x0008[16];
		char* m_pName; //0x0018 
		char _0x0020[32];
		char szName[16]; //0x0040 

		int GetTeamId()
		{
			return *(int*)( (BYTE*)this + OFFSET_Player_TeamId );
		}

	};

	class WorldViewDesc
	{
	public:
		char _0x0000[32];
		RenderScreenInfo viewport; //0x0020
	};//Size=0x0030

	class WorldRenderer
	{
	public:
		char _0x0000[2248+0x18];
		WorldOcclusionQueryRenderModule* m_WorldOcclusionQueriesRenderModule; //0x08E0 
		char _0x08D0[2096-0x18];

		class /*fb::WorldRenderer::*/RootView
		{
		public:
			char _0x0000[3232+0x10];
			WorldViewDesc m_rootView; //0x0CB0
		};

		RootView m_rootviews; //0x1100 
	};

	class WorldRenderModule
	{
	public:
		char _0x0000[64];
		WorldRenderer* m_worldRenderer; //0x0040
	};

	class ClientLevel
	{
	public:
		char _0x0000[24];
		class LevelData* m_pLevelData; //0x0018 
		class TeamInfo* m_pTeamInfo; //0x0020 
		char* m_LevelName; //0x0028 
		char _0x0030[208];
		WorldRenderModule* m_worldRenderModule; //0x0100
		char _0x0108[8];
		class HavokPhysicsManager* m_pPhysicsManager; //0x0110
		char _0x0118[24];
		class ClientGameWorld* m_pGameWorld; //0x0130
	};

	class ClientGameContext
	{
	public:
		char _0x0000[0x20];
		class ClientPlayerManager* m_playerManager; //0x0020 
		class GameTime* m_pGameTime; //0x0028 
		class ClientLevel* m_pClientLevel; //0x0030 
		char _0x0028[0x30];
		class ClientPlayerManager* m_clientPlayerManager; //0x0068 

		static ClientGameContext* GetInstance()
		{
			return *(ClientGameContext**)(OFFSET_CLIENTGAMECONTEXT);
		}
	};
#if defined( OFFSET_DEBUGRENDERER )
	class DebugRenderer2
	{
	public:
		static DebugRenderer2* Singleton(void) 
		{
			typedef DebugRenderer2* (__thiscall* fb__DebugRenderManager_getThreadContext_t)(void);
			fb__DebugRenderManager_getThreadContext_t fb__DebugRenderManager_getThreadContext=(fb__DebugRenderManager_getThreadContext_t)OFFSET_DEBUGRENDERER;
			return fb__DebugRenderManager_getThreadContext();
		}
#if defined( OFFSET_DrawText )
		void drawText(int x, int y, Color32 color, char* text, float scale)
		{
			typedef void (__thiscall *tdrawText)(DebugRenderer2*,int, int, char*, Color32,float);
			tdrawText mdrawText=(tdrawText)OFFSET_DrawText;
			mdrawText(this,x,y,text,color,scale);
		}
#endif
#if defined( OFFSET_DrawLine )
		void DrawLine( float x1, float y1, float x2, float y2, Color32 color )
		{
			Tuple2< float > tupFrom =Tuple2< float >( x1, y1 );
			Tuple2< float > tupTo = Tuple2< float >( x2, y2 );

			typedef void ( __thiscall* tDrawLine )( DebugRenderer2*, Tuple2< float >*, Tuple2< float >*, Color32 );
			tDrawLine oDrawLine = ( tDrawLine )OFFSET_DrawLine;

			oDrawLine( this, &tupFrom, &tupTo, color );
		}

		void DrawBox2D(float x1, float y1, float w, float h, Color32 color)
		{
			DrawLine(x1,y1,x1+w,y1,color);
			DrawLine(x1+w,y1,x1+w,y1+h,color);
			DrawLine(x1+w,y1+h,x1,y1+h,color);
			DrawLine(x1,y1+h,x1,y1,color);
		}

		void DrawVec2Line(Vec2 start, Vec2 end,Color32 color)
		{
			DrawLine(start.x,start.y,end.x ,end.y,color);
		}
		void DrawVecLine(Vec4 start, Vec4 end,Color32 color)
		{
			DrawLine(start.x,start.y,end.x ,end.y,color);
		}
#endif
#if defined( OFFSET_DrawRect2D )
		void DrawRect( float x, float y, float width, float height, Color32 color )
		{
			Tuple2< float > tupMin = Tuple2< float >( x, y );
			Tuple2< float > tupMax = Tuple2< float >( x + width, y + height );

			typedef void ( __thiscall* tDrawRectangle )( DebugRenderer2*, Tuple2< float >*, Tuple2< float >*, Color32 );
			tDrawRectangle oDrawRectangle = ( tDrawRectangle )OFFSET_DrawRect2D;

			oDrawRectangle( this, &tupMin, &tupMax, color );
		}
#endif
	};
#endif

	//[Characters/skeletons/3P_MaleSoldier_FB]
	enum BoneIds
	{
		BONE_Reference = 0x0,
		BONE_AITrajectory = 0x1,
		BONE_Hips = 0x2,
		BONE_Spine = 0x3,
		BONE_Spine1 = 0x4,
		BONE_Spine2 = 0x5,
		BONE_Neck = 0x6,
		BONE_Neck1 = 0x7,
		BONE_Head = 0x8,
		BONE_HeadEnd = 0x9,
		BONE_FACIAL_C_FacialRoot = 0xA,
		BONE_FACIAL_LOD1_C_Forehead = 0xB,
		BONE_FACIAL_LOD1_L_ForeheadIn = 0xC,
		BONE_FACIAL_LOD1_R_ForeheadIn = 0xD,
		BONE_FACIAL_LOD1_L_ForeheadMid = 0xE,
		BONE_FACIAL_LOD1_R_ForeheadMid = 0xF,
		BONE_FACIAL_LOD1_L_ForeheadOut = 0x10,
		BONE_FACIAL_LOD1_R_ForeheadOut = 0x11,
		BONE_FACIAL_LOD1_L_EyesackUpper = 0x12,
		BONE_FACIAL_LOD1_R_EyesackUpper = 0x13,
		BONE_FACIAL_LOD1_L_EyelidUpperFurrow = 0x14,
		BONE_FACIAL_LOD1_R_EyelidUpperFurrow = 0x15,
		BONE_FACIAL_LOD1_L_EyelidUpper = 0x16,
		BONE_FACIAL_LOD1_R_EyelidUpper = 0x17,
		BONE_FACIAL_LOD1_L_Eyeball = 0x18,
		BONE_FACIAL_LOD1_L_Pupil = 0x19,
		BONE_FACIAL_LOD1_R_Eyeball = 0x1A,
		BONE_FACIAL_LOD1_R_Pupil = 0x1B,
		BONE_FACIAL_LOD1_L_EyelidLower = 0x1C,
		BONE_FACIAL_LOD1_R_EyelidLower = 0x1D,
		BONE_FACIAL_LOD1_L_EyesackLower = 0x1E,
		BONE_FACIAL_LOD1_R_EyesackLower = 0x1F,
		BONE_FACIAL_LOD1_L_CheekInner = 0x20,
		BONE_FACIAL_LOD1_R_CheekInner = 0x21,
		BONE_FACIAL_LOD1_L_CheekOuter = 0x22,
		BONE_FACIAL_LOD1_R_CheekOuter = 0x23,
		BONE_FACIAL_LOD1_C_NoseBridge = 0x24,
		BONE_FACIAL_LOD1_L_NasolabialBulge = 0x25,
		BONE_FACIAL_LOD1_R_NasolabialBulge = 0x26,
		BONE_FACIAL_LOD1_L_NasolabialFurrow = 0x27,
		BONE_FACIAL_LOD1_R_NasolabialFurrow = 0x28,
		BONE_FACIAL_LOD1_L_CheekLower = 0x29,
		BONE_FACIAL_LOD1_R_CheekLower = 0x2A,
		BONE_FACIAL_LOD1_L_Ear = 0x2B,
		BONE_FACIAL_LOD1_R_Ear = 0x2C,
		BONE_FACIAL_LOD1_C_Nose = 0x2D,
		BONE_FACIAL_LOD1_C_NoseLower = 0x2E,
		BONE_FACIAL_LOD1_L_Nostril = 0x2F,
		BONE_FACIAL_LOD1_R_Nostril = 0x30,
		BONE_FACIAL_LOD1_C_Mouth = 0x31,
		BONE_FACIAL_LOD1_C_LipUpper = 0x32,
		BONE_FACIAL_LOD1_C_LipUpperInner = 0x33,
		BONE_FACIAL_LOD1_L_LipUpper = 0x34,
		BONE_FACIAL_LOD1_L_LipUpperInner = 0x35,
		BONE_FACIAL_LOD1_R_LipUpper = 0x36,
		BONE_FACIAL_LOD1_R_LipUpperInner = 0x37,
		BONE_FACIAL_LOD1_L_LipUpperOuter = 0x38,
		BONE_FACIAL_LOD1_L_LipUpperOuterInner = 0x39,
		BONE_FACIAL_LOD1_R_LipUpperOuter = 0x3A,
		BONE_FACIAL_LOD1_R_LipUpperOuterInner = 0x3B,
		BONE_FACIAL_LOD1_L_LipCorner = 0x3C,
		BONE_FACIAL_LOD1_L_LipCornerInner = 0x3D,
		BONE_FACIAL_LOD1_R_LipCorner = 0x3E,
		BONE_FACIAL_LOD1_R_LipCornerInner = 0x3F,
		BONE_FACIAL_LOD1_C_LipLower = 0x40,
		BONE_FACIAL_LOD1_C_LipLowerInner = 0x41,
		BONE_FACIAL_LOD1_L_LipLower = 0x42,
		BONE_FACIAL_LOD1_L_LipLowerInner = 0x43,
		BONE_FACIAL_LOD1_R_LipLower = 0x44,
		BONE_FACIAL_LOD1_R_LipLowerInner = 0x45,
		BONE_FACIAL_LOD1_L_LipLowerOuter = 0x46,
		BONE_FACIAL_LOD1_L_LipLowerOuterInner = 0x47,
		BONE_FACIAL_LOD1_R_LipLowerOuter = 0x48,
		BONE_FACIAL_LOD1_R_LipLowerOuterInner = 0x49,
		BONE_FACIAL_LOD1_C_Jaw = 0x4A,
		BONE_FACIAL_LOD1_C_Chin = 0x4B,
		BONE_FACIAL_LOD1_L_ChinSide = 0x4C,
		BONE_FACIAL_LOD1_R_ChinSide = 0x4D,
		BONE_FACIAL_LOD1_C_Tongue1 = 0x4E,
		BONE_FACIAL_LOD1_C_Tongue2 = 0x4F,
		BONE_FACIAL_LOD1_C_Tongue3 = 0x50,
		BONE_FACIAL_LOD1_C_Tongue4 = 0x51,
		BONE_FACIAL_LOD1_L_Masseter = 0x52,
		BONE_FACIAL_LOD1_R_Masseter = 0x53,
		BONE_FACIAL_LOD1_C_UnderChin = 0x54,
		BONE_FACIAL_LOD1_L_UnderChin = 0x55,
		BONE_FACIAL_LOD1_R_UnderChin = 0x56,
		BONE_Head_Prop = 0x57,
		BONE_Head_Phys = 0x58,
		BONE_FACIAL_C_Neck2Root = 0x59,
		BONE_FACIAL_LOD1_C_AdamsApple = 0x5A,
		BONE_HeadRoll = 0x5B,
		BONE_Neck_Phys = 0x5C,
		BONE_SpineX = 0x5D,
		BONE_Wep_Aim = 0x5E,
		BONE_SpineXRight = 0x5F,
		BONE_RightShoulder = 0x60,
		BONE_RightArm = 0x61,
		BONE_RightForeArm = 0x62,
		BONE_RightHand = 0x63,
		BONE_RightHandAttach = 0x64,
		BONE_RightHandIndex0 = 0x65,
		BONE_RightHandIndex1 = 0x66,
		BONE_RightHandIndex2 = 0x67,
		BONE_RightHandIndex3 = 0x68,
		BONE_RightHandIndex4 = 0x69,
		BONE_RightHandMiddle0 = 0x6A,
		BONE_RightHandMiddle1 = 0x6B,
		BONE_RightHandMiddle2 = 0x6C,
		BONE_RightHandMiddle3 = 0x6D,
		BONE_RightHandMiddle4 = 0x6E,
		BONE_RightHandThumb0 = 0x6F,
		BONE_RightHandThumb1 = 0x70,
		BONE_RightHandThumb2 = 0x71,
		BONE_RightHandThumb3 = 0x72,
		BONE_RightHandThumb4 = 0x73,
		BONE_RightHandPinky0 = 0x74,
		BONE_RightHandPinky1 = 0x75,
		BONE_RightHandPinky2 = 0x76,
		BONE_RightHandPinky3 = 0x77,
		BONE_RightHandPinky4 = 0x78,
		BONE_RightHandRing0 = 0x79,
		BONE_RightHandRing1 = 0x7A,
		BONE_RightHandRing2 = 0x7B,
		BONE_RightHandRing3 = 0x7C,
		BONE_RightHandRing4 = 0x7D,
		BONE_RightForeArmRoll = 0x7E,
		BONE_RightForeArmRoll1 = 0x7F,
		BONE_RightForeArmRoll2 = 0x80,
		BONE_RightForeArm_Upper = 0x81,
		BONE_RightForeArm_Lower = 0x82,
		BONE_RightElbowRoll = 0x83,
		BONE_RightArmRoll = 0x84,
		BONE_RightArmRoll1 = 0x85,
		BONE_RightArmBend = 0x86,
		BONE_RightShoulder_Phys = 0x87,
		BONE_SpineXLeft = 0x88,
		BONE_LeftShoulder = 0x89,
		BONE_LeftArm = 0x8A,
		BONE_LeftForeArm = 0x8B,
		BONE_LeftHand = 0x8C,
		BONE_LeftHandAttach = 0x8D,
		BONE_LeftHandIndex0 = 0x8E,
		BONE_LeftHandIndex1 = 0x8F,
		BONE_LeftHandIndex2 = 0x90,
		BONE_LeftHandIndex3 = 0x91,
		BONE_LeftHandIndex4 = 0x92,
		BONE_LeftHandMiddle0 = 0x93,
		BONE_LeftHandMiddle1 = 0x94,
		BONE_LeftHandMiddle2 = 0x95,
		BONE_LeftHandMiddle3 = 0x96,
		BONE_LeftHandMiddle4 = 0x97,
		BONE_LeftHandThumb0 = 0x98,
		BONE_LeftHandThumb1 = 0x99,
		BONE_LeftHandThumb2 = 0x9A,
		BONE_LeftHandThumb3 = 0x9B,
		BONE_LeftHandThumb4 = 0x9C,
		BONE_LeftHandPinky0 = 0x9D,
		BONE_LeftHandPinky1 = 0x9E,
		BONE_LeftHandPinky2 = 0x9F,
		BONE_LeftHandPinky3 = 0xA0,
		BONE_LeftHandPinky4 = 0xA1,
		BONE_LeftHandRing0 = 0xA2,
		BONE_LeftHandRing1 = 0xA3,
		BONE_LeftHandRing2 = 0xA4,
		BONE_LeftHandRing3 = 0xA5,
		BONE_LeftHandRing4 = 0xA6,
		BONE_LeftForeArmRoll = 0xA7,
		BONE_LeftForeArmRoll1 = 0xA8,
		BONE_LeftForeArmRoll2 = 0xA9,
		BONE_LeftForeArm_Upper = 0xAA,
		BONE_LeftForeArm_Lower = 0xAB,
		BONE_LeftElbowRoll = 0xAC,
		BONE_LeftArmRoll = 0xAD,
		BONE_LeftArmRoll1 = 0xAE,
		BONE_LeftArmBend = 0xAF,
		BONE_LeftShoulder_Phys = 0xB0,
		BONE_Wep_SpineX = 0xB1,
		BONE_Wep_Root = 0xB2,
		BONE_Wep_Align = 0xB3,
		BONE_Wep_Trigger = 0xB4,
		BONE_Wep_Slide = 0xB5,
		BONE_Wep_Grenade1 = 0xB6,
		BONE_Wep_Grenade2 = 0xB7,
		BONE_IK_Joint_LeftHand = 0xB8,
		BONE_IK_Joint_RightHand = 0xB9,
		BONE_Wep_Physic1 = 0xBA,
		BONE_Wep_Physic2 = 0xBB,
		BONE_Wep_Physic3 = 0xBC,
		BONE_Wep_Physic4 = 0xBD,
		BONE_Wep_Physic5 = 0xBE,
		BONE_Wep_Extra1 = 0xBF,
		BONE_Wep_Extra2 = 0xC0,
		BONE_Wep_Extra3 = 0xC1,
		BONE_Wep_Extra4 = 0xC2,
		BONE_Wep_Extra5 = 0xC3,
		BONE_Wep_FX = 0xC4,
		BONE_Wep_ButtStock = 0xC5,
		BONE_IK_Dyn_LeftHand = 0xC6,
		BONE_IK_Dyn_RightHand = 0xC7,
		BONE_Wep_Belt1 = 0xC8,
		BONE_Wep_Belt2 = 0xC9,
		BONE_Wep_Belt3 = 0xCA,
		BONE_Wep_Belt4 = 0xCB,
		BONE_Wep_Belt5 = 0xCC,
		BONE_Wep_Belt6 = 0xCD,
		BONE_Wep_Belt7 = 0xCE,
		BONE_Wep_Belt8 = 0xCF,
		BONE_Wep_Belt9 = 0xD0,
		BONE_Wep_Belt10 = 0xD1,
		BONE_Wep_Mag = 0xD2,
		BONE_Wep_Mag_Ammo = 0xD3,
		BONE_Wep_Mag_Extra1 = 0xD4,
		BONE_Wep_Scope1 = 0xD5,
		BONE_Wep_Scope2 = 0xD6,
		BONE_Wep_Bipod1 = 0xD7,
		BONE_Wep_Bipod2 = 0xD8,
		BONE_Wep_Bipod3 = 0xD9,
		BONE_Wep_Belt_Out1 = 0xDA,
		BONE_Wep_Belt_Out2 = 0xDB,
		BONE_Wep_Belt_Out3 = 0xDC,
		BONE_Wep_Belt_Out4 = 0xDD,
		BONE_Wep_Belt_Out5 = 0xDE,
		BONE_Wep_Belt_Out6 = 0xDF,
		BONE_Wep_Belt_Out7 = 0xE0,
		BONE_Wep_Belt_Out8 = 0xE1,
		BONE_Wep_Belt_Out9 = 0xE2,
		BONE_Wep_Belt_Out10 = 0xE3,
		BONE_Wep_Charm1 = 0xE4,
		BONE_Spine2_Phys = 0xE5,
		BONE_RightArmpit = 0xE6,
		BONE_RightDeltoidBulge = 0xE7,
		BONE_LeftArmpit = 0xE8,
		BONE_LeftDeltoidBulge = 0xE9,
		BONE_BackPackAnchor = 0xEA,
		BONE_BackPackRoot = 0xEB,
		BONE_BackPackPhys = 0xEC,
		BONE_Spine1_Phys = 0xED,
		BONE_Spine_Phys = 0xEE,
		BONE_LeftUpLeg = 0xEF,
		BONE_LeftKneeUp = 0xF0,
		BONE_LeftUpLegRoll = 0xF1,
		BONE_LeftUpLegRoll_Phys = 0xF2,
		BONE_LeftKneeRoll = 0xF3,
		BONE_LeftKneeUp_Phys = 0xF4,
		BONE_LeftLeg = 0xF5,
		BONE_LeftKneeLow = 0xF6,
		BONE_LeftKneeLow_Phys = 0xF7,
		BONE_LeftFoot = 0xF8,
		BONE_LeftToeBase = 0xF9,
		BONE_LeftToe = 0xFA,
		BONE_LeftFootAttach = 0xFB,
		BONE_LeftAnkle = 0xFC,
		BONE_LeftHipsRoll = 0xFD,
		BONE_RightUpLeg = 0xFE,
		BONE_RightKneeUp = 0xFF,
		BONE_RightUpLegRoll = 0x100,
		BONE_RightUpLegRoll_Phys = 0x101,
		BONE_RightKneeRoll = 0x102,
		BONE_RightKneeUp_Phys = 0x103,
		BONE_RightLeg = 0x104,
		BONE_RightKneeLow = 0x105,
		BONE_RightKneeLow_Phys = 0x106,
		BONE_RightFoot = 0x107,
		BONE_RightToeBase = 0x108,
		BONE_RightToe = 0x109,
		BONE_RightFootAttach = 0x10A,
		BONE_RightAnkle = 0x10B,
		BONE_RightHipsRoll = 0x10C,
		BONE_Hips_Phys = 0x10D,
		BONE_Trajectory = 0x10E,
		BONE_TrajectoryEnd = 0x10F,
		BONE_CameraBase = 0x110,
		BONE_CameraJoint = 0x111,
		BONE_WepRootOffset = 0x112,
		BONE_Connect = 0x113,
		BONE_ConnectEnd = 0x114,
		BONE_LeftFootPhaseEnd = 0x115,
		BONE_RightFootPhaseEnd = 0x116,
		BONE_RightAnkleEffectorAux = 0x117,
		BONE_LeftAnkleEffectorAux = 0x118,
		BONE_Wep_ProxyRoot = 0x119,
		BONE_Wep_Proxy1 = 0x11A,
		BONE_Wep_Proxy2 = 0x11B,
		BONE_Wep_ProxyExtra1 = 0x11C,
		BONE_Wep_ProxyExtra2 = 0x11D,
		BONE_SpineX_Driver = 0x11E,
		BONE_FacePosesAnimation = 0x11F
	};

	struct VehicleLockableInfoData
	{
		float m_HeatSignature; //0x0000
		float m_RadarSignature; //0x0004
		float m_LowAltitudeHeight; //0x0008
		float m_LowAltitudeHeatSignature; //0x000C
	};//Size=0x0010
	struct VehicleHudData
	{
		LinearTransform m_CustomizationTransform; //0x0000
		class VehicleCustomizationAsset* m_Customization; //0x0040
		enum UIHudIcon m_MinimapIcon; //0x0048
		char _0x004C[4];
		char* m_IconName; //0x0050
		char*  m_CustomInteractionPromptString; //0x0058
		enum MinimapZoomLevelEnum m_MinimapZoomLevel; //0x0060
		char _0x0064[4];
		class TextureAsset* m_Texture; //0x0068
		int m_VehicleItemHash; //0x0070
		float m_DisplayWithinRadius; //0x0074
		bool m_DrawIconsForSelf; //0x0078
		bool m_IsGadgetOverride; //0x0079
		bool m_RotateInMap; //0x007A
		bool m_ShowHealth; //0x007B
		bool m_IsHeavyStationary; //0x007C
		bool m_ImitateSoldierInventory; //0x007D
		bool m_DisableInteractionPrompt; //0x007E
		char _0x007F[1];
	};//Size=0x0080
	struct VehicleHealthZoneData
	{
		float m_MaxHealth; //0x0000
		float m_MaxShieldHealth; //0x0004
		float m_MinDamageAngle; //0x0008
		float m_DamageAngleMultiplier; //0x000C
		class Curve2D* m_DamageAngleMultiplierCurve; //0x0010
		float m_VerticalFactor; //0x0018
		bool m_UseDamageAngleCalculation; //0x001C
		char _0x001D[3];
	};//Size=0x0020
	struct AngleOfImpactData
	{
		float m_Zone12Delimiter; //0x0000
		float m_Zone23Delimiter; //0x0004
		float m_Zone1Multiplier; //0x0008
		float m_Zone2Multiplier; //0x000C
		float m_Zone3Multiplier; //0x0010
		bool m_Enabled; //0x0014
		char _0x0015[3];
	};//Size=0x0018
	class VehicleEntityData
	{
	public:
		char _0x0000[16];
		char* m_Name;
		char _0x0018[8];
		LinearTransform m_Transform; //0x0020
		Array<class GameObjectData*> m_Components; //0x0060
		Array<AxisAlignedBox> m_PartBoundingBoxes; //0x0068
		UINT8 m_ClientRuntimeComponentCount; //0x0070
		UINT8 m_ServerRuntimeComponentCount; //0x0071
		UINT8 m_ClientRuntimeTransformationCount; //0x0072
		UINT8 m_ServerRuntimeTransformationCount; //0x0073
		char _0x0074[12];
		bool m_Enabled; //0x0080
		char _0x0081[15];
		class PhysicsEntityData* m_PhysicsData; //0x0090
		bool m_IsTraversable; //0x0098
		char _0x0099[7];
		enum TeamId m_DefaultTeam; //0x00A0
		float m_LowHealthThreshold; //0x00A4
		int m_MaterialPair; //0x00A8
		char _0x00AC[4];
		Array<int> m_SuppressedInputs; //0x00B0
		bool m_UsePrediction; //0x00B8
		bool m_ShouldChangeTeamWhenPlayerEnters; //0x00B9
		bool m_ResetTeamOnLastPlayerExits; //0x00BA
		bool m_Immortal; //0x00BB
		bool m_FakeImmortal; //0x00BC
		bool m_ForceForegroundRendering; //0x00BD
		bool m_IsNotSuspendable; //0x00BE
		char _0x00BF[1];
		Vec4 m_CriticallyDamagedEffectPosition; //0x00B0
		Vec4 m_PreExplosionEffectPosition; //0x00C0
		Vec4 m_VictimOffsetOverride; //0x00D0
		VehicleHudData m_HudData; //0x00E0
		Vec4 m_DisturbShapeScale; //0x0160
		Vec4 m_FLIRKeyColor; //0x0170
		Vec4 m_IconOffset; //0x0180
		Vec4 m_InteractionOffset; //0x0190
		Vec4 m_StaticBoundingBoxPaddingMin; //0x01A0
		Vec4 m_StaticBoundingBoxPaddingMax; //0x01B0
		Vec4 m_BoneCollisionBoundingBoxPaddingMin; //0x01C0
		Vec4 m_BoneCollisionBoundingBoxPaddingMax; //0x01D0
		Array<class PartLinkData*> m_PartLinks; //0x01E0
		char* m_ControllableType; //0x01E8
		enum CaptureTypeEnum m_CaptureType; //0x01F0
		int m_CapturePlayerLimit; //0x01F4
		float m_DisabledDamageThreshold; //0x01F8
		float m_ClearDisabledDamageThreshold; //0x01FC
		float m_PreDestructionDamageThreshold; //0x0200
		char _0x0204[4];
		VehicleHealthZoneData m_FrontHealthZone; //0x0208
		VehicleHealthZoneData m_RearHealthZone; //0x0228
		VehicleHealthZoneData m_LeftHealthZone; //0x0248
		VehicleHealthZoneData m_RightHealthZone; //0x0268
		VehicleHealthZoneData m_TopHealthZone; //0x0288
		float m_TopHitHeight; //0x02A8
		float m_TopHitAngle; //0x02AC
		float m_RegenerationDelay; //0x02B0
		float m_RegenerationRate; //0x02B4
		float m_ArmorMultiplier; //0x02B8
		float m_RegenerationDelayMultiplier; //0x02BC
		float m_RegenerationRateMultiplier; //0x02C0
		float m_EmergencyRepairHealth; //0x02C4
		float m_DecayDelay; //0x02C8
		float m_DecayRate; //0x02CC
		class EffectBlueprint* m_CriticallyDamagedEffect; //0x02D0
		float m_CriticalDamageTime; //0x02D8
		char _0x02DC[4];
		class EffectBlueprint* m_PreExplosionEffect; //0x02E0
		float m_PreExplosionTime; //0x02E8
		char _0x02EC[4];
		class ExplosionEntityData* m_Explosion; //0x02F0
		class CompositeMeshAsset* m_Mesh; //0x02F8
		CompositeMeshAsset* m_SharedMesh; //0x0300
		class ObjectBlueprint* m_SharedMeshBlueprint; //0x0308
		class ObjectBlueprint* m_CockpitMesh; //0x0310
		char* m_NameSid; //0x0318
		int m_MaxPlayersInVehicle; //0x0320
		float m_MinSpeedForMineActivation; //0x0324
		enum DamageEntityType m_DamageTypeIdentifier; //0x0328
		float m_UpsideDownDamage; //0x032C
		float m_UpsideDownDamageDelay; //0x0330
		float m_UpsideDownAngle; //0x0334
		float m_WaterDamage; //0x0338
		float m_BelowWaterDamageDelay; //0x033C
		float m_WaterDamageOffset; //0x0340
		int m_RepairAnimationType; //0x0344
		float m_VelocityDamageThreshold; //0x0348
		float m_VelocityDamageMagnifier; //0x034C
		float m_RepairRateModifier; //0x0350
		float m_KillSoldierCollisionSpeedThreshold; //0x0354
		float m_ExitSpeedThreshold; //0x0358
		float m_ExitDirectionSpeedThreshold; //0x035C
		enum VehicleSpawnerType m_SpawnerType; //0x0360
		float m_FLIRValue; //0x0364
		enum MPModeData m_MPMode; //0x0368
		float m_ExitCameraSwitchDelay; //0x036C
		AngleOfImpactData m_AngleOfImpact; //0x0370
		float m_LockingTimeMultiplier; //0x0388
		VehicleLockableInfoData m_VehicleLockableInfo; //0x038C
		float m_HighAltitudeLockHeight; //0x039C
		class VehicleSoundData* m_Sound; //0x03A0
		class EntityVoiceOverInfo* m_VoiceOverInfo; //0x03A8
		float m_SpottingFovScale; //0x03B0
		char _0x03B4[4];
		class CharacterBlueprint* m_CharacterBlueprintOverride; //0x03B8
		float m_AIFleeRadius; //0x03C0
		enum AIVehicleSoundCategory m_AISoundCategory; //0x03C4
		enum VehicleScoringCategory m_ScoringCategory; //0x03C8
		float m_CapWaterDepthAt; //0x03CC
		int m_WaterDepthUpdateRate; //0x03D0
		bool m_AllowVehicleOutsideCombatAreas; //0x03D4
		bool m_UseTopZone; //0x03D5
		bool m_HealthZonesShareDamage; //0x03D6
		bool m_UseProtectedShields; //0x03D7
		bool m_ShowPlayerHealth; //0x03D8
		bool m_UseSelfForSelfDestruct; //0x03D9
		bool m_RandomPreDestruction; //0x03DA
		bool m_BypassPreDestruction; //0x03DB
		bool m_DestroyAllComponentsOnDestroyed; //0x03DC
		bool m_AllowCockpitMesh; //0x03DD
		bool m_ForegroundRenderCockpitMesh; //0x03DE
		bool m_MotionBlurMask; //0x03DF
		bool m_TransparentWithEmittersEnable; //0x03E0
		bool m_SuppressDamageByPassengers; //0x03E1
		bool m_AllowClientSideSimulation; //0x03E2
		bool m_AllowSquadSpawnOnFullVehicle; //0x03E3
		bool m_TriggerVehicleDetonation; //0x03E4
		bool m_TriggerExplosionPackTriggerTypes; //0x03E5
		bool m_IsAffectedByEMP; //0x03E6
		bool m_CanBeRepaired; //0x03E7
		bool m_CanTakeDynamicFireDamage; //0x03E8
		bool m_AlwaysDealCollisionDamage; //0x03E9
		bool m_HasExclusiveEntries; //0x03EA
		bool m_OnlyCreatorCanEnter; //0x03EB
		bool m_AllowExclusiveEntryPassThrough; //0x03EC
		bool m_ProhibitEntrySwitching; //0x03ED
		bool m_AllowInactiveEntries; //0x03EE
		bool m_ThrowOutSoldierInsideOnWaterDamage; //0x03EF
		bool m_DeadSoldiersCanBeThrownOut; //0x03F0
		bool m_IgnoreSoldierCollisionNormal; //0x03F1
		bool m_ChoseExitPointByDirection; //0x03F2
		bool m_EnterAllowed; //0x03F3
		bool m_SpawnAllowed; //0x03F4
		bool m_ExitAllowed; //0x03F5
		bool m_EnableGroundmapLighting; //0x03F6
		bool m_BlockSpawnArea; //0x03F7
		bool m_ShouldDisturbWater; //0x03F8
		bool m_IsLockable; //0x03F9
		bool m_NeverReportVehicleAsEmpty; //0x03FA
		bool m_AITarget; //0x03FB
		bool m_UseLowAltitudeHeatSignature; //0x03FC
		bool m_UseSpottingTargetComponentForRaycast; //0x03FD
		bool m_EquipmentFakeVehicle; //0x03FE
		bool m_ExplosionPacksAttachable; //0x03FF
		bool m_DamageGiverOverrideOwnerVehicle; //0x0400
		bool m_EnableSubRealm; //0x0401
		bool m_UpdateWhenEmpty; //0x0402
		bool m_UseStaticBoundingBox; //0x0403
		bool m_UseDisplayWithinRadius; //0x0404
		bool m_StickyToEdgeInMap; //0x0405
		bool m_CausesAIToFlee; //0x0406
		bool m_CausesAIToEvade; //0x0407
		bool m_ForceNetworkingInclusion; //0x0408
		char _0x0409[7];
	};//Size=0x0410
};
fb::hashtable_iterator<_QWORD> *__fastcall hashtable_find( fb::hashtable<_QWORD>* table, fb::hashtable_iterator<_QWORD>* iterator, _QWORD key );
