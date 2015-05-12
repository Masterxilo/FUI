#include "myinclude.h"
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

float len(int x, int y) {
	return sqrt(x*x + y*y);
}

#include <math.h>
#include <vector>
#include <map>
using namespace std;

UINT cInputs;

// ==

void inputConfigChanges(POINT* p1, POINT* p2);
void move(POINT* p1, POINT* p2);


POINT* pget(
	vector<int>& I,
	map<int, POINT>& p, int i) {
	if (i >= I.size()) return 0;
	return &p[I[i]];
}

POINT getTouchinput(HWND hWnd, TOUCHINPUT& ip) {
	POINT p = { ip.x / 100, ip.y / 100 };
	ScreenToClient(hWnd, &p);
	return p;
}

// swipe
extern int swipeThreasholdDistanceInPixels, swipeThresholdTimeInMs;
void mm_swipe(
	double xfrom, double yfrom,
	double xto, double yto,
	double angle,
	double length,
	double timeInMs
	);
map<DWORD, POINT> touches;
void swipe_up() {
	touches.clear();
}
#include <math.h>
DWORD lastTime = 0;
void swipe_move(POINT & xy) {
	bool enableSwipe = 1;
	if (!enableSwipe) return;
	if (abs((int)(lastTime - GetCurrentTime())) < 3) return;
	lastTime = GetCurrentTime();
	touches[GetCurrentTime()] = xy;

	// travel at least distance d within last ... ms
	for (DWORD t = GetCurrentTime();
		t >= GetCurrentTime() - swipeThresholdTimeInMs; t--) {
		// no record for that time
		if (touches.find(t) == touches.end())
			continue;
		// distance too small
		float lenn;
		if ((lenn = len(
			touches[t].x - xy.x,
			touches[t].y - xy.y)
			)
			<
			swipeThreasholdDistanceInPixels)
			continue;
		// all tests passed, convert to window-relative pixel coords
		POINT p1 = touches[t], p2 = xy;

		mm_swipe(
			p1.x, p1.y, p2.x, p2.y,
			atan2(
			p2.y - p1.y,
			p2.x - p1.x),// angle
			lenn,
			GetCurrentTime() - t
			);
		touches.clear();
		break;
	}
}
// ==
bool enableManip = 1;
LRESULT onTouch(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	cInputs = LOWORD(wParam);
	PTOUCHINPUT pInputs = new TOUCHINPUT[cInputs];

	// ==
	static vector<int> I_;
	vector<int> I;
	map<int, POINT> p;
	// ==

	// : 
	dprintf(":--- cInputs = %d\n", cInputs);
	// :
	if (!GetTouchInputInfo((HTOUCHINPUT)lParam,
		cInputs,
		pInputs,
		sizeof(TOUCHINPUT))) return 0;

	for (UINT i = 0; i < cInputs; i++) {

		// : 
		POINT pt;
		pt = getTouchinput(hWnd, pInputs[i]);

		dprintf(":%i: (%i %i) %d (d%i m%i u%i)\n", i,
			pt.x, pt.y,
			pInputs[i].dwID,
			(pInputs[i].dwFlags & TOUCHEVENTF_DOWN) > 0,
			(pInputs[i].dwFlags & TOUCHEVENTF_MOVE) > 0,
			(pInputs[i].dwFlags & TOUCHEVENTF_UP) > 0

			);
		// :
		POINT touchinputPoint = getTouchinput(hWnd, pInputs[i]);
		// == 
		if (pInputs[i].dwFlags & TOUCHEVENTF_DOWN
			||
			pInputs[i].dwFlags & TOUCHEVENTF_MOVE
			) {
			I.push_back(pInputs[i].dwID);
			p[pInputs[i].dwID] = touchinputPoint;
		}
		// ==

		if (pInputs[i].dwFlags & TOUCHEVENTF_DOWN
			//&& 
			//pInputs[i].dwFlags & TOUCHEVENTF_MOVE == 0
			){
			// mp
			if (enableManip)
				g_pIManipProc->ProcessDown(pInputs[i].dwID, static_cast<FLOAT>(pInputs[i].x), static_cast<FLOAT>(pInputs[i].y));
			// mp

			/*POINT p = {pInputs[i].x, pInputs[i].y};
			ScreenToClient(hWnd, &p);*/
			swipe_up();
		}
		if (pInputs[i].dwFlags & TOUCHEVENTF_UP){
			// mp
			if (enableManip)
				g_pIManipProc->ProcessUp(pInputs[i].dwID, static_cast<FLOAT>(pInputs[i].x), static_cast<FLOAT>(pInputs[i].y));
			// mp

			swipe_up();
		}
		if (pInputs[i].dwFlags & TOUCHEVENTF_MOVE){
			// mp
			if (enableManip)
				g_pIManipProc->ProcessMove(pInputs[i].dwID, static_cast<FLOAT>(pInputs[i].x), static_cast<FLOAT>(pInputs[i].y));
			// mp
			if (cInputs == 1)
				swipe_move(touchinputPoint);

		} // process touchmove

	} // for cInputs


	delete[] pInputs;
	// :
	dprintf("I ");
	for (int i : I)
		dprintf("%d ", i);
	dprintf("\n");


	dprintf("I_ ");
	for (int i : I_)
		dprintf("%d ", i);
	dprintf("\n");
	// :

	// ==
	if (I != I_)
		inputConfigChanges(pget(I, p, 0), pget(I, p, 1));
	else
		move(pget(I, p, 0), pget(I, p, 1));

	I_ = I;
	// ==

	CloseTouchInputHandle((HTOUCHINPUT)lParam); // Do not close the handle, let defwndproc do it (?)
	return 0;
}