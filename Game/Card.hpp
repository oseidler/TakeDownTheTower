#pragma once


//forward declarations
class CardDefinition;
class Enemy;
class Encounter;
class Player;


class Card
{
//public member functions
public:
	//constructor
	Card() {}
	explicit Card(CardDefinition const* definition, Player* player);

	//game flow functions
	void Update(int cardPosition, bool& wasCardPlayed, bool isReward = false);
	void Render(int cardPosition, bool isReward = false) const;

	//card actions
	void Play(Enemy* enemyTarget, Encounter* currentEncounter) const;

//public member variables
public:
	CardDefinition const* m_definition = nullptr;
	Player* m_player = nullptr;
};
