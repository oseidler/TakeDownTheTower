#include "Game/SaveManager.hpp"
#include "Game/Game.hpp"
#include "Game/Map.hpp"
#include "Game/Player.hpp"
#include "Game/GameCommon.hpp"
#include "Game/App.hpp"
#include "Game/CardDefinition.hpp"
#include "Game/EncounterDefinition.hpp"
#include "Engine/Core/FileUtils.hpp"
#include "Engine/Renderer/DebugRenderSystem.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"


SaveManager g_saveManager;


void SaveManager::RecordGameState()
{
	//record rng variables
	m_rngSeed = g_rng.m_seed;
	m_rngPosition = g_rng.m_position;

	//record encounter variables
	Map* map = g_theGame->m_map;
	m_encounterDefID = map->m_allEncounters[map->m_currentEncounterNumber]->m_definition->m_id;
	m_encounterNumber = static_cast<uint8_t>(map->m_currentEncounterNumber);
	if (map->m_isRestTime)
	{
		m_gameState = 2;
	}
	else if (map->m_allEncounters[map->m_currentEncounterNumber]->m_cardRewardScreenOpen)
	{
		m_gameState = 1;
	}
	else
	{
		m_gameState = 0;
	}

	//record player variables
	m_playerCurrentHealth = static_cast<uint8_t>(g_theGame->m_player->m_currentHealth);
	m_playerDeckCardDefIDs.clear();
	for (int cardIndex = 0; cardIndex < g_theGame->m_player->m_deck.size(); cardIndex++)
	{
		m_playerDeckCardDefIDs.emplace_back((g_theGame->m_player->m_deck[cardIndex].m_definition->m_id));
	}

	//std::string saveMessage = "Progress saved";
	//DebugAddMessage(saveMessage, 3.0f);
}


void SaveManager::SaveProgress()
{
	DebuggerPrintf("Save\n");

	std::vector<uint8_t> saveBuffer;

	//write 4cc first ("TDTT")
	saveBuffer.emplace_back('T');
	saveBuffer.emplace_back('D');
	saveBuffer.emplace_back('T');
	saveBuffer.emplace_back('T');

	//then save rng variables
	uint8_t rngSeedByte1 = static_cast<uint8_t>(m_rngSeed >> 24);
	uint8_t rngSeedByte2 = static_cast<uint8_t>(m_rngSeed >> 16);
	uint8_t rngSeedByte3 = static_cast<uint8_t>(m_rngSeed >> 8);
	uint8_t rngSeedByte4 = static_cast<uint8_t>(m_rngSeed);

	saveBuffer.emplace_back(rngSeedByte1);
	saveBuffer.emplace_back(rngSeedByte2);
	saveBuffer.emplace_back(rngSeedByte3);
	saveBuffer.emplace_back(rngSeedByte4); 

	uint8_t rngPositionByte1 = static_cast<uint8_t>(m_rngPosition >> 24);
	uint8_t rngPositionByte2 = static_cast<uint8_t>(m_rngPosition >> 16);
	uint8_t rngPositionByte3 = static_cast<uint8_t>(m_rngPosition >> 8);
	uint8_t rngPositionByte4 = static_cast<uint8_t>(m_rngPosition);

	saveBuffer.emplace_back(rngPositionByte1);
	saveBuffer.emplace_back(rngPositionByte2);
	saveBuffer.emplace_back(rngPositionByte3);
	saveBuffer.emplace_back(rngPositionByte4);

	//then save encounter variables
	saveBuffer.emplace_back(m_encounterDefID);
	saveBuffer.emplace_back(m_encounterNumber);
	saveBuffer.emplace_back(m_gameState);

	//then save player variables
	saveBuffer.emplace_back(m_playerCurrentHealth);
	for (int cardIndex = 0; cardIndex < m_playerDeckCardDefIDs.size(); cardIndex++)
	{
		saveBuffer.emplace_back(m_playerDeckCardDefIDs[cardIndex]);
	}
	
	std::string saveFilePath = "Save.bin";

	FileWriteFromBuffer(saveBuffer, saveFilePath);
}


