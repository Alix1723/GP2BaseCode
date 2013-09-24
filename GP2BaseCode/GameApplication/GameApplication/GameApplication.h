//header guard, so this file is only compiled once
#pragma once

#include <string>

using namespace std;

//Forward decleration of our window interface
class IWindow;

//Structure for holding GameOptions, this will be loaded from config files
struct GameOptionsDesc
{
	wstring gameName;
	int width;
	int height;
	bool fullscreen;
};

//We have this here so we don't need to prefix all standard library types with std::
using namespace std;

//Our game application class
class CGameApplication
{
public:
	CGameApplication(void);
	//virtual deconstructor, so this class can be overridden
	virtual ~CGameApplication(void);
	//virtual function, can be overridden
	virtual bool init();
	void run();
	//virtual function, can be overridden
	virtual void render();
	//virtual function, can be overridden
	virtual void update();
private:
	bool parseConfigFile();
	bool initInput();
	bool initGame();
	bool initGraphics();
	bool initPhysics();

	bool initWindow();
private:
	IWindow * m_pWindow;
	GameOptionsDesc m_GameOptionDesc;
	wstring m_ConfigFileName;
};