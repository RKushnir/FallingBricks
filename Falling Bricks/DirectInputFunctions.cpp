#include "DirectInputFunctions.h"

static IDirectInput8 *g_pDirectInputObject = NULL; // the DirectInput object
IDirectInputDevice8 *g_pDirectInputDevice = NULL;

bool InitializeDirectInput(HWND hAppWindow)
{
	bool bResult = false;

	do 
	{
		IDirectInput8 *pDirectInputObject = NULL;

		HRESULT hrDInputCreateResult = DirectInput8Create(GetModuleHandle(NULL),
			DIRECTINPUT_VERSION,
			IID_IDirectInput8,
			(void**)&pDirectInputObject,
			NULL);

		if (hrDInputCreateResult == DI_OK)
		{
			IDirectInputDevice8 *pDirectInputDevice = NULL;

			HRESULT hrDInputDeviceCreateResult = pDirectInputObject->CreateDevice(GUID_SysKeyboard, &pDirectInputDevice, NULL);
			if (hrDInputDeviceCreateResult == DI_OK)
			{
				HRESULT hrSetInputDeviceFormatResult = pDirectInputDevice->SetDataFormat(&c_dfDIKeyboard);

				if (hrSetInputDeviceFormatResult == DI_OK)
				{
					pDirectInputDevice->SetCooperativeLevel(hAppWindow, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);

					HRESULT hrInputDeviceAcquireresult = pDirectInputDevice->Acquire();
					if (hrInputDeviceAcquireresult == DI_OK)
					{
						g_pDirectInputObject = pDirectInputObject;
						g_pDirectInputDevice = pDirectInputDevice;

						bResult = true;
						break;
					}
				}

				pDirectInputDevice->Release();
			}

			pDirectInputObject->Release();
		}
	}
	while (false);

	return bResult;
}

void FinalizeDirectInput()
{
	if (g_pDirectInputObject)
	{
		if (g_pDirectInputDevice)
		{
			g_pDirectInputDevice->Unacquire();
			g_pDirectInputDevice->Release();
			g_pDirectInputDevice = NULL;
		}

		g_pDirectInputObject->Release();
		g_pDirectInputObject = NULL;
	}
}