bool SaveManager::LoadProgress()
{
	std::string saveFilePath = "Save.bin";
	if (!CheckForFile(saveFilePath))
	{
		return false;
	}

	std::vector<uint8_t> saveBuffer;

	FileReadToBuffer(saveBuffer, saveFilePath);

	//check 4cc ("TDTT")
	if (saveBuffer.size() < 15)
	{
		ERROR_RECOVERABLE("Save file too small!");
		return false;
	}
	if (saveBuffer[0] != 'T' || saveBuffer[1] != 'D' || saveBuffer[2] != 'T' || saveBuffer[3] != 'T')
	{
		ERROR_RECOVERABLE("4cc was incorrect!");
		return false;
	}

	//load rng state
	unsigned int rngSeedByte1 = static_cast<unsigned int>(saveBuffer[4]) << 24;
	unsigned int rngSeedByte2 = static_cast<unsigned int>(saveBuffer[5]) << 16;
	unsigned int rngSeedByte3 = static_cast<unsigned int>(saveBuffer[6]) << 8;
	unsigned int rngSeedByte4 = static_cast<unsigned int>(saveBuffer[7]);
	m_rngSeed = rngSeedByte4 | rngSeedByte3 | rngSeedByte2 | rngSeedByte1;

	unsigned int rngPositionByte1 = static_cast<unsigned int>(saveBuffer[8]) << 24;
	unsigned int rngPositionByte2 = static_cast<unsigned int>(saveBuffer[9]) << 16;
	unsigned int rngPositionByte3 = static_cast<unsigned int>(saveBuffer[10]) << 8;
	unsigned int rngPositionByte4 = static_cast<unsigned int>(saveBuffer[11]);
	m_rngPosition = rngPositionByte4 | rngPositionByte3 | rngPositionByte2 | rngPositionByte1;

	//load encounter and map state
	m_encounterDefID = saveBuffer[12];
	m_encounterNumber = saveBuffer[13];
	m_gameState = saveBuffer[14];

	//load player state
	m_playerCurrentHealth = saveBuffer[15];
	m_playerDeckCardDefIDs.clear();
	for (int bufferIndex = 16; bufferIndex < saveBuffer.size(); bufferIndex++)
	{
		m_playerDeckCardDefIDs.emplace_back(saveBuffer[bufferIndex]);
	}

	//create player and map
	g_theGame->m_player = new Player();
	Player* player = g_theGame->m_player;
	player->m_currentHealth = m_playerCurrentHealth;
	player->m_deck.clear();
	for (int cardIndex = 0; cardIndex < m_playerDeckCardDefIDs.size(); cardIndex++)
	{
		player->m_deck.emplace_back(Card(&CardDefinition::s_cardDefs[m_playerDeckCardDefIDs[cardIndex]], player));
	}

	g_rng.SeedRNG(m_rngSeed);
	g_rng.m_position = 0;
	g_theGame->m_map = new Map(g_theGame->m_player);
	Map* map = g_theGame->m_map;
	//actually use loaded encounter and map state variables
	map->m_currentEncounterNumber = m_encounterNumber;
	for (int encounterIndex = 0; encounterIndex < m_encounterNumber; encounterIndex++)
	{
		delete map->m_allEncounters[encounterIndex];
		map->m_allEncounters[encounterIndex] = nullptr;
	}
	
	if (m_gameState == 2)
	{
		map->m_isRestTime = true;
	}
	else if (m_gameState == 1)
	{
		map->m_allEncounters[m_encounterNumber]->OpenCardRewardScreen();
	}

	g_rng.m_position = m_rngPosition;

	g_theGame->m_map->m_allEncounters[m_encounterNumber]->BeginEncounter();

	//set appropriate music
	if (map->m_isRestTime)
	{
		if (map->m_currentEncounterNumber == ENCOUNTER_DIFFICULTY_2_MAX_INDEX)
		{
			g_finalRestStopMusicPlayback = g_theAudio->StartSound(g_finalRestStopMusic, true);
		}
		else
		{
			g_restStopMusicPlayback = g_theAudio->StartSound(g_restStopMusic, true);
		}
		g_campfireSoundPlayback = g_theAudio->StartSound(g_campfireSound, true);
	}
	else
	{
		if (map->m_currentEncounterNumber >= NUM_ENCOUNTERS_DIFFICULTY_0 && map->m_currentEncounterNumber < ENCOUNTER_DIFFICULTY_1_MAX_INDEX)
		{
			g_battle2MusicPlayback = g_theAudio->StartSound(g_battle2Music, true, 0.8f);
		}
		else if (map->m_currentEncounterNumber >= ENCOUNTER_DIFFICULTY_1_MAX_INDEX && map->m_currentEncounterNumber < ENCOUNTER_DIFFICULTY_2_MAX_INDEX)
		{
			g_battle3MusicPlayback = g_theAudio->StartSound(g_battle3Music, true, 0.8f);
		}
		else if (map->m_currentEncounterNumber == ENCOUNTER_DIFFICULTY_2_MAX_INDEX)
		{
			g_bossMusicPlayback = g_theAudio->StartSound(g_bossMusic, true);
		}
		else if (map->m_currentEncounterNumber == ENCOUNTER_DIFFICULTY_2_MAX_INDEX + 1)
		{
			g_finalBossMusicPlayback = g_theAudio->StartSound(g_finalBossMusic, true);
		}
		else
		{
			g_battleMusicPlayback = g_theAudio->StartSound(g_battleMusic, true);
		}
	}

	return true;
}
