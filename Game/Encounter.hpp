#pragma once
#include "Game/Card.hpp"
#include "Engine/Core/EngineCommon.hpp"


//forward declarations
class EncounterDefinition;
class Enemy;
class Player;
class Map;


//constants
constexpr float ENEMY_TURN_DURATION = 2.0f;


//turn state machine
enum class TurnState
{
	PLAYER,
	ENEMY,
};


class Encounter
{
//public member functions
public:
	//constructor and destructor
	explicit Encounter(EncounterDefinition const* definition, int encounterNumber, Player* player, Map* map);
	~Encounter();

	//game flow functions
	void Update();
	void Render() const;
	void UpdateCardRewardScreen();
	void RenderCardRewardScreen() const;

	//turn utilities
	void BeginEncounter();
	void ChangeTurnState(TurnState turnState);
	void BeginPlayerTurn();
	void EndPlayerTurn();
	void BeginEnemyTurn();
	void EndEnemyTurn();
	void EndEncounter();

	//enemy utilities
	bool AreAllEnemiesDead() const;
	void KillAllEnemies();

	//card reward screen functions
	void OpenCardRewardScreen();
	void AcceptCardReward(int cardRewardNum);

//public member variables
public:
	int		  m_encounterNumber = 0;
	int		  m_turnNumber = 0;
	int		  m_nextEnemyToAct = 0;
	TurnState m_turnState = TurnState::PLAYER;
	std::vector<Enemy*> m_currentEnemies;
	float m_enemyTurnTimer = ENEMY_TURN_DURATION;

	bool m_cardRewardScreenOpen = false;
	Card m_cardRewards[3];
	
	EncounterDefinition const* m_definition = nullptr;
	Map* m_map = nullptr;
	Player* m_player = nullptr;
};
