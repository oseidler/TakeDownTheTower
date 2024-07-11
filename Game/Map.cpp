#include "Game/Map.hpp"
#include "Game/Encounter.hpp"
#include "Game/EncounterDefinition.hpp"
#include "Game/Game.hpp"
#include "Game/App.hpp"
#include "Game/Player.hpp"
#include "Game/GameCommon.hpp"
#include "Game/SaveManager.hpp"
#include "ThirdParty/Squirrel/SmoothNoise.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Renderer/DebugRenderSystem.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Math/MathUtils.hpp"


//
//constructor and destructor
//
Map::Map(Player* player)
{
	//initialize all encounters
	for (int encounterIndex = 0; encounterIndex < NUM_ENCOUNTERS_DIFFICULTY_0; encounterIndex++)
	{
		EncounterDefinition const* encounterDef = nullptr;
		int randomEasyEncounter;
		do
		{
			randomEasyEncounter = g_rng.RollRandomIntLessThan(static_cast<int>(EncounterDefinition::s_encounterDefs.size()));
			encounterDef = EncounterDefinition::GetEncounterDefinition(randomEasyEncounter);
		} while (encounterDef->m_difficultyLevel != 0);

		m_allEncounters.emplace_back(new Encounter(encounterDef, encounterIndex, player, this));
	}
	for (int encounterIndex = NUM_ENCOUNTERS_DIFFICULTY_0; encounterIndex < ENCOUNTER_DIFFICULTY_1_MAX_INDEX; encounterIndex++)
	{
		EncounterDefinition const* encounterDef = nullptr;
		int randomNormalEncounter;
		do 
		{
			randomNormalEncounter = g_rng.RollRandomIntLessThan(static_cast<int>(EncounterDefinition::s_encounterDefs.size()));
			encounterDef = EncounterDefinition::GetEncounterDefinition(randomNormalEncounter);
		} while (encounterDef->m_difficultyLevel != 1);

		m_allEncounters.emplace_back(new Encounter(encounterDef, encounterIndex, player, this));
	}
	for (int encounterIndex = ENCOUNTER_DIFFICULTY_1_MAX_INDEX; encounterIndex < ENCOUNTER_DIFFICULTY_2_MAX_INDEX; encounterIndex++)
	{
		EncounterDefinition const* encounterDef = nullptr;
		int randomHardEncounter;
		do
		{
			randomHardEncounter = g_rng.RollRandomIntLessThan(static_cast<int>(EncounterDefinition::s_encounterDefs.size()));
			encounterDef = EncounterDefinition::GetEncounterDefinition(randomHardEncounter);
		} while (encounterDef->m_difficultyLevel != 2);

		m_allEncounters.emplace_back(new Encounter(encounterDef, encounterIndex, player, this));
	}

	EncounterDefinition const* bossEncounter = EncounterDefinition::GetEncounterDefinition(static_cast<int>(EncounterDefinition::s_encounterDefs.size()) - 2);
	m_allEncounters.emplace_back(new Encounter(bossEncounter, ENCOUNTER_DIFFICULTY_2_MAX_INDEX, player, this));

	//put secret final boss encounter here
	EncounterDefinition const* finalBossEncounter = EncounterDefinition::GetEncounterDefinition(static_cast<int>(EncounterDefinition::s_encounterDefs.size()) - 1);
	m_allEncounters.emplace_back(new Encounter(finalBossEncounter, ENCOUNTER_DIFFICULTY_2_MAX_INDEX + 1, player, this));
}


Map::~Map()
{
	for (int encounterIndex = 0; encounterIndex < m_allEncounters.size(); encounterIndex++)
	{
		if (m_allEncounters[encounterIndex] != nullptr)
		{
			delete m_allEncounters[encounterIndex];
		}
	}
}


//
//public game flow functions
//
void Map::UpdateRestStop()
{
}


