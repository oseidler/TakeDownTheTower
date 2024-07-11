#include "Game/App.hpp"
#include "Game/Game.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Window/Window.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Renderer/DebugRenderSystem.hpp"


App* g_theApp = nullptr;

Renderer*	 g_theRenderer = nullptr;
InputSystem* g_theInput = nullptr;
AudioSystem* g_theAudio = nullptr;
Window*		 g_theWindow = nullptr;

Game* g_theGame = nullptr;


//public game flow functions
void App::Startup()
{
	XmlDocument gameConfigXml;
	char const* filePath = "Data/GameConfig.xml";
	XmlError result = gameConfigXml.LoadFile(filePath);
	GUARANTEE_OR_DIE(result == tinyxml2::XML_SUCCESS, Stringf("Failed to open game config file!"));
	XmlElement* root = gameConfigXml.RootElement();
	g_gameConfigBlackboard.PopulateFromXmlElementAttributes(*root);
	
	EventSystemConfig eventSystemConfig;
	g_theEventSystem = new EventSystem(eventSystemConfig);
	
	InputSystemConfig inputSystemConfig;
	g_theInput = new InputSystem(inputSystemConfig);

	WindowConfig windowConfig;
	windowConfig.m_windowTitle = "Take Down The Tower";
	windowConfig.m_clientAspect = g_gameConfigBlackboard.GetValue("windowAspect", 2.0f);
	windowConfig.m_clientFraction = g_gameConfigBlackboard.GetValue("windowFraction", 1.0f);
	windowConfig.m_inputSystem = g_theInput;
	g_theWindow = new Window(windowConfig);

	RendererConfig rendererConfig;
	rendererConfig.m_window = g_theWindow;
	g_theRenderer = new Renderer(rendererConfig);

	DevConsoleConfig devConsoleConfig;
	devConsoleConfig.m_renderer = g_theRenderer;
	devConsoleConfig.m_camera = &m_devConsoleCamera;
	g_theDevConsole = new DevConsole(devConsoleConfig);
	
	AudioSystemConfig audioSystemConfig;
	g_theAudio = new AudioSystem(audioSystemConfig);
	
	g_theEventSystem->Startup();
	g_theDevConsole->Startup();
	g_theInput->Startup();
	g_theWindow->Startup();
	g_theRenderer->Startup();
	g_theAudio->Startup();

	DebugRenderConfig debugRenderConfig;
	debugRenderConfig.m_renderer = g_theRenderer;
	DebugRenderSystemStartup(debugRenderConfig);

	g_theGame = new Game();
	g_theGame->Startup();

	SubscribeEventCallbackFunction("quit", Event_Quit);

	m_devConsoleCamera.SetOrthoView(Vec2(0.f, 0.f), Vec2(SCREEN_CAMERA_SIZE_X, SCREEN_CAMERA_SIZE_Y));

	g_theDevConsole->AddLine(DevConsole::COLOR_INFO_MAJOR, "------Debug Controls------");
	g_theDevConsole->AddLine(DevConsole::COLOR_INFO_MAJOR, "Shift: Insta-win");
	g_theDevConsole->AddLine(DevConsole::COLOR_INFO_MAJOR, "T: Toggle speed-up");
	g_theDevConsole->AddLine(DevConsole::COLOR_INFO_MAJOR, "");
	g_theDevConsole->AddLine(DevConsole::COLOR_INFO_MAJOR, "------How to Play------");
	g_theDevConsole->AddLine(DevConsole::COLOR_INFO_MAJOR, " - Each turn, you will start by drawing five cards from your draw pile into your hand");
	g_theDevConsole->AddLine(DevConsole::COLOR_INFO_MAJOR, " - On your turn, click on a card to select it, or click on an already selected card to deselect it.");
	g_theDevConsole->AddLine(DevConsole::COLOR_INFO_MAJOR, " - For cards that target one enemy, click on an enemy with the card selected to play it");
	g_theDevConsole->AddLine(DevConsole::COLOR_INFO_MAJOR, " - For all other cards, click anywhere above your hand to play it");
	g_theDevConsole->AddLine(DevConsole::COLOR_INFO_MAJOR, " - Playing cards costs energy, which is displayed in the top-left corner of each card.");
	g_theDevConsole->AddLine(DevConsole::COLOR_INFO_MAJOR, " - Once you no longer have enough energy to play more cards, click the End Turn button to let the enemy's turn commence.");
	g_theDevConsole->AddLine(DevConsole::COLOR_INFO_MAJOR, " - After winning a battle by defeating all enemies, you will get your choice of 1 out of 3 new cards to add to your deck.");
	g_theDevConsole->AddLine(DevConsole::COLOR_INFO_MAJOR, " - Defeat the boss at the end of the sequence of battles to win the game.");
	g_theDevConsole->AddLine(DevConsole::COLOR_INFO_MAJOR, "");
	g_theDevConsole->AddLine(DevConsole::COLOR_INFO_MAJOR, "Use the command \"effects\" to learn what each effect does");

	SubscribeEventCallbackFunction("effects", Event_PrintEffects);
}


