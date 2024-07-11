#include "Game/Game.hpp"
#include "Game/App.hpp"
#include "Game/Player.hpp"
#include "Game/Enemy.hpp"
#include "Game/Map.hpp"
#include "Game/CardDefinition.hpp"
#include "Game/EncounterDefinition.hpp"
#include "Game/EnemyDefinition.hpp"
#include "Game/EffectDefinition.hpp"
#include "Game/SaveManager.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Window/Window.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Renderer/SpriteAnimDefinition.hpp"
#include "Engine/Core/FileUtils.hpp"
#include "Engine/Renderer/DebugRenderSystem.hpp"


//game flow functions
void Game::Startup()
{
	//load game assets
	LoadAssets();
	g_font = g_theRenderer->CreateOrGetBitmapFont("Data/Fonts/SquirrelFixedFont");

	//initialize all definitions
	InitializeDefinitions();
	
	//set camera bounds
	m_screenCamera.SetOrthoView(Vec2(0.f, 0.f), Vec2(SCREEN_CAMERA_SIZE_X, SCREEN_CAMERA_SIZE_Y));
	g_theRenderer->SetModelConstants();

	//create buttons
	Vec2 screenBounds = Vec2(SCREEN_CAMERA_SIZE_X, SCREEN_CAMERA_SIZE_Y);

	AABB2 startButtonBounds = AABB2(SCREEN_CAMERA_CENTER_X - 125.0f, SCREEN_CAMERA_CENTER_Y - 200.0f, SCREEN_CAMERA_CENTER_X + 125.0f, SCREEN_CAMERA_CENTER_Y - 100.0f);
	m_startButton = new Button(g_theRenderer, g_theInput, startButtonBounds, screenBounds, nullptr, "New Game", AABB2(0.1f, 0.1f, 0.9f, 0.9f), Rgba8(75, 0, 0));
	m_startButton->SubscribeToEvent("Start Game", Game::Event_StartGame);

	AABB2 continueButtonBounds = AABB2(SCREEN_CAMERA_CENTER_X - 125.0f, SCREEN_CAMERA_CENTER_Y - 75.0f, SCREEN_CAMERA_CENTER_X + 125.0f, SCREEN_CAMERA_CENTER_Y + 25.0f);
	m_continueButton = new Button(g_theRenderer, g_theInput, continueButtonBounds, screenBounds, nullptr, "Continue", AABB2(0.1f, 0.1f, 0.9f, 0.9f), Rgba8(75, 0, 0));
	m_continueButton->SubscribeToEvent("Continue Game", Game::Event_ContinueGame);

	AABB2 quitButtonBounds = AABB2(SCREEN_CAMERA_CENTER_X - 125.0f, SCREEN_CAMERA_CENTER_Y - 325.0f, SCREEN_CAMERA_CENTER_X + 125.0f, SCREEN_CAMERA_CENTER_Y - 225.0f);
	m_quitButton = new Button(g_theRenderer, g_theInput, quitButtonBounds, screenBounds, nullptr, "Exit Game", AABB2(0.1f, 0.1f, 0.9f, 0.9f), Rgba8(75, 0, 0));
	m_quitButton->SubscribeToEvent("quit", App::Event_Quit);

	AABB2 endTurnButtonBounds = AABB2(SCREEN_CAMERA_SIZE_X - 175.0f, 150.0f, SCREEN_CAMERA_SIZE_X - 50.0f, 200.0f);
	m_endTurnButton = new Button(g_theRenderer, g_theInput, endTurnButtonBounds, screenBounds, nullptr, "End Turn", AABB2(0.15f, 0.15f, 0.85f, 0.85f), Rgba8(30, 30, 30));
	m_endTurnButton->SubscribeToEvent("End Turn", Game::Event_EndTurn);

	AABB2 drawPileButtonBounds = AABB2(25.0f, 15.0f, 105.0f, 95.0f);
	m_drawPileButton = new Button(g_theRenderer, g_theInput, drawPileButtonBounds, screenBounds, nullptr, "", AABB2(), Rgba8(0, 0, 0, 0));
	m_drawPileButton->SubscribeToEvent("View Draw Pile", Game::Event_ViewDrawPile);

	AABB2 discardPileButtonBounds = AABB2(SCREEN_CAMERA_SIZE_X - 105.0f, 15.0f, SCREEN_CAMERA_SIZE_X - 25.0f, 95.0f);
	m_discardPileButton = new Button(g_theRenderer, g_theInput, discardPileButtonBounds, screenBounds, nullptr, "", AABB2(), Rgba8(0, 0, 0, 0));
	m_discardPileButton->SubscribeToEvent("View Discard Pile", Game::Event_ViewDiscardPile);

	/*AABB2 backButtonBounds = AABB2(150.0f, 150.0f, 50.0f, 200.0f);
	m_backButton = new Button(g_theRenderer, g_theInput, backButtonBounds, screenBounds, nullptr, "Back", AABB2(0.1f, 0.1f, 0.9f, 0.9f), Rgba8(75, 0, 0));
	m_backButton->SubscribeToEvent("Return To Gameplay", Game::Event_ReturnToGameplay);*/

	AABB2 skipButtonBounds = AABB2(SCREEN_CAMERA_CENTER_X - 60.0f, SCREEN_CAMERA_CENTER_Y - 225.0f, SCREEN_CAMERA_CENTER_X + 60.0f, SCREEN_CAMERA_CENTER_Y - 175.0f);
	m_skipButton = new Button(g_theRenderer, g_theInput, skipButtonBounds, screenBounds, nullptr, "Skip", AABB2(0.1f, 0.1f, 0.9f, 0.9f), Rgba8(40, 0, 0));
	m_skipButton->SubscribeToEvent("Skip Card", Game::Event_SkipCard);

	AABB2 restButtonBounds = AABB2(SCREEN_CAMERA_CENTER_X - 120.0f, SCREEN_CAMERA_CENTER_Y - 150.0f, SCREEN_CAMERA_CENTER_X + 120.0f, SCREEN_CAMERA_CENTER_Y - 50.0f);
	m_restButton = new Button(g_theRenderer, g_theInput, restButtonBounds, screenBounds, nullptr, "Rest", AABB2(0.1f, 0.1f, 0.9f, 0.9f), Rgba8(40, 0, 0));
	m_restButton->SubscribeToEvent("Player Rest", Game::Event_PlayerRest);

	//check for save file
	m_loadedFile = CheckForFile("Save.bin");

	EnterAttractMode();
}


