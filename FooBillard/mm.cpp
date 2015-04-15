#include "mm.h"
#include <Windows.h>
#include <windowsx.h>
#include <stdio.h>
#include <vector>
#include <iostream>
#include <map>
#define _USE_MATH_DEFINES
#include <math.h>
using namespace std;

int swipeThreasholdDistanceInPixels = 150,
    swipeThresholdTimeInMs = 100;
void mm_swipe(double xfrom, double yfrom,
    double xto, double yto,
    double angle,
    double length,
    double timeInMs
    ) {
    printf("swipe (%f %f)->(%f %f), angle %f, length %f, time %f\n",
        xfrom, yfrom, xto, yto, angle, length, timeInMs);

    // todo only if upwards and close to center of screen
    if (angle < 0) {
        INPUT in = {0};
        in.type = INPUT_KEYBOARD;
        
            in.ki.wVk = VK_SPACE;
        SendInput(1, &in, sizeof(in)); // todo: only if ball is not moving
        in.ki.dwFlags = KEYEVENTF_KEYUP;
        SendInput(1, &in, sizeof(in)); // todo: only if ball is not moving

        
    }
}

void mm_gesture_begin() {
    printf("ManipulationStarted\n");
}
void mm_gesture_end() {
    printf("ManipulationCompleted\n");
}

void mm_gesture(  /* [in] */ FLOAT x,
    /* [in] */ FLOAT y,
    /* [in] */ FLOAT translationDeltaX,
    /* [in] */ FLOAT translationDeltaY,
    /* [in] */ FLOAT scaleDelta,
    /* [in] */ FLOAT expansionDelta,
    /* [in] */ FLOAT rotationDelta,
    /* [in] */ FLOAT cumulativeTranslationX,
    /* [in] */ FLOAT cumulativeTranslationY,
    /* [in] */ FLOAT cumulativeScale,
    /* [in] */ FLOAT cumulativeExpansion,
    /* [in] */ FLOAT cumulativeRotation) {

    if (0)
    printf("(%f %f) (%f %f) %f %f %f total: (%f %f) %f %f %f\n",
        x,
        y,
        translationDeltaX,
        translationDeltaY,
        scaleDelta,
        expansionDelta,
        rotationDelta,
        cumulativeTranslationX,
        cumulativeTranslationY,
        cumulativeScale,
        cumulativeExpansion,
        cumulativeRotation
        );


}

// Infrastructure
LRESULT (CALLBACK *oldProc)(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) = 0;

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    printf("message\n");
    return oldProc(hWnd, message, wParam, lParam);
}

/****************************************************************
WH_MSGFILTER hook procedure
****************************************************************/
HHOOK hhook;
HWND hWnd;

LRESULT onTouch(HWND hWnd, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK MessageProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    CWPSTRUCT* msg = (CWPSTRUCT*)lParam;
    if (nCode < 0 || !msg)
        return CallNextHookEx(hhook, nCode, wParam, lParam);

    if (msg->hwnd == hWnd) {

        if (msg->message == WM_TOUCH)
            onTouch(msg->hwnd, msg->wParam, msg->lParam);
    }

    return CallNextHookEx(hhook, nCode, wParam, lParam);

}
void touchInit(HWND);
extern "C" {
    void mm_init() {
        // Hook wndproc
        hWnd = FindWindowA(0, "FooBillard");
        *(LONG*)&oldProc = GetWindowLong(hWnd, GWL_WNDPROC);
        //SetWindowLong(hWnd, GWL_WNDPROC, (LONG)&WndProc);

        hhook = SetWindowsHookEx(
            WH_CALLWNDPROC,
            MessageProc,
            GetModuleHandle(0),//hinstDLL,
            GetCurrentThreadId());
        AllocConsole();
        freopen("CONOUT$", "w", stdout);

        printf("hwnd %d, hhook %d\n", hWnd, hhook);
    
        touchInit(hWnd);
    }

}