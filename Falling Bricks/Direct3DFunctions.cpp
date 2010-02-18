#include "StdAfx.h"
#include "Direct3DFunctions.h"

static IDirect3D9 *g_pDirect3DObject = NULL;
IDirect3DDevice9 *g_pDirect3DDevice = NULL;

bool InitializeDirect3D(HWND hAppWindow, int nWindowWidth, int nWindowHeight)
{
	bool bResult = false;

	do 
	{
		IDirect3D9 *pDirect3DObject = Direct3DCreate9(D3D_SDK_VERSION);

		if (pDirect3DObject != NULL)
		{
			IDirect3DDevice9 *pDevice = NULL;

			D3DPRESENT_PARAMETERS pPresentParameters; 
			ZeroMemory(&pPresentParameters, sizeof(pPresentParameters));
			pPresentParameters.BackBufferWidth = nWindowWidth;
			pPresentParameters.BackBufferHeight = nWindowHeight;
			pPresentParameters.BackBufferFormat = D3DFMT_A8R8G8B8;
			pPresentParameters.BackBufferCount = 1;
			pPresentParameters.SwapEffect = D3DSWAPEFFECT_DISCARD;
			pPresentParameters.hDeviceWindow = hAppWindow;
			pPresentParameters.Windowed = TRUE;

			const HRESULT hDeviceCreateResult = pDirect3DObject->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hAppWindow, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &pPresentParameters, &pDevice);

			if (hDeviceCreateResult == D3D_OK)
			{
				g_pDirect3DObject = pDirect3DObject;
				g_pDirect3DDevice = pDevice;

				bResult = true;
				break;
			}

			pDirect3DObject->Release();
		}
	}
	while (false);

	return bResult;
}

void FinalizeDirect3D()
{
	if (g_pDirect3DObject)
	{
		if (g_pDirect3DDevice)
		{
			g_pDirect3DDevice->Release();
			g_pDirect3DDevice = NULL;
		}

		g_pDirect3DObject->Release();
		g_pDirect3DObject = NULL;
	}
}

bool CreatePixelShaderFromResource(LPCTSTR szResourceId, IDirect3DPixelShader9 **ppsOutPixelShader)
{
	bool bResult = false;
	*ppsOutPixelShader = NULL;

	ID3DXBuffer *pbPixelShaderCode;

	const HRESULT hrPixelShaderCompilationResult = D3DXCompileShaderFromResource(NULL, szResourceId, NULL, NULL, "ps_main", "ps_2_0", 0, &pbPixelShaderCode, NULL, NULL);

	if (hrPixelShaderCompilationResult == D3D_OK)
	{
		const HRESULT hrPixelShaderCreationResult = g_pDirect3DDevice->CreatePixelShader((DWORD*)pbPixelShaderCode->GetBufferPointer(), ppsOutPixelShader);
		pbPixelShaderCode->Release();

		if (hrPixelShaderCreationResult == D3D_OK)
		{
			bResult = true;
		}
	}

	return bResult;
}