void Map::RenderRestStop() const
{
	//add flickering background
	std::vector<Vertex_PCU> backgroundVerts;
	AABB2 screenBounds = AABB2(0.0f, 0.0f, SCREEN_CAMERA_SIZE_X, SCREEN_CAMERA_SIZE_Y);

	float fireFlicker = Compute1dPerlinNoise(g_theGame->m_gameClock.GetTotalSeconds(), 1.0f, 3, 0.5f, 2.0f, true);
	unsigned char fireAlpha = static_cast<unsigned char>(RangeMapClamped(fireFlicker, -1.0f, 1.0f, 150, 200));
	Rgba8 fireColor = Rgba8(255, 150, 0, fireAlpha);
	if (m_currentEncounterNumber == ENCOUNTER_DIFFICULTY_2_MAX_INDEX)
	{
		fireColor = Rgba8(150, 0, 255, fireAlpha);
	}

	backgroundVerts.push_back(Vertex_PCU(screenBounds.m_mins, fireColor));
	backgroundVerts.push_back(Vertex_PCU(Vec2(screenBounds.m_maxs.x, screenBounds.m_mins.y), fireColor));
	backgroundVerts.push_back(Vertex_PCU(screenBounds.m_maxs, Rgba8(fireColor.r, fireColor.g, fireColor.b, 0)));
	
	backgroundVerts.push_back(Vertex_PCU(screenBounds.m_mins, fireColor));
	backgroundVerts.push_back(Vertex_PCU(screenBounds.m_maxs, Rgba8(fireColor.r, fireColor.g, fireColor.b, 0)));
	backgroundVerts.push_back(Vertex_PCU(Vec2(screenBounds.m_mins.x, screenBounds.m_maxs.y), Rgba8(fireColor.r, fireColor.g, fireColor.b, 0)));

	g_theRenderer->BindTexture(nullptr);
	g_theRenderer->DrawVertexArray(backgroundVerts);
	
	DebugAddScreenText("Take a breather...", Vec2(SCREEN_CAMERA_CENTER_X, SCREEN_CAMERA_SIZE_Y - 25.0f), 50.0f, Vec2(0.5f, 1.0f), 0.0f);

	std::string healthText = Stringf("HP: %i/%i", g_theGame->m_player->m_currentHealth, PLAYER_MAX_HEALTH);
	DebugAddScreenText(healthText, Vec2(SCREEN_CAMERA_CENTER_X, SCREEN_CAMERA_CENTER_Y), 50.0f, Vec2(0.5f, 0.0f), 0.0f, Rgba8(255, 0, 0), Rgba8(255, 0, 0));
}


//public map utilities
void Map::EnterFirstEncounter() const
{
	m_allEncounters[0]->BeginEncounter();
}


void Map::EnterNextEncounter()
{
	if (!m_isRestTime && (m_currentEncounterNumber == NUM_ENCOUNTERS_DIFFICULTY_0 - 1 || m_currentEncounterNumber == ENCOUNTER_DIFFICULTY_1_MAX_INDEX - 1 || m_currentEncounterNumber == ENCOUNTER_DIFFICULTY_2_MAX_INDEX - 1 || m_currentEncounterNumber == ENCOUNTER_DIFFICULTY_2_MAX_INDEX))
	{
		EnterRestStop();
		return;
	}

	g_theAudio->StopSound(g_restStopMusicPlayback);
	g_theAudio->StopSound(g_finalRestStopMusicPlayback);
	g_theAudio->StopSound(g_campfireSoundPlayback);
	
	if (m_currentEncounterNumber < m_allEncounters.size() - 1)
	{
		m_allEncounters[m_currentEncounterNumber]->EndEncounter();
		delete m_allEncounters[m_currentEncounterNumber];
		m_allEncounters[m_currentEncounterNumber] = nullptr;
		m_currentEncounterNumber++;
		m_isRestTime = false;
		m_allEncounters[m_currentEncounterNumber]->BeginEncounter();
		if (m_currentEncounterNumber == NUM_ENCOUNTERS_DIFFICULTY_0)
		{
			g_battle2MusicPlayback = g_theAudio->StartSound(g_battle2Music, true, 0.8f);
		}
		else if (m_currentEncounterNumber == ENCOUNTER_DIFFICULTY_1_MAX_INDEX)
		{
			g_battle3MusicPlayback = g_theAudio->StartSound(g_battle3Music, true, 0.8f);
		}
		else if (m_currentEncounterNumber == ENCOUNTER_DIFFICULTY_2_MAX_INDEX)
		{
			g_bossMusicPlayback = g_theAudio->StartSound(g_bossMusic, true);
		}
		else if (m_currentEncounterNumber == ENCOUNTER_DIFFICULTY_2_MAX_INDEX + 1)
		{
			g_finalBossMusicPlayback = g_theAudio->StartSound(g_finalBossMusic, true);
		}
	}
	else
	{
		g_theGame->m_isVictory = true;
	}
}


void Map::EnterRestStop()
{
	m_isRestTime = true;

	g_saveManager.RecordGameState();

	g_theAudio->StopSound(g_battleMusicPlayback);
	g_theAudio->StopSound(g_battle2MusicPlayback);
	g_theAudio->StopSound(g_battle3MusicPlayback);
	g_theAudio->StopSound(g_bossMusicPlayback);
	if (m_currentEncounterNumber == ENCOUNTER_DIFFICULTY_2_MAX_INDEX)
	{
		g_finalRestStopMusicPlayback = g_theAudio->StartSound(g_finalRestStopMusic, true);
	}
	else
	{
		g_restStopMusicPlayback = g_theAudio->StartSound(g_restStopMusic, true);
	}
	g_campfireSoundPlayback = g_theAudio->StartSound(g_campfireSound, true);
}
