#pragma once
#include "Engine/Core/EngineCommon.hpp"


class SaveManager
{
//public member functions
public:
	void RecordGameState();
	void SaveProgress();
	bool LoadProgress();

//public member variables
public:
	//rng variables
	unsigned int m_rngSeed = 0;
	int m_rngPosition = 0;

	//encounter variables
	uint8_t m_encounterDefID = 0;
	uint8_t m_encounterNumber = 0;
	uint8_t m_gameState = 0;	   //0 = normal, 1 = in card reward screen, 2 = in rest stop

	//player variables
	uint8_t m_playerCurrentHealth = 0;
	std::vector<uint8_t> m_playerDeckCardDefIDs;
};

extern SaveManager g_saveManager;