void Game::Update()
{
	//if in attract mode, just update that and don't bother with anything else
	if (m_isAttractMode)
	{
		UpdateAttract();

		return;
	}
	
	//reset camera bounds
	m_screenCamera.SetOrthoView(Vec2(0.f, 0.f), Vec2(SCREEN_CAMERA_SIZE_X, SCREEN_CAMERA_SIZE_Y));

	//translate camera for screen shake
	if (m_cameraOffsetAmount > 0.0f)
	{
		float offsetX = g_rng.RollRandomFloatInRange(-m_cameraOffsetAmount, m_cameraOffsetAmount);
		float offsetY = g_rng.RollRandomFloatInRange(-m_cameraOffsetAmount, m_cameraOffsetAmount);
		m_screenCamera.Translate2D(Vec2(offsetX, offsetY));
	}

	//reduce screen shake
	if (m_cameraOffsetAmount > 0.1f)
	{
		m_cameraOffsetAmount -= 0.25f;
	}
	else
	{
		m_cameraOffsetAmount = 0.f;
	}

	if (m_isVictory)
	{
		return;
	}

	//speed up control
	if (g_theInput->WasKeyJustPressed('T'))
	{
		if (m_gameClock.GetTimeScale() == 1.0f)
		{
			m_gameClock.SetTimeScale(5.0f);
			std::string debugMessage = "Set time scale to 5";
			DebugAddMessage(debugMessage, 2.5f);
		}
		else
		{
			m_gameClock.SetTimeScale(1.0f);
			std::string debugMessage = "Set time scale to 1";
			DebugAddMessage(debugMessage, 2.5f);
		}
	}

	if (m_map->m_isRestTime)
	{
		m_map->UpdateRestStop();
		m_restButton->Update();
		m_skipButton->Update();
		return;
	}

	Encounter* currentEncounter = m_map->m_allEncounters[m_map->m_currentEncounterNumber];

	//debug control to insta-win encounter
	if (g_theInput->WasKeyJustPressed(KEYCODE_SHIFT))
	{
		currentEncounter->KillAllEnemies();
	}

	//game over
	if (m_player->m_currentHealth == 0)
	{
		DebugAddScreenText("GAME OVER", Vec2(SCREEN_CAMERA_CENTER_X, SCREEN_CAMERA_CENTER_Y), 150.0f, Vec2(0.5f, 0.5f), 0.0f, Rgba8(255, 0, 0), Rgba8(255, 0, 0));

		m_encounterEndTimer -= m_gameClock.GetDeltaSeconds();

		if (m_encounterEndTimer <= 0.0f)
		{
			m_encounterEndTimer = 3.0f;
			m_isFinished = true;
			m_isGameOver = true;
		}

		return;
	}
	//player wins encounter
	if (!currentEncounter->m_cardRewardScreenOpen && currentEncounter->AreAllEnemiesDead())
	{
		DebugAddScreenText("Victory!", Vec2(SCREEN_CAMERA_CENTER_X, SCREEN_CAMERA_CENTER_Y), 150.0f, Vec2(0.5f, 0.5f), 0.0f, Rgba8(0, 255, 0), Rgba8(0, 255, 0));

		m_encounterEndTimer -= m_gameClock.GetDeltaSeconds();

		if (m_encounterEndTimer <= 0.0f)
		{
			m_encounterEndTimer = 3.0f;
			if (currentEncounter->m_encounterNumber == m_map->m_allEncounters.size() - 1)
			{
				m_isVictory = true;
				g_theAudio->StopSound(g_finalBossMusicPlayback);
				g_victoryMusicPlayback = g_theAudio->StartSound(g_victoryMusic);
			}
			else
			{
				currentEncounter->OpenCardRewardScreen();
			}
		}

		return;
	}

	m_player->Update();

	currentEncounter->Update();

	if (!currentEncounter->m_cardRewardScreenOpen)
	{
		if (currentEncounter->m_turnState == TurnState::PLAYER)
		{
			m_endTurnButton->Update();
		}

		m_drawPileButton->Update();
		m_discardPileButton->Update();
	}
	else
	{
		m_skipButton->Update();
	}
}


