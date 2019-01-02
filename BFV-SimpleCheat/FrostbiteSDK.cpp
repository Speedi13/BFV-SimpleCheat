#include "FrostbiteSDK.h"
DWORD OFFSET_Soldier = NULL;
DWORD OFFSET_Vehicle = NULL;

fb::ClientSoldierEntity* fb::ClientPlayer::GetSoldier()
{
	DWORD_PTR* vtable = *(DWORD_PTR**)this;
	if ( (DWORD_PTR)vtable < 0x140000000 || (DWORD_PTR)vtable > 0x14FFFFFFF ) return nullptr;

	if (OFFSET_Soldier == NULL)
	{
		BYTE* fncGetCharacterEntity = (BYTE*)vtable[1]; 
		if ( (DWORD_PTR)fncGetCharacterEntity < 0x140000000 || (DWORD_PTR)fncGetCharacterEntity > 0x14FFFFFFF ) return nullptr;

		if (fncGetCharacterEntity[0] == 0xE9) //a jump, resolve it
		{
			__int32 Offset = *(__int32*)&fncGetCharacterEntity[1];
			fncGetCharacterEntity = &fncGetCharacterEntity[1] + Offset + sizeof(__int32);
		}
		OFFSET_Soldier = *(DWORD*)&fncGetCharacterEntity[3]; //48 8B 81 38 1D 00 00	mov     rax, [rcx+1D38h]
	}
	WeakPtr<ClientSoldierEntity> m_soldier = *(WeakPtr<ClientSoldierEntity>*)( (DWORD_PTR)this + OFFSET_Soldier ) ;
	return m_soldier.GetData();
}

fb::ClientVehicleEntity* fb::ClientPlayer::GetVehicle()
{
	if (OFFSET_Soldier == NULL)
	{
		this->GetSoldier();
		if (OFFSET_Soldier == NULL)
			return nullptr;
	}
	
	if (OFFSET_Vehicle == NULL)
		OFFSET_Vehicle = OFFSET_Soldier + 0x10;

	WeakPtr<ClientVehicleEntity> AttachedControllable = *(WeakPtr<ClientVehicleEntity>*)( (DWORD_PTR)this + OFFSET_Vehicle ) ;
	return AttachedControllable.GetData();
}

bool fb::ClientSoldierEntity::GetBonePos( int BoneId, fb::Vec4 *vOut )
{
	BoneCollisionComponent* pBoneCollisionComponent = this->GetBoneCollisionComponent();
	if( !ValidPointer( pBoneCollisionComponent) ) return false;

	if ( !pBoneCollisionComponent->m_ragdollTransforms.m_ValidTransforms ) return false;

	fb::UpdatePoseResultData::QuatTransform* pQuat = pBoneCollisionComponent->m_ragdollTransforms.m_ActiveWorldTransforms;
	if( !ValidPointer(pQuat) ) return false;
		
	vOut->x = pQuat[BoneId].m_TransAndScale.x;
	vOut->y = pQuat[BoneId].m_TransAndScale.y;
	vOut->z = pQuat[BoneId].m_TransAndScale.z;
	vOut->w = pQuat[BoneId].m_TransAndScale.w;

	return true;
}

fb::hashtable_iterator<_QWORD> *__fastcall hashtable_find(fb::hashtable<_QWORD>* table, fb::hashtable_iterator<_QWORD>* iterator, _QWORD key)
{  
	unsigned int mnBucketCount = table->mnBucketCount;

	//bfv
	unsigned int startCount = (_QWORD)(key) % (_QWORD)(mnBucketCount);
 
	//bf1
	//unsigned int startCount = (unsigned int)(key) % mnBucketCount;

	fb::hash_node<_QWORD>* node = table->mpBucketArray[ startCount ];
 
	if ( ValidPointer(node) && node->mValue.first ) 
	{
		while ( key != node->mValue.first )
		{
			node = node->mpNext;
			if ( !node || !ValidPointer(node) 
				)
				goto LABEL_4;
		}
		iterator->mpNode = node;
		iterator->mpBucket = &table->mpBucketArray[ startCount ];
	}
	else
	{
LABEL_4:
		iterator->mpNode = table->mpBucketArray[ mnBucketCount ];
		iterator->mpBucket = &table->mpBucketArray[ mnBucketCount ];
	}
	return iterator;
}