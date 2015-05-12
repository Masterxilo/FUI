#include "VoiceInput.h"

#import <VoiceInputCom.tlb>
#include <thread>
#include <vector>
#include <set>
#include <algorithm>
//#include <sys_stuff.h>

extern "C" {
	void birdview();
	void shoot(int ani);
	void Key(int key, int modifiers);
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

		//bool val = p->wordRecognized();
		std::thread t(&VoiceInputCom::IVoiceInputComObject::init, p);
		t.detach();

		std::thread fetchT(fetchThread);
		fetchT.detach();
		//std::string a = p->hey();
	}
}


extern std::string lastRecognized, lastAction;

void fetchThread(){
	CoInitialize(NULL);
	VoiceInputCom::IVoiceInputComObjectPtr p(__uuidof(VoiceInputCom::VoiceInputComObject));
	std::set<std::string> commandList;
	//list has to be equal to command list in c# Program.cs
	

	std::vector<std::string> commands;
	commands.push_back("shoot");
	//commands.push_back("children");
	//commands.push_back("should");
	
	commands.push_back("que");
	commands.push_back("Q.");//cue
	commands.push_back("birdview");
	commands.push_back("menu");
	commands.push_back("down");
	commands.push_back("up");
	commands.push_back("select");
	

	while (true){
		std::string unrecognized = p->getLastInvalidCommand();
		if ((!unrecognized.empty()) && !(std::find(commands.begin(), commands.end(), unrecognized) != commands.end())){
			printf("Unrecognized command: %s", unrecognized.c_str());
			lastRecognized = unrecognized;
		}


		std::string input=p->getLastCommand();
		if (!input.empty()){
			//if (!(p->getRecognizedCommands()[0]=="shoot"))
				//int a =5;
			//if (input == "shoot" || input == "children" || input == "should"){
			if (input == "shoot"){
				printf("HeyheyHey");
				lastAction = "Shooting!";
				shoot(0);
				//birdview();
			}
			else if (input.compare("que") == 0 || input.compare("Q.") == 0){
				printf("ready now");
				lastAction = "que!";
				Key(KSYM_F3,0);

			}
			else if (input.compare("birdview") == 0){
				printf("ready now");
				lastAction = "bird!";
				Key(KSYM_F2, 0);

			}
			else if (input.compare("menu") == 0){
				printf("ready now");
				lastAction = "menu!";
				Key(27, 0); // esc
			}

			else if (input.compare("down") == 0){
				printf("ready now");
				lastAction = "down!";
				Key(KSYM_DOWN, 0);
			}

			else if (input.compare("up") == 0){
				printf("ready now");
				lastAction = "up!";
				Key(KSYM_UP, 0);
			}

			else if (input.compare("select") == 0){
				printf("ready now");
				lastAction = "select!";
				Key(13, 0);
			}

			Sleep(2000);

		}
	}
}