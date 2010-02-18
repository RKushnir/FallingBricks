#include "StdAfx.h"
#include "InputFunctions.h"

#define HID_USAGE_PAGE_GENERIC ((USHORT)0x01)
#define HID_USAGE_GENERIC_KEYBOARD ((USHORT)0x06)
#define HID_USAGE_GENERIC_MOUSE ((USHORT)0x02)

bool InitializeInput(HWND hAppWindow)
{
	bool bResult = false;

	RAWINPUTDEVICE ridInputDeviceInfo;
	ridInputDeviceInfo.usUsagePage = HID_USAGE_PAGE_GENERIC;
	ridInputDeviceInfo.usUsage = HID_USAGE_GENERIC_KEYBOARD;
	ridInputDeviceInfo.dwFlags = 0;
	ridInputDeviceInfo.hwndTarget = hAppWindow;

	if (RegisterRawInputDevices(&ridInputDeviceInfo, 1, sizeof(ridInputDeviceInfo)))
	{
		bResult = true;
	}

	return bResult;
}

