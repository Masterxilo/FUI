// --player1 human --player2 ai
#include "myinclude.h"
#include "mm.h"
#include <Windows.h>
#include <windowsx.h>
#include <stdio.h>
#include <vector>
#include <iostream>
#include <map>
#define _USE_MATH_DEFINES
#include <math.h>
#include <gl/GL.h>
using namespace std;

bool allowDebug = 1;

// External access
extern "C" {

	int touchmode = 0;

	extern GLfloat Xrot_offs, Yrot_offs, Zrot_offs;

	void draw_circle(float x, float y, float radius);

	void draw_rect(int x, int y, int w, int h);

	void draw_text(int x, int y, const char* s, int height);
	extern int queue_view;
	void
		toggle_queue_view();
	extern int options_free_view_on;

	GLdouble modelview[16];
	GLdouble projection[16];
	GLint viewport[4];

	extern GLfloat cam_dist_aim; // Camera distance to smoothly transition to

	// TODO find camera rotation to smoothly transition to?

	extern GLfloat Xrot; // height, -90 to 0 ?
	extern GLfloat Yrot; // unused
	extern GLfloat Zrot; // rotation around up axis, full 360

	int doingTouchGesture = 0; // unused


	extern int win_width;
	extern int win_height;
	extern int g_shot_due;
	void shoot(int ani);
	// Moing the cueball
#define CUE_BALL_IND (player[act_player].cue_ball)
#include "vmath.h"
#include "billmove.h"
#include "player.h"
	extern struct Player player[2];
	extern void ball_displace_clip(VMvect * cue_pos, VMvect offs);
	extern int  act_player;
	extern BallsType balls;
	extern int balls_moving;

#define player_placing_cue_ball (player[act_player].place_cue_ball)
#define placing_cue_ball (player[act_player].place_cue_ball && !balls_moving)
#define queue_strength (player[act_player].strength)
	extern
		double strength01(double value);

#define CUEBALLPOS balls.ball[CUE_BALL_IND].r

#include "sys_stuff.h"
	void MouseEvent(MouseButtonEnum button, MouseButtonState  state, int x, int y, int key_modifiers);
	void Key(int key, int modifiers);

	extern int MouseEventEnabled;
}

void shoot_now(double strengthIn01) {

	queue_strength = strength01(strengthIn01);
	if ((!player[act_player].is_net) && (!player[act_player].is_AI)){
		g_shot_due = 0;
		shoot(!queue_view);
	}
}

GLfloat old_cam_dist_aim, old_Xrot, old_Zrot; // unused

// Adjust!
int swipeThreasholdDistanceInPixels = 180,
swipeThresholdTimeInMs = 200;

// == Projection

#include <gl/GLU.h>


void toScreen(Vect pos, double* x, double* y) {
	// not changing, same problem:
	// http://www.gamedev.net/topic/114786-bogus-matrix-returned-with-glgetdoublev/


	double z;
	gluProject(pos.x, pos.y, pos.z, modelview, projection, viewport,
		x, y, &z);

	*y = (double)win_height - *y;
}
// http://nehe.gamedev.net/article/using_gluunproject/16013/
// http://www.google.ch/url?sa=t&rct=j&q=&esrc=s&source=web&cd=1&cad=rja&uact=8&ved=0CB4QFjAA&url=http%3A%2F%2Fmyweb.lmu.edu%2Fdondi%2Fshare%2Fcg%2Funproject-explained.pdf&ei=SvFMVdmsFYm4UauXgPAI&usg=AFQjCNG4seNb1U-yrCDYzPJJ3t2ocMN3FQ&sig2=rR64KsTFGbWzxbPLRPFJYQ&bvm=bv.92765956,d.d24
void unproject(float winX, float winY, float winZ,
	double* posX, double* posY, double* posZ
	) {
	winY = (float)viewport[3] - winY;           // Subtract The Current Mouse Y Coordinate From The Screen Height.

	gluUnProject(winX, winY, winZ, modelview, projection, viewport,
		posX, posY, posZ);
}

// confirm region
int cx = 0, cy = 0, cw = 300, ch = 100;

