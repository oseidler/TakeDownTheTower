#pragma once
#include "Engine/Core/EngineCommon.hpp"


//forward declarations
class EnemyDefinition;


class EncounterDefinition
{
//public member functions
public:
	//constructor
	explicit EncounterDefinition(XmlElement const& element);

	//static functions
	static void InitializeEncounterDefs();
	static EncounterDefinition const* GetEncounterDefinition(int encounterID);

//public member variables
public:
	//encounter parameters
	uint8_t m_id = 0;
	int m_difficultyLevel = 0;
	std::vector<EnemyDefinition const*> m_enemies;
	std::vector<AABB2> m_enemyBounds;

	//static variables
	static std::vector<EncounterDefinition> s_encounterDefs;
};
