#pragma once
#include "Game/Encounter.hpp"
#include "Engine/Core/EngineCommon.hpp"


//forward declaration
class Player;


//generation constants
constexpr int NUM_ENCOUNTERS_DIFFICULTY_0 = 3;
constexpr int NUM_ENCOUNTERS_DIFFICULTY_1 = 3;
constexpr int NUM_ENCOUNTERS_DIFFICULTY_2 = 3;

constexpr int ENCOUNTER_DIFFICULTY_1_MAX_INDEX = NUM_ENCOUNTERS_DIFFICULTY_0 + NUM_ENCOUNTERS_DIFFICULTY_1;
constexpr int ENCOUNTER_DIFFICULTY_2_MAX_INDEX = ENCOUNTER_DIFFICULTY_1_MAX_INDEX + NUM_ENCOUNTERS_DIFFICULTY_2;


class Map
{
//public member functions
public:
	//constructor and destructor
	Map(Player* player);
	~Map();

	//game flow functions
	void UpdateRestStop();
	void RenderRestStop() const;

	//map utilities
	void EnterFirstEncounter() const;
	void EnterNextEncounter();
	void EnterRestStop();

//public member variables
public:
	std::vector<Encounter*> m_allEncounters;
	int m_currentEncounterNumber = 0;

	bool m_isRestTime = false;
};
