#define _CRT_SECURE_NO_WARNINGS
#include "VoiceInput.h"

#import <VoiceInputCom.tlb>
#include <thread>
#include <vector>
#include <set>
#include <algorithm>
//#include <menu.h>
//#include <sys_stuff.h>

extern "C" {
	void birdview();
	void shoot(int ani);
	void Key(int key, int modifiers);
	int menu_on;
	//extern menuType;
	//struct menu_struct;
	//menu_struct g_act_menu;
}

enum {
	KSYM_UP = 257,
	KSYM_DOWN,
	KSYM_LEFT,
	KSYM_RIGHT,
	KSYM_PAGE_DOWN,
	KSYM_PAGE_UP,
	KSYM_F1,
	KSYM_F2,
	KSYM_F3,
	KSYM_F4,
	KSYM_F5,
	KSYM_F6,
	KSYM_F7,
	KSYM_F8,
	KSYM_F9,
	KSYM_F10,
	KSYM_F11,
	KSYM_F12,
	KSYM_KP_ENTER
};
//#include "billard3d.c"


/*
VoiceInput::VoiceInput()
{
}


VoiceInput::~VoiceInput()
{
}
*/

extern "C" {
	void voiceRecog_init() {
		CoInitialize(NULL);
		VoiceInputCom::IVoiceInputComObjectPtr p(__uuidof(VoiceInputCom::VoiceInputComObject));

		std::thread{ &VoiceInputCom::IVoiceInputComObject::init, p, true }.detach();
		Sleep(100);
		std::thread{&VoiceInputCom::IVoiceInputComObject::init, p, false}.detach();
		
		std::thread fetchT(fetchThread);
		fetchT.detach();
		//std::string a = p->hey();
	}
}


extern std::string lastRecognized, lastAction;
extern std::vector<std::string> commands;
extern bool showHelp, putHereDesired;
//g_act_menu != 0
void fetchThread(){
	CoInitialize(NULL);
	VoiceInputCom::IVoiceInputComObjectPtr p(__uuidof(VoiceInputCom::VoiceInputComObject));
	std::set<std::string> commandList;
	//list has to be equal to command list in c# Program.cs
	
	commands.push_back("shoot");
	commands.push_back("hit");
	commands.push_back("push");
	//commands.push_back("should");
	
	commands.push_back("stronger");
	commands.push_back("much stronger");
	commands.push_back("weaker");
	commands.push_back("much weaker");

	commands.push_back("que");
	//commands.push_back("Q.");//cue

	commands.push_back("birdview");
	commands.push_back("menu");
	commands.push_back("down");
	commands.push_back("up");
	commands.push_back("select");

	commands.push_back("commands");
	commands.push_back("help");
    commands.push_back("what can I say");

    commands.push_back("put here");

	commands.push_back("revert"); 
	commands.push_back("undo");
	
	time_t t;

	while (true){
		time(&t);
		char* timeStr = ctime(&t);
		int timeLen = strlen(timeStr);

		std::string unrecognized = p->getLastSpokenPhrase();
		//if ((!unrecognyized.empty()) && !(std::find(commands.begin(), commands.end(), unrecognized) != commands.end())){
		if ((!unrecognized.empty())){
			printf("%.*s>>>Last Voice Input: %s \n", timeLen-1, timeStr, unrecognized.c_str());
			lastRecognized = unrecognized;
		}


		std::string input=p->getLastCommand();
		if (!input.empty()){
			if (input == "shoot" || input == "hit" || input == "push"){
				printf("%.*s>>>Command: shoot (%s) \n", timeLen - 1, timeStr, input.c_str());
				lastAction = "Shooting!";
				shoot(0);
				//birdview();
			}
			else if (input=="que"){
				printf("%.*s>>>Command: que (%s) \n", timeLen - 1, timeStr, input.c_str());
				lastAction = "que!";
				Key(KSYM_F3,0);

			}
			else if (input=="birdview"){
				printf("%.*s>>>Command: birdview (%s) \n", timeLen - 1, timeStr, input.c_str());
				lastAction = "bird!";
				Key(KSYM_F2, 0);

            }
            else if (input == "put here"){
                printf("%.*s>>>Command: put here (%s) \n", timeLen - 1, timeStr, input.c_str());
                lastAction = "put here!";
                putHereDesired = true;
            }
			else if (input=="menu"){
				printf("%.*s>>>Command: menu (%s) \n", timeLen - 1, timeStr, input.c_str());
				lastAction = "menu!";
				Key(27, 0); // esc
			}

			else if (input=="down"){
				printf("%.*s>>>Command: down (%s) \n", timeLen - 1, timeStr, input.c_str());
				lastAction = "down!";
				Key(KSYM_DOWN, 0);
			}

			else if (input == "weaker"){
				printf("%.*s>>>Command: weaker (%s) \n", timeLen - 1, timeStr, input.c_str());
				lastAction = "weaker!";
				for (int i = 0; i < 4*2; i++){
					Key(KSYM_DOWN, 0);
				}
			}

			else if (input == "much weaker"){
				printf("%.*s>>>Command: much weaker (%s) \n", timeLen - 1, timeStr, input.c_str());
				lastAction = "much weaker!";
				for (int i = 0; i < 9*3; i++){
					Key(KSYM_DOWN, 0);
				}
			}

			else if (input == "up"){
				printf("%.*s>>>Command: up (%s) \n", timeLen - 1, timeStr, input.c_str());
				lastAction = "up!";
				Key(KSYM_UP, 0);
			}

			else if (input == "stronger"){
				printf("%.*s>>>Command: stronger (%s) \n", timeLen - 1, timeStr, input.c_str());
				lastAction = "stronger!";
                for (int i = 0; i < 4 * 2; i++){
					Key(KSYM_UP, 0);
				}
			}

			else if (input == "much stronger"){
				printf("%.*s>>>Command: much stronger (%s) \n", timeLen - 1, timeStr, input.c_str());
				lastAction = "much stronger!";
				for (int i = 0; i < 9*3; i++){
					Key(KSYM_UP, 0);
				}
			}

			else if (input=="select"){
				printf("%.*s>>>Command: select (%s) \n", timeLen - 1, timeStr, input.c_str());
				lastAction = "select!";
				Key(13, 0);
			}

			else if (input == "commands" || input == "help" || input == "what can I say"){
				printf("%.*s>>>Command: commands (%s) \n", timeLen - 1, timeStr, input.c_str());
				lastAction = "commands!";
				showHelp = !showHelp;

			}

			else if (input == "revert" || input == "undo"){
				printf("%.*s>>>Command: commands (%s) \n", timeLen - 1, timeStr, input.c_str());
				lastAction = "revert!";
				Key('u', 0);

			}

			Sleep(2000);

		}
	}
}