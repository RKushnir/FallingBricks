#ifndef __DIRECT3DFUNCTIONS_H_INCLUDED
#define __DIRECT3DFUNCTIONS_H_INCLUDED

#include <d3d9.h>
#include <d3dx9.h>
#include <D3DX9Shader.h>

extern IDirect3DDevice9 *g_pDirect3DDevice;

bool InitializeDirect3D(HWND hAppWindow, int nWindowWidth, int nWindowHeight);
void FinalizeDirect3D();

bool CreatePixelShaderFromResource(LPCTSTR szResourceId, IDirect3DPixelShader9 **ppsOutPixelShader);

#endif // __DIRECT3DFUNCTIONS_H_INCLUDED