// Ball region
double ballrad = 150;
#include <string>
std::string lastRecognized, lastAction;
extern "C" {

	void mm_draw_2d() {
		cw = 400, ch = 60;
		cx = win_width / 2 - cw / 2;
		cy = win_height / 4 - ch / 2;

		draw_text(10,10,"",40); //doesn't draw the circle around the balls otherwise
		draw_text(20, 100, ("You said: " + lastRecognized).c_str(), 40);
		draw_text(20, 120, ("Last voice action: " + lastAction).c_str(), 40);
		if (touchmode)
		if (placing_cue_ball) {
			draw_text(cx + cw / 2 - 60, cy + ch - 10, "Put here!", 40);

			draw_rect(cx, cy, cw, ch);
		}
		else {

			double x, y;
			toScreen(CUEBALLPOS, &x, &y);
			draw_circle(x, y, ballrad);
		}

		// draw balls
		/*
		for (int i = 0; i < balls.nr; i++) if (balls.ball[i].in_game){

		double x, y;
		toScreen(balls.ball[i].r, &x, &y);

		draw_circle(x, y, ballrad);

		}*/
	}
}
// == State machine for touch gesture position and kind
// Classification
enum State {
	NoState,
	Cm, // Camera move
	Cz, // camera zoom
	As, // Swipe area
	Ab  // Ball area
	// TODO change identifier names 
};
// Where the last touch gesture was started (or with how many fingers)
State state = NoState;

// Determine whether we are in
float len(int x, int y);
bool onBallWithDist(POINT& p) {

	double x, y;
	toScreen(CUEBALLPOS, &x, &y);
	//dprintf("current cueball screenpos %f %f\n", x, y);

	return len(x - p.x, y - p.y) < ballrad;
}
// Ball region
bool inAb(POINT* p) {
	return placing_cue_ball; // the whole screen && onBallWithDist(*p);
}

// Swipe region
bool inAs(POINT* p) {
	return !placing_cue_ball && onBallWithDist(*p);
}


// Confirm button region
bool inConfirmRegion(POINT* p) {
	return placing_cue_ball &&
		p->x > cx && p->y > cy
		&& p->x < cx + cw
		&& p->y < cy + ch;
}

double intersectPlane(double oz, double dz, double planez) {
	if (dz == 0) dz = 1e-10;
	return (planez - oz) / dz;
}
// 
Vect getNewBallPosition(int screenx, int screeny) {
	double ox, oy, oz;
	unproject(screenx, screeny, 0, &ox, &oy, &oz); // at znear
	double tx, ty, tz;
	unproject(screenx, screeny, 1, &tx, &ty, &tz); // at zfar
	double dx = tx - ox,
		dz = tz - oz,
		dy = ty - oy;

	double planez = 0;
	double t = intersectPlane(oz, dz, planez);

	Vect v = { ox + dx * t,
		oy + dy * t,
		oz + dz * t };
	return v;
}
#include <algorithm>
template<typename T>
void clamp(T& v, T a, T b) {
	if (a > b) swap(a, b);
	v = min(max(v, a), b);
}

POINT p1_; // old p1
// Called when the touchpoints moved but still have the same ids (fingers)
// as they did when the config was last changed
void move(POINT* p1, POINT* p2) {
	POINT p1d;
	if (p1) {
		p1d.x = p1->x - p1_.x; p1d.y = p1->y - p1_.y;
	}
	if (state == Ab) {
		dprintf("moving ball to %d %d\n", p1->x, p1->y);


		/*
		Vect v = {p1d.x, p1d.y, 0};
		float f = 100;
		v.x /= f; // right
		v.y /= f; // forward
		v.z /= f; // up-down
		// ball_displace_clip(&(CUEBALLPOS), v);
		v = CUEBALLPOS;
		dprintf("new ball pos %f %f %f\n", v.x, v.y, v.z);
		*/
		Vect v = getNewBallPosition(p1->x, p1->y);
		v.x -= CUEBALLPOS.x;
		v.y -= CUEBALLPOS.y;
		ball_displace_clip(&(CUEBALLPOS), v);
		// CUEBALLPOS = v;
		dprintf("new ball pos unproj %f %f %f\n", v.x, v.y, v.z);
		//  CUEBALLPOS = v;
	}
	else if (state == Cm) {
		dprintf("moving camera to %d %d\n", p1->x, p1->y);
		MouseEventEnabled = 1;
		/*
		Xrot += p1d.y; // old_Xrot + cumulativeRotation; // up down
		clamp( Xrot, -90.f,0.f);

		Zrot += p1d.x; // old_Zrot - cumulativeRotation*30;

		dprintf("angle %f %f\n", Xrot, Zrot);*/
	}
	else
		dprintf("move called while in state %d?\n", state);

	if (p1)
		p1_ = *p1;
}

