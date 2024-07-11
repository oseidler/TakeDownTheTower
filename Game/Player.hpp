#pragma once
#include "Game/Card.hpp"
#include "Game/Effect.hpp"
#include "Engine/Core/EngineCommon.hpp"


//constants
constexpr int PLAYER_MAX_HEALTH = 30;
constexpr int PLAYER_START_ENERGY = 3;
constexpr int MAX_HAND_SIZE = 5;

constexpr int REST_HEAL_AMOUNT = 10;


//forward declarations
class Enemy;
class Encounter;
class EffectDefinition;


class Player
{
//public member functions
public:
	//constructor
	Player();

	//game flow functions
	void Update();
	void Render() const;

	//player utilities
	bool PlayCard(Card* cardToPlay, Enemy* enemyTarget);
	void GainBlock(int blockAmount);
	void RestoreHealth(int healthAmount);
	void GainEnergy(int energyAmount);
	void ReturnToStartEnergy();
	void TakeDamage(int damageAmount);
	void ReceiveEffect(EffectDefinition const* definition, int stack);

	//card management functions
	void InitializeDeck();
	void ShuffleDrawPileFromDeck();
	void ShuffleDrawPileFromDiscardPile();
	void DrawCard();
	void DiscardHand();
	void ResetCards();

//public member variables
public:
	//player parameters
	int m_currentHealth = PLAYER_MAX_HEALTH;
	int m_maxHealth = PLAYER_MAX_HEALTH;
	int m_currentEnergy = PLAYER_START_ENERGY;
	int m_startEnergy = PLAYER_START_ENERGY;
	int m_currentBlock = 0;

	//card variables
	std::vector<Card> m_deck;

	std::vector<Card*> m_drawPile;
	std::vector<Card*> m_hand;
	std::vector<Card*> m_discardPile;

	std::vector<Card*> m_tempAddedCards;

	Card* m_selectedCard = nullptr;

	std::vector<Effect> m_effects;

	AABB2 m_playerBounds = AABB2(200.0f, 350.0f, 550.0f, 600.0f);
	Rgba8 m_renderColor = Rgba8();
};