void Game::Render() const
{
	//if in attract mode, just render that and not anything else
	if (m_isAttractMode)
	{
		RenderAttract();

		return;
	}
	
	g_theRenderer->ClearScreen(Rgba8(65, 65, 65));	//clear screen to dark gray

	g_theRenderer->BeginCamera(m_screenCamera);	//render UI with the screen camera

	if (m_isVictory)
	{
		DebugAddScreenText("YOU WIN!!!", Vec2(SCREEN_CAMERA_CENTER_X, SCREEN_CAMERA_CENTER_Y + 100.0f), 100.0f, Vec2(0.5f, 0.5f), 0.0f, Rgba8(255, 200, 50), Rgba8(255, 200, 50));
		DebugAddScreenText("Press ESC to return to the title screen", Vec2(SCREEN_CAMERA_CENTER_X, SCREEN_CAMERA_CENTER_Y - 100.0f), 35.0f, Vec2(0.5f, 0.5f), 0.0f, Rgba8(255, 200, 50), Rgba8(255, 200, 50));
	}
	else if (m_map->m_isRestTime)
	{
		m_map->RenderRestStop();
		m_restButton->Render();
		m_skipButton->Render();
	}
	else
	{
		Encounter* currentEncounter = m_map->m_allEncounters[m_map->m_currentEncounterNumber];

		currentEncounter->Render();

		//render buttons
		if (!currentEncounter->m_cardRewardScreenOpen)
		{
			m_endTurnButton->Render();
		}
		else
		{
			m_skipButton->Render();
		}
	}

	g_theRenderer->EndCamera(m_screenCamera);

	DebugRenderScreen(m_screenCamera);
}


void Game::Shutdown()
{
	//stop all sounds
	g_theAudio->StopSound(g_battleMusicPlayback);
	g_theAudio->StopSound(g_startMenuMusicPlayback);
	g_theAudio->StopSound(g_bossMusicPlayback);
	g_theAudio->StopSound(g_restStopMusicPlayback);
	g_theAudio->StopSound(g_battle2MusicPlayback);
	g_theAudio->StopSound(g_battle3MusicPlayback);
	g_theAudio->StopSound(g_victoryMusicPlayback);
	g_theAudio->StopSound(g_finalRestStopMusicPlayback);
	g_theAudio->StopSound(g_finalBossMusicPlayback);
	g_theAudio->StopSound(g_finalVictoryMusicPlayback);
	g_theAudio->StopSound(g_campfireSoundPlayback);

	//save progress if there's progress to save
	if (!m_isGameOver && m_player != nullptr)
	{
		g_saveManager.SaveProgress();
	}
	
	//delete all allocated pointers here
	if (m_map != nullptr)
	{
		delete m_map;
	}

	if (m_player != nullptr)
	{
		delete m_player;
	}

	if (m_endTurnButton != nullptr)
	{
		delete m_endTurnButton;
	}

	if (m_quitButton != nullptr)
	{
		delete m_quitButton;
	}

	if (m_continueButton != nullptr)
	{
		delete m_continueButton;
	}

	if (m_startButton != nullptr)
	{
		delete m_startButton;
	}

	if (m_drawPileButton != nullptr)
	{
		delete m_drawPileButton;
	}

	if (m_discardPileButton != nullptr)
	{
		delete m_discardPileButton;
	}

	/*if (m_backButton != nullptr)
	{
		delete m_backButton;
	}*/

	if (m_skipButton != nullptr)
	{
		delete m_skipButton;
	}

	if (m_restButton != nullptr)
	{
		delete m_restButton;
	}
}


