// MinipulationsPlayground.cpp : Definiert den Einstiegspunkt für die Anwendung.
//

#include "stdafx.h"
#include "MinipulationsPlayground.h"

//Include windows.h for touch events
#include "windows.h"  
#include <comdef.h>
#include <manipulations.h>
#include <ocidl.h>
// Manipulation implementation file
#include <manipulations_i.c>

// Smart Pointer to a global reference of a manipulation processor, event sink
IManipulationProcessor* g_pIManipProc;

//Include your definition of the event sink, CManipulationEventSink.h in this case
#include "CManipulationEventSink.h"    

// Set up a variable to point to the manipulation event sink implementation class    
CManipulationEventSink* g_pManipulationEventSink;



#define MAX_LOADSTRING 100

// Globale Variablen:
HINSTANCE hInst;								// Aktuelle Instanz
TCHAR szTitle[MAX_LOADSTRING];					// Titelleistentext
TCHAR szWindowClass[MAX_LOADSTRING];			// Klassenname des Hauptfensters

// Vorwärtsdeklarationen der in diesem Codemodul enthaltenen Funktionen:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPTSTR    lpCmdLine,
    _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Hier Code einfügen.

    HRESULT hr = CoInitialize(0);

    hr = CoCreateInstance(CLSID_ManipulationProcessor,
        NULL,
        CLSCTX_INPROC_SERVER,
        IID_IUnknown,
        (VOID**)(&g_pIManipProc)
        );




    MSG msg;
    HACCEL hAccelTable;

    // Globale Zeichenfolgen initialisieren
    LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadString(hInstance, IDC_MINIPULATIONSPLAYGROUND, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Anwendungsinitialisierung ausführen:
    if (!InitInstance(hInstance, nCmdShow))
    {
        return FALSE;
    }

    hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_MINIPULATIONSPLAYGROUND));

    // Hauptnachrichtenschleife:
    while (GetMessage(&msg, NULL, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int)msg.wParam;
}



//
//  FUNKTION: MyRegisterClass()
//
//  ZWECK: Registriert die Fensterklasse.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEX wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MINIPULATIONSPLAYGROUND));
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = MAKEINTRESOURCE(IDC_MINIPULATIONSPLAYGROUND);
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassEx(&wcex);
}

//
//   FUNKTION: InitInstance(HINSTANCE, int)
//
//   ZWECK: Speichert das Instanzenhandle und erstellt das Hauptfenster.
//
//   KOMMENTARE:
//
//        In dieser Funktion wird das Instanzenhandle in einer globalen Variablen gespeichert, und das
//        Hauptprogrammfenster wird erstellt und angezeigt.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    HWND hWnd;

    hInst = hInstance; // Instanzenhandle in der globalen Variablen speichern

    hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

    if (!hWnd)
    {
        return FALSE;
    }

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    // OnInitDialog
    g_pManipulationEventSink = new CManipulationEventSink(g_pIManipProc, hWnd);


    RegisterTouchWindow(hWnd, 0);


    return TRUE;
}

#include <map>
using namespace std;
map<DWORD, map<int, POINT>> touches;

float len(int x, int y) {
    return sqrt(x*x + y*y);
}

LRESULT OnTouch(HWND hWnd, WPARAM wParam, LPARAM lParam)
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

                        // travel at least distance d within last 300 ms
                        for (DWORD t = GetCurrentTime(); t >= GetCurrentTime() - 100; t--) {
                            // no record for that time
                            if (touches[pInputs[i].dwID].find(t) == touches[pInputs[i].dwID].end())
                                continue;
                            // distance too small
                            if (len(
                                touches[pInputs[i].dwID][t].x - pInputs[i].x,
                                touches[pInputs[i].dwID][t].y - pInputs[i].y) < 200 * 100)
                                continue;

                            printf("=============== SWIPE ===========\n");

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
    else{
        return DefWindowProc(hWnd, WM_TOUCH, wParam, lParam);
    }
}



//
//  FUNKTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  ZWECK:  Verarbeitet Meldungen vom Hauptfenster.
//
//  WM_COMMAND	- Verarbeiten des Anwendungsmenüs
//  WM_PAINT	- Zeichnen des Hauptfensters
//  WM_DESTROY	- Beenden-Meldung anzeigen und zurückgeben
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    int wmId, wmEvent;
    PAINTSTRUCT ps;
    HDC hdc;

    switch (message)
    {
    case WM_COMMAND:
        wmId = LOWORD(wParam);
        wmEvent = HIWORD(wParam);
        // Menüauswahl bearbeiten:
        switch (wmId)
        {
        case IDM_ABOUT:
            DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
            break;
        case IDM_EXIT:
            DestroyWindow(hWnd);
            break;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
        break;
    case WM_TOUCH:
        return OnTouch(hWnd, wParam, lParam);

    case WM_PAINT:
        hdc = BeginPaint(hWnd, &ps);
        // TODO: Hier den Zeichnungscode hinzufügen.
        EndPaint(hWnd, &ps);
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Meldungshandler für Infofeld.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
