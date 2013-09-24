#include "GameApplication.h"

//should really chweck to see if we are on a windows platform
#include "../Window/Win32Window.h"

//Constructor
CGameApplication::CGameApplication(void)
{
	//Set to Null
	m_pWindow=NULL;
	//Set the window name to GP"
	m_GameOptionDesc.gameName=TEXT("GP2");
	//Window height and width
	m_GameOptionDesc.width = 640;
	m_GameOptionDesc.height = 480;
	//Fullscreen
	m_GameOptionDesc.fullscreen = false;
	//Config Options
	m_ConfigFileName = TEXT("game.cfg");
}

//Desconscructor
CGameApplication::~CGameApplication(void)
{
	//Delete things in reverse order
	if(m_pRenderer)
	{
		delete m_pRenderer;
		m_pRenderer=NULL;
	}
	if(m_pWindow)
	{
		delete m_pWindow;
		m_pWindow = NULL;
	}
}

//Init
//This initialises all subsystems
bool CGameApplication::init()
{
	if(!parseConfigFile())
		return false;
	if(!initWindow())
		return false;
	if(!initGraphics())
		return false;
	if(!initInput())
		return false;
	if(!initGame())
		return false;
	return true;
}

//Called to parse the config file
bool CGameApplication::parseConfigFile()
{
	return true;
}

//initInput() - initialises the input
bool CGameApplication::initPhysics()
{
	return true;
}

//initPhysics() - initialises the physics system
bool CGameApplication::initPhysics()
{
	return true;
}

//initGraphics() - initialises the graphics subsystem
bool CGameApplication::initGraphics()
{
	return true;
}

//initWindow() - initialises the physics system
bool CGameApplication::initWindow()
{
	//Create a Win32 Window
	m_pWindow=new CWin32Window();
	m_pWindow->init(m_GameOptionDesc.gameName,m_GameOptionDesc.width,m_GameOptionDesc.height,m_GameOptionDesc.fullscreen);

	return true;
}

//called to init the game elements
bool CGameApplication::initGame()
{
	return true;
}	