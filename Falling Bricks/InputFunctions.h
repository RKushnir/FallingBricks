#ifndef __INPUTFUNCTIONS_H_INCLUDED
#define __INPUTFUNCTIONS_H_INCLUDED

#define KEYDOWN(InputData, Key) (InputData.data.keyboard.VKey == Key && InputData.data.keyboard.Message == WM_KEYDOWN)

bool InitializeInput(HWND hAppWindow);

#endif // __INPUTFUNCTIONS_H_INCLUDED
