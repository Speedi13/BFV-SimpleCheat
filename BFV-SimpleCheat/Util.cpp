#include "FrostbiteSDK.h"
#include "Util.h"
#include <math.h>

BYTE* HookVTableFunction(DWORD64** ppVTable, BYTE* pHook, SIZE_T iIndex)
{
	DWORD dwOld = 0;
	VirtualProtect((void*)((*ppVTable) + iIndex), sizeof(PDWORD64), PAGE_EXECUTE_READWRITE, &dwOld);

	PBYTE pOrig = ((PBYTE)(*ppVTable)[iIndex]);
	(*ppVTable)[iIndex] = (DWORD64)pHook;

	VirtualProtect((void*)((*ppVTable) + iIndex), sizeof(PDWORD64), dwOld, &dwOld);

	return pOrig;
}

void PlaceJmp( DWORD64 pAddr, DWORD64 pTarget )
{
#if defined(_DEBUG_ENABLED)
	consolelog("["__FUNCTION__"] writing jmp to { %I64X }",pAddr);
#endif
	BYTE jmp[6+8] = {
		0xFF, 0x25, 0x00, 0x00, 0x00, 0x00
	};
    DWORD dwProtect = 0;
    VirtualProtect((void*)pAddr, 14, PAGE_EXECUTE_READWRITE, &dwProtect);
	
	*(DWORD64*)&jmp[6] = pTarget;
	for (int i = 0; i < (14/sizeof(WORD)); i++)
		*(WORD*)( pAddr + i*sizeof(WORD) ) = *(WORD*)&jmp[i*sizeof(WORD)];

    VirtualProtect((void*)pAddr, 14, dwProtect, NULL);
}

double Distance2D(float x1,float y1,float x2,float y2) 
{
	return sqrt((x2-x1)*(x2-x1)+(y2-y1)*(y2-y1));
}

double Distance3D(float x1,float y1,float z1,float x2,float y2,float z2)
{
	return sqrt((x2-x1)*(x2-x1)+(y2-y1)*(y2-y1)+(z2-z1)*(z2-z1));
}

double Distance3D(fb::Vec4 *vec1 ,fb::Vec4 *vec2)
{
	return sqrt(	( vec2->x - vec1->x )*
					( vec2->x - vec1->x )+( vec2->y - vec1->y )*
					( vec2->y - vec1->y )+( vec2->z - vec1->z )*
					( vec2->z - vec1->z )
			  );
}

//credits to Chevyyy
float XAngle(float x1,float y1,float x2,float y2,float myangle)
{
	float dl=(float)Distance2D( x1, y1 ,x2, y2 );                              
	if(dl==0)dl=1.0;                                                    
	float dl2=(float)abs(x2-x1);
	float teta=((float)(180.0/D3DX_PI)*(float)acos(dl2/dl));                            
	if(x2<x1)teta=(float)(180-teta);                                        
	if(y2<y1)teta=(float)(teta*-1.0);
	teta=teta-myangle;
	if(teta>180.0)teta=(float)((360.0-teta)*(-1.0));
	if(teta<-180.0)teta=(float)((360.0+teta));                                        
	return (float)teta;
}


void RotatePointAlpha(float *outV, float x, float y, float z, float cx, float cy, float cz, float alpha)
{
	//I don't want to include or link the DirectX SDK neither do I want to do it my self ^^
	static HMODULE hD3dx9_43 = NULL;
	if ( hD3dx9_43 == NULL )
		 hD3dx9_43 = LoadLibraryW( L"d3dx9_43.dll" );

	typedef fb::LinearTransform* (WINAPI* t_D3DXMatrixRotationY)( fb::LinearTransform *pOut, FLOAT Angle	);
	static t_D3DXMatrixRotationY D3DXMatrixRotationY = NULL;
	if (D3DXMatrixRotationY == NULL)
		D3DXMatrixRotationY = (t_D3DXMatrixRotationY)GetProcAddress( hD3dx9_43, "D3DXMatrixRotationY" );

	typedef fb::Vec4* (WINAPI* t_D3DXVec3Transform)( fb::Vec4 *pOut, CONST fb::Vec4 *pV, CONST fb::LinearTransform *pM	);
	static t_D3DXVec3Transform D3DXVec4Transform = NULL;
	if (D3DXVec4Transform == NULL)
		D3DXVec4Transform = (t_D3DXVec3Transform)GetProcAddress( hD3dx9_43, "D3DXVec4Transform" );

	fb::LinearTransform rot1;
	fb::Vec4 vec;
	vec.x=x-cx;
	vec.z=y-cy;
	vec.y=z-cz;
	vec.w=1.0f;
	D3DXMatrixRotationY(&rot1,(FLOAT)(alpha*D3DX_PI/180.0));
	D3DXVec4Transform(&vec,&vec,&rot1);
	outV[0]=vec.x+cx;
	outV[1]=vec.z+cy;
	outV[2]=vec.y+cz;
};