void App::Run()
{
	while (!IsQuitting())
	{
		RunFrame();
	}
}


void App::Shutdown()
{
	g_theGame->Shutdown();
	delete g_theGame;
	g_theGame = nullptr;

	DebugRenderSystemShutdown();

	g_theAudio->Shutdown();
	delete g_theAudio;
	g_theAudio = nullptr;

	g_theRenderer->Shutdown();
	delete g_theRenderer;
	g_theRenderer = nullptr;

	g_theWindow->Shutdown();
	delete g_theWindow;
	g_theWindow = nullptr;

	g_theInput->Shutdown();
	delete g_theInput;
	g_theInput = nullptr;

	g_theDevConsole->Shutdown();
	delete g_theDevConsole;
	g_theDevConsole = nullptr;

	g_theEventSystem->Shutdown();
	delete g_theEventSystem;
	g_theEventSystem = nullptr;
}


void App::RunFrame()
{
	//tick the system clock
	Clock::TickSystemClock();

	//run through the four parts of the frame
	BeginFrame();
	Update();
	Render();
	EndFrame();
}


//
//public app utilities
//
bool App::HandleQuitRequested()
{
	m_isQuitting = true;
	return m_isQuitting;
}


//
//static app utilities
//
bool App::Event_Quit(EventArgs& args)
{
	UNUSED(args);

	if (g_theApp != nullptr)
	{
		g_theApp->HandleQuitRequested();
	}

	return true;
}


bool App::Event_PrintEffects(EventArgs& args)
{
	UNUSED(args);

	g_theDevConsole->AddLine(DevConsole::COLOR_INFO_MAJOR, "------List of Effects------");
	g_theDevConsole->AddLine(DevConsole::COLOR_INFO_MAJOR, "Buffs:");
	g_theDevConsole->AddLine(DevConsole::COLOR_INFO_MAJOR, " - Strength: Increases damage dealt by 1 per level. Lasts until the end of battle.");
	g_theDevConsole->AddLine(DevConsole::COLOR_INFO_MAJOR, " - Dexterity: Increases block gained by 1 per level. Lasts until the end of battle.");
	g_theDevConsole->AddLine(DevConsole::COLOR_INFO_MAJOR, " - Artifact: Blocks one debuff per level. Wears off when all levels are used up.");
	g_theDevConsole->AddLine(DevConsole::COLOR_INFO_MAJOR, "");
	g_theDevConsole->AddLine(DevConsole::COLOR_INFO_MAJOR, "Debuffs:");
	g_theDevConsole->AddLine(DevConsole::COLOR_INFO_MAJOR, " - Vulnerable: Increases damage taken by 50%. Decreases by 1 level at the end of each turn; wears off after reaching 0.");
	g_theDevConsole->AddLine(DevConsole::COLOR_INFO_MAJOR, " - Weak: Decreases damage dealt by 25%. Decreases by 1 level at the end of each turn; wears off after reaching 0.");
	g_theDevConsole->AddLine(DevConsole::COLOR_INFO_MAJOR, " - Frail: Decreases block gained by 25%. Decreases by 1 level at the end of each turn; wears off after reaching 0.");

	return true;
}


//
//private game flow functions
//
void App::BeginFrame()
{
	g_theEventSystem->BeginFrame();
	g_theDevConsole->BeginFrame();
	g_theInput->BeginFrame();
	g_theWindow->BeginFrame();
	g_theRenderer->BeginFrame();
	g_theAudio->BeginFrame();

	DebugRenderBeginFrame();
}


void App::Update()
{
	//quit or leave attract mode if q is pressed
	if (g_theInput->WasKeyJustPressed(KEYCODE_ESC))
	{
		if (g_theGame->m_isAttractMode)
		{
			HandleQuitRequested();
		}
		else
		{
			RestartGame();
		}
	}

	//recreate game if f8 is pressed
	if (g_theInput->WasKeyJustPressed(KEYCODE_F8))
	{
		RestartGame();
	}

	//update the game
	g_theGame->Update();

	//go back to the start if the game finishes
	if (g_theGame->m_isFinished)
	{
		RestartGame();
	}
}


void App::Render() const
{	
	g_theGame->Render();

	//render dev console separately from and after rest of game
	g_theRenderer->BeginCamera(m_devConsoleCamera);
	g_theDevConsole->Render(AABB2(0.0f, 0.0f, SCREEN_CAMERA_SIZE_X * 0.9f, SCREEN_CAMERA_SIZE_Y * 0.9f));
	g_theRenderer->EndCamera(m_devConsoleCamera);
}


void App::EndFrame()
{
	g_theEventSystem->EndFrame();
	g_theDevConsole->EndFrame();
	g_theInput->EndFrame();
	g_theWindow->EndFrame();
	g_theRenderer->EndFrame();
	g_theAudio->EndFrame();

	DebugRenderEndFrame();
}


//
//private app utilities
//
void App::RestartGame()
{
	//delete old game
	g_theGame->Shutdown();
	delete g_theGame;
	g_theGame = nullptr;

	//clear debug rendering
	DebugRenderClear();

	//initialize new game
	g_theGame = new Game();
	g_theGame->Startup();
}
