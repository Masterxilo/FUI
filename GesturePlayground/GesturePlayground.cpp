// GesturePlayground.cpp : Definiert den Einstiegspunkt fur die Anwendung.
//

#include "stdafx.h"
#include "GesturePlayground.h"

#define MAX_LOADSTRING 100

// Globale Variablen:
HINSTANCE hInst;								// Aktuelle Instanz
TCHAR szTitle[MAX_LOADSTRING];					// Titelleistentext
TCHAR szWindowClass[MAX_LOADSTRING];			// Klassenname des Hauptfensters

// Vorwartsdeklarationen der in diesem Codemodul enthaltenen Funktionen:
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

  // TODO: Hier Code einfugen.
  MSG msg;
  HACCEL hAccelTable;

  // Globale Zeichenfolgen initialisieren
  LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
  LoadString(hInstance, IDC_GESTUREPLAYGROUND, szWindowClass, MAX_LOADSTRING);
  MyRegisterClass(hInstance);

  // Anwendungsinitialisierung ausfuhren:
  if (!InitInstance(hInstance, nCmdShow))
  {
    return FALSE;
  }

  hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_GESTUREPLAYGROUND));

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
  wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_GESTUREPLAYGROUND));
  wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
  wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
  wcex.lpszMenuName = MAKEINTRESOURCE(IDC_GESTUREPLAYGROUND);
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
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow){
  HWND hWnd;
  hInst = hInstance; // Store instance handle in our global variable
  hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
    CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);
  if (!hWnd)
  {
    return FALSE;
  }
  ShowWindow(hWnd, nCmdShow);
  UpdateWindow(hWnd);
  return TRUE;
}
#include <stdio.h>
LRESULT DecodeGesture(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam){
  // Create a structure to populate and retrieve the extra message info.
  GESTUREINFO gi;

  ZeroMemory(&gi, sizeof(GESTUREINFO));

  gi.cbSize = sizeof(GESTUREINFO);
  BOOL bResult = GetGestureInfo((HGESTUREINFO)lParam, &gi);
  BOOL bHandled = FALSE;

  if (gi.dwFlags & GF_BEGIN) printf("GF_BEGIN ");
  if (gi.dwFlags & GF_END) printf("GF_END ");
  if (gi.dwFlags & GF_INERTIA) printf("GF_INTERTIA ");
  printf("%ull ", gi.ullArguments);
  printf("(%d %d) ", gi.ptsLocation.x, gi.ptsLocation.y);
  if (bResult){
    // now interpret the gesture
    switch (gi.dwID){
    case GID_ZOOM:
      // Code for zooming goes here
      printf("zoom\n");
      bHandled = TRUE;
      break;
    case GID_PAN:
      // Code for panning goes here
      // The GID_PAN gesture has built‐in inertia. At the end of a pan gesture, additional pan gesture messages are
//      created by the operating system.
      printf("pan\n");
      bHandled = TRUE;
      break;
    case GID_ROTATE:
      // Code for rotation goes here
      printf("rotata\n");
      bHandled = TRUE;
      break;
    case GID_TWOFINGERTAP:
      // Code for two-finger tap goes here
      printf("twofingertap\n");
      bHandled = TRUE;
      break;
    case GID_PRESSANDTAP:
      // Code for roll over goes here
      printf("pressandtap\n");
      bHandled = TRUE;
      break;
    default:
      // A gesture was not recognized
      printf("no gesture\n");
      break;
    }
  }
  else{
    DWORD dwErr = GetLastError();
    if (dwErr  > 0){
      //MessageBoxW(hWnd, L"Error!", L"Could not retrieve a GESTUREINFO structure.", MB_OK);
    }
  }
  if (bHandled){
    return 0;
  }
  else{
    return DefWindowProc(hWnd, message, wParam, lParam);
  }
}
//
//  FUNKTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  ZWECK:  Verarbeitet Meldungen vom Hauptfenster.
//
//  WM_COMMAND	- Verarbeiten des Anwendungsmenus
//  WM_PAINT	- Zeichnen des Hauptfensters
//  WM_DESTROY	- Beenden-Meldung anzeigen und zuruckgeben
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  int wmId, wmEvent;
  PAINTSTRUCT ps;
  HDC hdc;

  switch (message)
  {
  case WM_GESTURE:
    // Insert handler code here to interpret the gesture.           
    return DecodeGesture(hWnd, message, wParam, lParam);
  case WM_COMMAND:
    wmId = LOWORD(wParam);
    wmEvent = HIWORD(wParam);
    // Menuauswahl bearbeiten:
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
  case WM_PAINT:
    hdc = BeginPaint(hWnd, &ps);
    // TODO: Hier den Zeichnungscode hinzufugen.
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

// Meldungshandler fur Infofeld.
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