void inputConfigChanges(POINT* p1, POINT* p2) {
	MouseEventEnabled = 1;
	dprintf("input config changes\n");
	if (p1) {
		dprintf("p1 %d %d\n", p1->x, p1->y);
		p1_ = *p1;
	}
	if (p2)
		dprintf("p2 %d %d\n", p2->x, p2->y);

	// State transition table
	if (p1 == p2 && p1 == 0) {
		state = NoState; return;
	}
	if (p2 != 0) {
		state = Cz; // Two fingers is zooming

		MouseEventEnabled = 0;
		dprintf("zoom possible\n");
		return;
	}

	if (inConfirmRegion(p1)) {
		MouseEventEnabled = 0;
		// disable ball move
		if (placing_cue_ball) {
			if (!queue_view) Key('c', 0); // disable queue view
			if (options_free_view_on) Key('f', 0); // disable free view

			// Xrot_offs = Zrot_offs = 0.0; // too jumpy
			player_placing_cue_ball = 0;
			//            placing_cue_ball = 0;
		}
		return;
	}

	// Ballmove
	if (inAb(p1)) {
		MouseEventEnabled = 0;
		state = Ab;

		if (queue_view) Key('c', 0); // enable queue view
		if (!options_free_view_on) Key('f', 0);
		dprintf("ballmove possible\n");
		return;
	}

	if (inAs(p1)) {
		MouseEventEnabled = 0;
		state = As;
		dprintf("swipe possible\n");
		return;
	}
	dprintf("camera move possible\n");
	state = Cm;
}

void sendkey(int vk) {
	INPUT in = { 0 };
	in.type = INPUT_KEYBOARD;
	in.ki.wVk = vk;
	SendInput(1, &in, sizeof(in)); // todo: only if ball is not moving
	in.ki.dwFlags = KEYEVENTF_KEYUP;
	SendInput(1, &in, sizeof(in)); // todo: only if ball is not moving

}



extern "C" {
	void mm_placing_cue_ball() {
		// needed?
	}

}
// A swipe gesture was detected
void mm_swipe(double xfrom, double yfrom,
	double xto, double yto,
	double angle,
	double length,
	double timeInMs
	) {
	dprintf("swipe (%f %f)->(%f %f), angle %f, length %f, time %f\n",
		xfrom, yfrom, xto, yto, angle, length, timeInMs);

	if (state == As){
		dprintf("swiped from swipe area\n");
		if (angle < 0) { // upwards
			double s = (length - swipeThreasholdDistanceInPixels);
			s /= timeInMs;
			dprintf("=========== s %f\n", s);
			clamp(s, 0.05, 1.0);
			shoot_now(s);
		}
	}
}

void mm_gesture_begin() {
	dprintf("ManipulationStarted\n");
	old_cam_dist_aim = cam_dist_aim;
	old_Xrot = Xrot;
	old_Zrot = Zrot;
}


void mm_gesture_end() {
	dprintf("ManipulationCompleted\n");
}

// Motion processor detected and parsed out gesture.
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

	if (state == Cz) {
		dprintf("zooming\n");
		cam_dist_aim = old_cam_dist_aim * (1.0 / cumulativeScale);
	}
	//Xrot = old_Xrot + cumulativeRotation; // up down
	//Zrot = old_Zrot - cumulativeRotation*30;
	if (0)
		dprintf("(%f %f) (%f %f) %f %f %f total: (%f %f) %f %f %f\n",
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

// Infrastructure for getting our windows messages

/****************************************************************
WH_MSGFILTER hook procedure
****************************************************************/
HHOOK hhook;
HWND hWnd;

LRESULT onTouch(HWND hWnd, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK MessageProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	// process event here
	if (nCode == HC_ACTION) {
		CWPSTRUCT* msg = (CWPSTRUCT*)lParam;
		if (msg->hwnd == hWnd) {
			if (msg->message == WM_TOUCH)
				onTouch(msg->hwnd, msg->wParam, msg->lParam);
		}
	}
	return CallNextHookEx(NULL, nCode, wParam, lParam);
}

void touchInit(HWND);
extern "C" {
	void mm_init() {
		// Hook wndproc
		hWnd = FindWindowA(0, "FooBillard");
		hhook = SetWindowsHookEx(
			// When exactly is this called?
			// Right before the application's (windows)
			// Window proc?
			// Can we drop messages?
			WH_CALLWNDPROC,
			MessageProc,
			GetModuleHandle(0),//hinstDLL,
			GetCurrentThreadId());

		// Debug output
		if (allowDebug) {
			AllocConsole();
			freopen("CONOUT$", "w", stdout);
			dprintf("hwnd %d, hhook %d\n", hWnd, hhook);
		}
		touchInit(hWnd);

		if (!touchmode)
			MouseEventEnabled = 1;
	}

}