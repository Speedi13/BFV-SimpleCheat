#pragma once

BYTE* HookVTableFunction(DWORD64** ppVTable, BYTE* pHook, SIZE_T iIndex);

void PlaceJmp( DWORD64 pAddr, DWORD64 pTarget );

double Distance2D(float x1,float y1,float x2,float y2) ;
double Distance3D(float x1,float y1,float z1,float x2,float y2,float z2);
double Distance3D(fb::Vec4 *vec1 ,fb::Vec4 *vec2);

//credits to Chevyyy
float XAngle(float x1,float y1,float x2,float y2,float myangle);
void RotatePointAlpha(float *outV, float x, float y, float z, float cx, float cy, float cz, float alpha);



