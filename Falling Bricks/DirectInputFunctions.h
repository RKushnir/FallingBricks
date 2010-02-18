#ifndef __DIRECTINPUTFUNCTIONS_H_INCLUDED
#define __DIRECTINPUTFUNCTIONS_H_INCLUDED

#include <windows.h>

#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>

#define KEYPRESSED(Keys, Index) (Keys[Index] & 0x80)
#define KEYUP(PreviousKeys, Keys, Index) ((PreviousKeys[Index] & 0x80) && !(Keys[Index] & 0x80))
#define KEYDOWN(PreviousKeys, Keys, Index) (!(PreviousKeys[Index] & 0x80) && (Keys[Index] & 0x80))

extern IDirectInputDevice8 *g_pDirectInputDevice;

bool InitializeDirectInput(HWND hAppWindow);
void FinalizeDirectInput();

#endif // __DIRECTINPUTFUNCTIONS_H_INCLUDED