//
//public game utilities
//
void Game::BeginScreenShake(float screenShakeAmount)
{
	m_cameraOffsetAmount = screenShakeAmount;
}


//
//public static functions
//
bool Game::Event_EndTurn(EventArgs& args)
{
	UNUSED(args);
	
	Encounter* currentEncounter = g_theGame->m_map->m_allEncounters[g_theGame->m_map->m_currentEncounterNumber];
	currentEncounter->ChangeTurnState(TurnState::ENEMY);

	return true;
}


bool Game::Event_StartGame(EventArgs& args)
{
	UNUSED(args);
	
	g_theGame->EnterGameplay(false);

	return true;
}


bool Game::Event_ContinueGame(EventArgs& args)
{
	UNUSED(args);

	g_theGame->EnterGameplay(true);

	return true;
}


bool Game::Event_ViewDrawPile(EventArgs& args)
{
	UNUSED(args);

	Player* player = g_theGame->m_player;
	for (int cardIndex = 0; cardIndex < player->m_drawPile.size(); cardIndex++)
	{
		std::string cardName = player->m_drawPile[cardIndex]->m_definition->m_name;
		DebugAddMessage(cardName, 5.0f);
	}

	return true;
}


bool Game::Event_ViewDiscardPile(EventArgs& args)
{
	UNUSED(args);

	Player* player = g_theGame->m_player;
	for (int cardIndex = 0; cardIndex < player->m_discardPile.size(); cardIndex++)
	{
		std::string cardName = player->m_discardPile[cardIndex]->m_definition->m_name;
		DebugAddMessage(cardName, 5.0f);
	}

	return true;
}


bool Game::Event_ReturnToGameplay(EventArgs& args)
{
	UNUSED(args);

	std::string mes = "Return to Game";
	DebugAddMessage(mes, 3.0f);

	return true;
}


bool Game::Event_SkipCard(EventArgs& args)
{
	UNUSED(args);
	
	g_theGame->m_map->EnterNextEncounter();

	return true;
}


bool Game::Event_PlayerRest(EventArgs& args)
{
	UNUSED(args);
	
	if (g_theGame->m_player != nullptr)
	{
		g_theGame->m_player->RestoreHealth(REST_HEAL_AMOUNT);
	}

	g_theGame->m_map->EnterNextEncounter();

	return true;
}


//
//game flow sub-functions
//
void Game::UpdateAttract()
{
	m_startButton->Update();
	m_quitButton->Update();

	if (m_loadedFile)
	{
		m_continueButton->Update();
	}
}


void Game::RenderAttract() const
{
	g_theRenderer->ClearScreen(Rgba8(125, 50, 0));	//clear screen to dark reddish color
	
	g_theRenderer->BeginCamera(m_screenCamera);	//render attract screen with the screen camera
	
	DebugAddScreenText("Take Down The Tower", Vec2(SCREEN_CAMERA_CENTER_X, SCREEN_CAMERA_CENTER_Y + 200.0f), 75.0f, Vec2(0.5f, 0.5f), 0.0f, Rgba8(255, 170, 0), Rgba8(255, 170, 0));
	m_startButton->Render();
	m_quitButton->Render();

	if (m_loadedFile)
	{
		m_continueButton->Render();
	}

	g_theRenderer->EndCamera(m_screenCamera);

	DebugRenderScreen(m_screenCamera);
}


//
//mode switching functions
//
void Game::EnterAttractMode()
{
	m_isAttractMode = true;

	g_startMenuMusicPlayback = g_theAudio->StartSound(g_startMenuMusic, true);
}


