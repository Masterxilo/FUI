//Include windows.h for touch events
#include "windows.h"  
#include <comdef.h>
#include <manipulations.h>
#include <ocidl.h>
// Manipulation implementation file
#include <manipulations_i.c>
#include <stdio.h>

// Smart Pointer to a global reference of a manipulation processor, event sink
IManipulationProcessor* g_pIManipProc;

//Include your definition of the event sink, CManipulationEventSink.h in this case
#include "CManipulationEventSink.h"    

// Set up a variable to point to the manipulation event sink implementation class    
CManipulationEventSink* g_pManipulationEventSink;

void touchInit(HWND hWnd) {

    HRESULT hr = CoInitialize(0);

    hr = CoCreateInstance(CLSID_ManipulationProcessor,
        NULL,
        CLSCTX_INPROC_SERVER,
        IID_IUnknown,
        (VOID**)(&g_pIManipProc)
        );
    // OnInitDialog
    g_pManipulationEventSink = new CManipulationEventSink(g_pIManipProc, hWnd);
    RegisterTouchWindow(hWnd, 0);

}


#include <map>
using namespace std;
map<DWORD, map<int, POINT>> touches;

float len(int x, int y) {
    return sqrt(x*x + y*y);
}
extern int swipeThreasholdDistanceInPixels, swipeThresholdTimeInMs;
void mm_swipe(
    double xfrom, double yfrom,
    double xto, double yto,
    double angle,
    double length,
    double timeInMs
    );
#include <math.h>
LRESULT onTouch(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
    UINT cInputs = LOWORD(wParam);
    PTOUCHINPUT pInputs = new TOUCHINPUT[cInputs];

    BOOL bHandled = FALSE;

    if (NULL != pInputs) {
        if (GetTouchInputInfo((HTOUCHINPUT)lParam,
            cInputs,
            pInputs,
            sizeof(TOUCHINPUT))) {


            for (UINT i = 0; i < cInputs; i++) {
                if (pInputs[i].dwFlags & TOUCHEVENTF_DOWN
                    //&& 
                    //pInputs[i].dwFlags & TOUCHEVENTF_MOVE == 0
                    ){
                    g_pIManipProc->ProcessDown(pInputs[i].dwID, static_cast<FLOAT>(pInputs[i].x), static_cast<FLOAT>(pInputs[i].y));
                    bHandled = TRUE;

                    /*POINT p = {pInputs[i].x, pInputs[i].y};
                    ScreenToClient(hWnd, &p);*/
                    touches[pInputs[i].dwID] = map<int, POINT>();
                    touches[pInputs[i].dwID][GetCurrentTime()] = {pInputs[i].x, pInputs[i].y};
                }
                if (pInputs[i].dwFlags & TOUCHEVENTF_UP){
                    g_pIManipProc->ProcessUp(pInputs[i].dwID, static_cast<FLOAT>(pInputs[i].x), static_cast<FLOAT>(pInputs[i].y));
                    bHandled = TRUE;

                    touches.erase(pInputs[i].dwID);
                }
                if (pInputs[i].dwFlags & TOUCHEVENTF_MOVE){
                    g_pIManipProc->ProcessMove(pInputs[i].dwID, static_cast<FLOAT>(pInputs[i].x), static_cast<FLOAT>(pInputs[i].y));
                    bHandled = TRUE;

                    if (!(touches.find(pInputs[i].dwID) == touches.end())) {
                        touches[pInputs[i].dwID][GetCurrentTime()] = {pInputs[i].x, pInputs[i].y};

                        // travel at least distance d within last ... ms
                        for (DWORD t = GetCurrentTime(); t >= GetCurrentTime() - swipeThresholdTimeInMs; t--) {
                            // no record for that time
                            if (touches[pInputs[i].dwID].find(t) == touches[pInputs[i].dwID].end())
                                continue;
                            // distance too small
                            float lenn;
                            if ((lenn = len(
                                touches[pInputs[i].dwID][t].x - pInputs[i].x,
                                touches[pInputs[i].dwID][t].y - pInputs[i].y)/100.0
                                )
                                < 
                                swipeThreasholdDistanceInPixels)
                                continue;
                            // all tests passed, convert to window-relative pixel coords
                            POINT p1 = {touches[pInputs[i].dwID][t].x / 100, touches[pInputs[i].dwID][t].y / 100},
                                p2 = {pInputs[i].x / 100, pInputs[i].y / 100};
                            ScreenToClient(hWnd, &p1);
                            ScreenToClient(hWnd, &p2);

                            mm_swipe(
                                p1.x, p1.y, p2.x, p2.y,
                                atan2(
                                p2.y - p1.y,
                                p2.x - p1.x),// angle
                                lenn,
                                GetCurrentTime() - t
                                );
                            touches.erase(pInputs[i].dwID);
                            break;
                        }
                    } // process swipe
                } // process touchmove

            } // for cInputs
        }
        else {
            // GetLastError() and error handling
        }
        delete[] pInputs;
    }
    else {
        // error handling, presumably out of memory
    }
    if (bHandled){
        // if you don't want to pass to DefWindowProc, close the touch input handle
        if (!CloseTouchInputHandle((HTOUCHINPUT)lParam)) {
            // error handling
        }
        return 0;
    }
    /*else{
        return DefWindowProc(hWnd, WM_TOUCH, wParam, lParam);
        }*/
}