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

void mm_gesture_begin();
void mm_gesture_end();

///////////////////////////////////
//Implement IManipulationEvents
///////////////////////////////////

HRESULT STDMETHODCALLTYPE CManipulationEventSink::ManipulationStarted(
	/* [in] */ FLOAT x,
	/* [in] */ FLOAT y)
{
	m_cStartedEventCount++;
	mm_gesture_begin();
	return S_OK;
}

void mm_gesture(
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
	/* [in] */ FLOAT cumulativeRotation);
extern HWND hWnd;
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
	POINT p = { x / 100, y / 100 };
	ScreenToClient(hWnd, &p);
	mm_gesture(p.x,
		p.y,
		translationDeltaX / 100,
		translationDeltaY / 100,
		scaleDelta,
		expansionDelta,
		rotationDelta,
		cumulativeTranslationX / 100,
		cumulativeTranslationY / 100,
		cumulativeScale,
		cumulativeExpansion,
		cumulativeRotation

		);

	m_cDeltaEventCount++;
	// redraw

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
	m_cCompletedEventCount++;

	m_fX = x;
	m_fY = y;

	// place your code handler here to do any operations based on the manipulation   
	mm_gesture_end();
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

