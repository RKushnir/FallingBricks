#include "StdAfx.h"
#include "Direct3DFunctions.h"
#include "InputFunctions.h"
#include "XACTFunctions.h"

static const int g_nWindowWidth = 580;
static const int g_nWindowHeight = 700;

bool InitializeWindow(HINSTANCE hInstance, int nWindowWidth, int nWindowHeight, HWND &hOutAppWindow);
int RunMessageLoop();
LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR szCmdLine, int iCmdShow)
{
	int nResult = 0;

	HWND hAppWindow;
	if (InitializeWindow(hInstance, g_nWindowWidth, g_nWindowHeight, hAppWindow))
	{
		if (InitializeDirect3D(hAppWindow, g_nWindowWidth, g_nWindowHeight))
		{
			if (InitializeInput(hAppWindow))
			{
				if (SUCCEEDED(InitializeXACT()))
				{
					if (InitializeGame())
					{
						nResult = RunMessageLoop();
						FinalizeGame();
					}

					FinalizeXACT();
				}
			}

			FinalizeDirect3D();
		}
	}

    return nResult;
}

bool InitializeWindow(HINSTANCE hInstance, int nWindowWidth, int nWindowHeight, HWND &hOutAppWindow)
{
	bool bResult = false;
	hOutAppWindow = NULL;

	static TCHAR szAppName[] = TEXT("Falling Bricks");
	WNDCLASSEX wcWindowClassDesc = {0};
	wcWindowClassDesc.cbSize        = sizeof(WNDCLASSEX);
	wcWindowClassDesc.style         = CS_HREDRAW | CS_VREDRAW;
	wcWindowClassDesc.lpfnWndProc   = WndProc;
	wcWindowClassDesc.cbClsExtra    = 0;
	wcWindowClassDesc.cbWndExtra    = 0;
	wcWindowClassDesc.hInstance     = hInstance;
	wcWindowClassDesc.hIcon         = LoadIcon(NULL, IDI_APPLICATION);
	wcWindowClassDesc.hCursor       = LoadCursor(NULL, IDC_ARROW);
	wcWindowClassDesc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wcWindowClassDesc.lpszMenuName  = NULL;
	wcWindowClassDesc.lpszClassName = szAppName;
	wcWindowClassDesc.hIconSm       = wcWindowClassDesc.hIcon;

	if (RegisterClassEx(&wcWindowClassDesc))
	{
		RECT rWindowRect = { 0, 0, nWindowWidth, nWindowHeight };
		AdjustWindowRect(&rWindowRect, WS_OVERLAPPED | WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU, FALSE);

		const HWND hAppWindow = CreateWindowEx(WS_EX_OVERLAPPEDWINDOW,
			szAppName,
			TEXT ("Falling Bricks"),
			WS_OVERLAPPED | WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			rWindowRect.right - rWindowRect.left,
			rWindowRect.bottom - rWindowRect.top,
			NULL,
			NULL,
			hInstance,
			NULL);

		if (hAppWindow)
		{
			ShowWindow(hAppWindow, SW_SHOW);
			UpdateWindow(hAppWindow);

			hOutAppWindow = hAppWindow;
			bResult = true;
		}
	}

	return bResult;
}

int RunMessageLoop()
{
	MSG msg = {0};

	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE) == TRUE)
		{
			if (msg.message == WM_INPUT)
			{
				ProcessUserInput(msg.lParam);
			}

			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			UpdateGame();
			RenderGame();
		}
	}

    return msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
	    case WM_DESTROY:
		{
			PostQuitMessage(0);

			break;
		}

		default:
		{
			//break;
		}
    }

	return DefWindowProc(hwnd, message, wParam, lParam);
}

