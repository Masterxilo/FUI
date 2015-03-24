#include "stdafx.h"
#include "cmanipulationeventsink.h"

CManipulationEventSink::CManipulationEventSink(IManipulationProcessor *manip, HWND hWnd)
{
  m_hWnd = hWnd;

  //Set initial ref count to 1.
  m_cRefCount = 1;

  m_pManip = manip;
  m_pManip->put_PivotRadius(-1); // ?

  m_cStartedEventCount = 0;
  m_cDeltaEventCount = 0;
  m_cCompletedEventCount = 0;

  HRESULT hr;

  //Get the container with the connection points.
  IConnectionPointContainer* spConnectionContainer;

  hr = manip->QueryInterface(
    IID_IConnectionPointContainer,
    (LPVOID*)&spConnectionContainer
    );
  //hr = manip->QueryInterface(&spConnectionContainer);
  if (spConnectionContainer == NULL){
    // something went wrong, try to gracefully quit
    printf("spConnectionContainer == NULL"); exit(1);
  }

  //Get a connection point.
  hr = spConnectionContainer->FindConnectionPoint(__uuidof(_IManipulationEvents), &m_pConnPoint);
  if (m_pConnPoint == NULL){
    // something went wrong, try to gracefully quit
    printf("m_pConnPoint == NULL"); exit(1);
  }

  DWORD dwCookie;

  //Advise.
  hr = m_pConnPoint->Advise(this, &dwCookie);
}

int CManipulationEventSink::GetStartedEventCount()
{
  return m_cStartedEventCount;
}

int CManipulationEventSink::GetDeltaEventCount()
{
  return m_cDeltaEventCount;
}

int CManipulationEventSink::GetCompletedEventCount()
{
  return m_cCompletedEventCount;
}

double CManipulationEventSink::GetX()
{
  return m_fX;
}

double CManipulationEventSink::GetY()
{
  return m_fY;
}

CManipulationEventSink::~CManipulationEventSink()
{
  //Cleanup.
}

///////////////////////////////////
//Implement IManipulationEvents
///////////////////////////////////

HRESULT STDMETHODCALLTYPE CManipulationEventSink::ManipulationStarted(
  /* [in] */ FLOAT x,
  /* [in] */ FLOAT y)
{
  m_cStartedEventCount++;
  printf("ManipulationStarted\n");
  return S_OK;
}
void ClearScreen()
{
  HANDLE                     hStdOut;
  CONSOLE_SCREEN_BUFFER_INFO csbi;
  DWORD                      count;
  DWORD                      cellCount;
  COORD                      homeCoords = {0, 0};

  hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
  if (hStdOut == INVALID_HANDLE_VALUE) return;

  /* Get the number of cells in the current buffer */
  if (!GetConsoleScreenBufferInfo(hStdOut, &csbi)) return;
  cellCount = csbi.dwSize.X *csbi.dwSize.Y;

  /* Fill the entire buffer with spaces */
  if (!FillConsoleOutputCharacter(
    hStdOut,
    (TCHAR) ' ',
    cellCount,
    homeCoords,
    &count
    )) return;

  /* Fill the entire buffer with the current colors and attributes */
  if (!FillConsoleOutputAttribute(
    hStdOut,
    csbi.wAttributes,
    cellCount,
    homeCoords,
    &count
    )) return;

  /* Move the cursor home */
  SetConsoleCursorPosition(hStdOut, homeCoords);
}
HRESULT STDMETHODCALLTYPE CManipulationEventSink::ManipulationDelta(
  /* [in] */ FLOAT x,
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
  /* [in] */ FLOAT cumulativeRotation)
{
  ClearScreen();
  printf("ManipulationDelta\n");
  printf("\
    %f x\n\
    %f y\n\
    %f translationDeltaX\n\
    %f translationDeltaY\n\
    %f scaleDelta\n\
    %f expansionDelta\n\
    %f rotationDelta\n\
    %f cumulativeTranslationX\n\
    %f cumulativeTranslationY\n\
    %f cumulativeScale\n\
    %f cumulativeExpansion\n\
    %f cumulativeRotation\n\
    \n",
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

  m_cDeltaEventCount++;

  RECT rect;

  GetWindowRect(m_hWnd, &rect);

  int oldWidth = rect.right - rect.left;
  int oldHeight = rect.bottom - rect.top;

  // scale and translate the window size / position    
  MoveWindow(m_hWnd,                                                     // the window to move
    static_cast<int>(rect.left + (translationDeltaX / 100.0f)), // the x position
    static_cast<int>(rect.top + (translationDeltaY / 100.0f)),    // the y position
    static_cast<int>(oldWidth * scaleDelta),                    // width
    static_cast<int>(oldHeight * scaleDelta),                   // height
    TRUE);                                                      // redraw

  return S_OK;
}

HRESULT STDMETHODCALLTYPE CManipulationEventSink::ManipulationCompleted(
  /* [in] */ FLOAT x,
  /* [in] */ FLOAT y,
  /* [in] */ FLOAT cumulativeTranslationX,
  /* [in] */ FLOAT cumulativeTranslationY,
  /* [in] */ FLOAT cumulativeScale,
  /* [in] */ FLOAT cumulativeExpansion,
  /* [in] */ FLOAT cumulativeRotation)
{
  printf("ManipulationCompleted\n");
  m_cCompletedEventCount++;

  m_fX = x;
  m_fY = y;

  // place your code handler here to do any operations based on the manipulation   

  return S_OK;
}


/////////////////////////////////
//Implement IUnknown
/////////////////////////////////

ULONG CManipulationEventSink::AddRef(void)
{
  return ++m_cRefCount;
}

ULONG CManipulationEventSink::Release(void)
{
  m_cRefCount--;

  if (0 == m_cRefCount) {
    delete this;
    return 0;
  }

  return m_cRefCount;
}

HRESULT CManipulationEventSink::QueryInterface(REFIID riid, LPVOID *ppvObj)
{
  if (IID__IManipulationEvents == riid) {
    *ppvObj = (_IManipulationEvents *)(this); AddRef(); return S_OK;
  }
  else if (IID_IUnknown == riid) {
    *ppvObj = (IUnknown *)(this); AddRef(); return S_OK;
  }
  else {
    return E_NOINTERFACE;
  }
}

