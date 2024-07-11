#pragma once
#include "Game/GameCommon.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Input/Button.hpp"


//forward declaration
class Player;
class Map;


class Game 
{
//public member functions
public:
	//game flow functions
	void Startup();
	void Update();
	void Render() const;
	void Shutdown();

	//game utilities
	void BeginScreenShake(float screenShakeAmount);

	//static functions
	static bool Event_EndTurn(EventArgs& args);
	static bool Event_StartGame(EventArgs& args);
	static bool Event_ContinueGame(EventArgs& args);
	static bool Event_ViewDrawPile(EventArgs& args);
	static bool Event_ViewDiscardPile(EventArgs& args);
	static bool Event_ReturnToGameplay(EventArgs& args);
	static bool Event_SkipCard(EventArgs& args);
	static bool Event_PlayerRest(EventArgs& args);

//public member variables
public:
	//game state bools
	bool		m_isFinished = false;
	bool		m_isAttractMode = true;
	bool		m_isVictory = false;
	bool		m_isGameOver = false;

	//buttons
	Button* m_startButton;
	Button* m_continueButton;
	Button* m_quitButton;
	Button* m_endTurnButton;
	Button* m_drawPileButton;
	Button* m_discardPileButton;
	//Button* m_backButton;
	Button* m_skipButton;
	Button* m_restButton;

	//game clock
	Clock m_gameClock = Clock();

	//game actors
	Player* m_player = nullptr;
	Map*	m_map = nullptr;

//private member functions
private:
	//game flow sub-functions
	void UpdateAttract();
	void RenderAttract() const;

	//mode-switching functions
	void EnterAttractMode();
	void EnterGameplay(bool loadFile);

	//asset management functions
	void LoadAssets();
	void LoadSounds();
	void LoadTextures();
	void InitializeDefinitions();

//private member variables
private:
	//camera variables
	Camera m_screenCamera;
	float m_cameraOffsetAmount = 0.f;

	float m_encounterEndTimer = 3.0f;
	
	bool m_loadedFile = false;
};