void Game::EnterGameplay(bool loadFile)
{
	m_isAttractMode = false;

	g_theAudio->StopSound(g_startMenuMusicPlayback);

	bool wasProgressLoaded = false;

	if (loadFile)
	{
		//load progress if there's progress to load
		wasProgressLoaded = g_saveManager.LoadProgress();
	}
	
	if (!wasProgressLoaded)
	{
		//seed rng
		g_rng.SeedRNGWithTime();
		g_rng.m_position = 0;

		//create player and map
		m_player = new Player();
		m_map = new Map(m_player);
		m_map->EnterFirstEncounter();

		g_battleMusicPlayback = g_theAudio->StartSound(g_battleMusic, true);
	}
}


//
//asset management functions
//
void Game::LoadAssets()
{
	LoadSounds();
	LoadTextures();
}


void Game::LoadSounds()
{
	//load sound effects
	g_attackSliceSound = g_theAudio->CreateOrGetSound("Data/Audio/Sounds/Attack_Slice.wav");
	g_attackPierceSound = g_theAudio->CreateOrGetSound("Data/Audio/Sounds/Attack_Pierce.mp3");
	g_attackLightImpactSound = g_theAudio->CreateOrGetSound("Data/Audio/Sounds/Attack_Light_Impact.mp3");
	g_attackHeavyImpactSound = g_theAudio->CreateOrGetSound("Data/Audio/Sounds/Attack_Heavy_Impact.wav");
	g_attackFireSound = g_theAudio->CreateOrGetSound("Data/Audio/Sounds/Attack_Fire.wav");
	g_attackMagicSound = g_theAudio->CreateOrGetSound("Data/Audio/Sounds/Attack_Magic.wav");
	g_damageSound = g_theAudio->CreateOrGetSound("Data/Audio/Sounds/Damage.mp3");
	g_damageBlockedSound = g_theAudio->CreateOrGetSound("Data/Audio//Sounds/Damage_Blocked.wav");
	g_healSound = g_theAudio->CreateOrGetSound("Data/Audio/Sounds/Heal.wav");
	g_cardSound = g_theAudio->CreateOrGetSound("Data/Audio/Sounds/Card_Draw.wav");
	g_campfireSound = g_theAudio->CreateOrGetSound("Data/Audio/Sounds/Rest_Stop_Loop.mp3");
	g_blockSound = g_theAudio->CreateOrGetSound("Data/Audio/Sounds/Block.mp3");
	g_buffSound = g_theAudio->CreateOrGetSound("Data/Audio/Sounds/Buff.mp3");
	g_debuffSound = g_theAudio->CreateOrGetSound("Data/Audio/Sounds/Debuff.mp3");
	//nullify sound

	//load music
	g_startMenuMusic = g_theAudio->CreateOrGetSound("Data/Audio/Music/TitleTheme.mp3");
	g_battleMusic = g_theAudio->CreateOrGetSound("Data/Audio/Music/BattleTheme.mp3");
	g_bossMusic = g_theAudio->CreateOrGetSound("Data/Audio/Music/BossTheme.mp3");
	g_restStopMusic = g_theAudio->CreateOrGetSound("Data/Audio/Music/RestTheme.mp3");
	g_battle2Music = g_theAudio->CreateOrGetSound("Data/Audio/Music/BattleTheme2.mp3");
	g_battle3Music = g_theAudio->CreateOrGetSound("Data/Audio/Music/BattleTheme3.mp3");
	g_victoryMusic = g_theAudio->CreateOrGetSound("Data/Audio/Music/VictoryTheme.mp3");
	g_finalRestStopMusic = g_theAudio->CreateOrGetSound("Data/Audio/Music/RestThemeFinal.mp3");
	g_finalBossMusic = g_theAudio->CreateOrGetSound("Data/Audio/Music/BossThemeFinal.mp3");
	g_finalVictoryMusic = g_theAudio->CreateOrGetSound("Data/Audio/Music/VictoryThemeFinal.mp3");
}


void Game::LoadTextures()
{
	g_playerSprite = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/Player_Ironclad.png");
}


void Game::InitializeDefinitions()
{
	if (EffectDefinition::s_effectDefs.size() == 0)
	{
		EffectDefinition::InitializeEffectDefs();
	}
	if (CardDefinition::s_cardDefs.size() == 0)
	{
		CardDefinition::InitializeCardDefs();
	}
	if (EnemyDefinition::s_enemyDefs.size() == 0)
	{
		EnemyDefinition::InitializeEnemyDefs();
	}
	if (EncounterDefinition::s_encounterDefs.size() == 0)
	{
		EncounterDefinition::InitializeEncounterDefs();
	}
}
