#include "Game/Encounter.hpp"
#include "Game/EncounterDefinition.hpp"
#include "Game/Enemy.hpp"
#include "Game/Player.hpp"
#include "Game/App.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Game.hpp"
#include "Game/Map.hpp"
#include "Game/CardDefinition.hpp"
#include "Game/Card.hpp"
#include "Game/SaveManager.hpp"
#include "Engine/Renderer/DebugRenderSystem.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Audio/AudioSystem.hpp"


//
//constructor and destructor
//
Encounter::Encounter(EncounterDefinition const* definition, int encounterNumber, Player* player, Map* map)
	: m_definition(definition)
	, m_encounterNumber(encounterNumber)
	, m_player(player)
	, m_map(map)
{
	for (int defIndex = 0; defIndex < m_definition->m_enemies.size(); defIndex++)
	{
		m_currentEnemies.emplace_back(new Enemy(m_definition->m_enemies[defIndex], this, m_definition->m_enemyBounds[defIndex]));
	}

	//generate random rewards, start at 2 to not generate starter cards, cut out status cards at end
	// #ToDo: weight based on rarity
	int randomCardIndex0 = g_rng.RollRandomIntInRange(NUM_STARTER_CARDS, static_cast<int>(CardDefinition::s_cardDefs.size()) - 1 - NUM_STATUS_CARDS);
	m_cardRewards[0] = Card(&CardDefinition::s_cardDefs[randomCardIndex0], m_player);

	int randomCardIndex1;
	do
	{
		randomCardIndex1 = g_rng.RollRandomIntInRange(NUM_STARTER_CARDS, static_cast<int>(CardDefinition::s_cardDefs.size()) - 1 - NUM_STATUS_CARDS);
	} while (randomCardIndex1 == randomCardIndex0);
	m_cardRewards[1] = Card(&CardDefinition::s_cardDefs[randomCardIndex1], m_player);

	int randomCardIndex2;
	do
	{
		randomCardIndex2 = g_rng.RollRandomIntInRange(NUM_STARTER_CARDS, static_cast<int>(CardDefinition::s_cardDefs.size()) - 1 - NUM_STATUS_CARDS);
	} while (randomCardIndex2 == randomCardIndex0 || randomCardIndex2 == randomCardIndex1);
	m_cardRewards[2] = Card(&CardDefinition::s_cardDefs[randomCardIndex2], m_player);
}


Encounter::~Encounter()
{
	for (int defIndex = 0; defIndex < m_currentEnemies.size(); defIndex++)
	{
		if (m_currentEnemies[defIndex] != nullptr)
		{
			delete m_currentEnemies[defIndex];
		}
	}
}


//
//public game flow functions
//
void Encounter::Update()
{
	if (m_cardRewardScreenOpen)
	{
		UpdateCardRewardScreen();

		return;
	}

	for (int enemyIndex = 0; enemyIndex < m_currentEnemies.size(); enemyIndex++)
	{
		if (m_currentEnemies[enemyIndex]->m_currentHealth > 0)
		{
			m_currentEnemies[enemyIndex]->Update();
		}
	}
	
	std::string encounterNumberText = Stringf("Encounter %i", m_encounterNumber + 1);
	DebugAddScreenText(encounterNumberText, Vec2(SCREEN_CAMERA_CENTER_X, SCREEN_CAMERA_SIZE_Y - 5.0f), 20.0f, Vec2(0.5f, 1.0f), 0.0f);
	
	if (m_turnState == TurnState::PLAYER)
	{
		std::string turnNumberText = Stringf("Turn %i: Player's Turn", m_turnNumber);
		DebugAddScreenText(turnNumberText, Vec2(SCREEN_CAMERA_CENTER_X, SCREEN_CAMERA_SIZE_Y - 30.0f), 30.0f, Vec2(0.5f, 1.0f), 0.0f);
	}
	else if (m_turnState == TurnState::ENEMY)
	{
		std::string turnNumberText = Stringf("Turn %i: Enemy's Turn", m_turnNumber);
		DebugAddScreenText(turnNumberText, Vec2(SCREEN_CAMERA_CENTER_X, SCREEN_CAMERA_SIZE_Y - 30.0f), 30.0f, Vec2(0.5f, 1.0f), 0.0f, Rgba8(255, 100, 100), Rgba8(255, 100, 100));

		m_enemyTurnTimer -= g_theGame->m_gameClock.GetDeltaSeconds();
		if (m_enemyTurnTimer <= 0.0f)
		{
			m_enemyTurnTimer = ENEMY_TURN_DURATION;
			
			if (m_nextEnemyToAct >= m_currentEnemies.size())
			{
				ChangeTurnState(TurnState::PLAYER);
				m_nextEnemyToAct = 0;
			}
			else
			{
				while (m_currentEnemies[m_nextEnemyToAct]->m_currentHealth == 0)
				{
					m_nextEnemyToAct++;	//skip over dead enemies
					if (m_nextEnemyToAct >= m_currentEnemies.size()) break;
				}
				if (m_nextEnemyToAct < m_currentEnemies.size())
				{
					m_currentEnemies[m_nextEnemyToAct]->PerformCurrentIntention();
				}
				m_nextEnemyToAct++;
			}
		}
	}
}


void Encounter::Render() const
{
	if (m_cardRewardScreenOpen)
	{
		RenderCardRewardScreen();

		return;
	}

	//render background
	std::vector<Vertex_PCU> backgroundVerts;
	AABB2 screenBounds = AABB2(0.0f, 0.0f, SCREEN_CAMERA_SIZE_X, SCREEN_CAMERA_SIZE_Y);

	Rgba8 backgroundColor1 = Rgba8(30, 65, 100);
	Rgba8 backgroundColor2 = Rgba8(60, 120, 100);
	if (m_definition->m_difficultyLevel == 1)
	{
		backgroundColor1 = Rgba8(100, 15, 90);
		backgroundColor2 = Rgba8(30, 120, 5);
	}
	else if(m_definition->m_difficultyLevel == 2)
	{
		backgroundColor1 = Rgba8(190, 75, 0);
		backgroundColor2 = Rgba8(100, 50, 0);
	}
	else if (m_definition->m_difficultyLevel == 3)
	{
		backgroundColor1 = Rgba8(95, 165, 5);
		backgroundColor2 = Rgba8(25, 75, 35);
	}
	else if (m_definition->m_difficultyLevel == 4)
	{
		backgroundColor1 = Rgba8(165, 0, 190);
		backgroundColor2 = Rgba8(100, 0, 75);
	}

	backgroundVerts.push_back(Vertex_PCU(screenBounds.m_mins, backgroundColor1));
	backgroundVerts.push_back(Vertex_PCU(Vec2(screenBounds.m_maxs.x, screenBounds.m_mins.y), backgroundColor1));
	backgroundVerts.push_back(Vertex_PCU(screenBounds.m_maxs, backgroundColor2));

	backgroundVerts.push_back(Vertex_PCU(screenBounds.m_mins, backgroundColor1));
	backgroundVerts.push_back(Vertex_PCU(screenBounds.m_maxs, backgroundColor2));
	backgroundVerts.push_back(Vertex_PCU(Vec2(screenBounds.m_mins.x, screenBounds.m_maxs.y), backgroundColor2));

	g_theRenderer->BindTexture(nullptr);
	g_theRenderer->DrawVertexArray(backgroundVerts);
	
	//render player
	m_player->Render();

	//render all enemies
	for (int enemyIndex = 0; enemyIndex < m_currentEnemies.size(); enemyIndex++)
	{
		if (m_currentEnemies[enemyIndex]->m_currentHealth > 0)
		{
			m_currentEnemies[enemyIndex]->Render();
		}
	}

	////render turn indicator
	//std::vector<Vertex_PCU> indicatorVerts;
	//if (m_turnState == TurnState::PLAYER)
	//{
	//	AddVertsForArrow2D(indicatorVerts, Vec2(375.0f, 685.0f), Vec2(375.0f, 650.0f), 10.0f, 8.0f, Rgba8(0, 255, 0));
	//}
	//else
	//{
	//	/*if (m_nextEnemyToAct < m_currentEnemies.size())
	//	{
	//		Enemy* currentEnemy = m_currentEnemies[m_nextEnemyToAct];
	//		if (currentEnemy != nullptr)
	//		{
	//			float boundsMidX = (currentEnemy->m_renderBounds.m_mins.x + currentEnemy->m_renderBounds.m_maxs.x) * 0.5f;
	//			AddVertsForArrow2D(indicatorVerts, Vec2(boundsMidX, 685.0f), Vec2(boundsMidX, 650.0f), 10.0f, 8.0f, Rgba8(0, 255, 0));
	//		}
	//	}*/
	//	AddVertsForArrow2D(indicatorVerts, Vec2(1200.0f, 685.0f), Vec2(1200.0f, 650.0f), 10.0f, 8.0f, Rgba8(0, 255, 0));
	//}
	//g_theRenderer->BindTexture(nullptr);
	//g_theRenderer->DrawVertexArray(indicatorVerts);
}


void Encounter::UpdateCardRewardScreen()
{
	bool card0Chosen = false;
	bool card1Chosen = false;
	bool card2Chosen = false;
	m_cardRewards[0].Update(0, card0Chosen, true);
	m_cardRewards[1].Update(1, card1Chosen, true);
	m_cardRewards[2].Update(2, card2Chosen, true);

	if (card0Chosen)
	{
		AcceptCardReward(0);
	}
	else if (card1Chosen)
	{
		AcceptCardReward(1);
	}
	else if (card2Chosen)
	{
		AcceptCardReward(2);
	}
}


void Encounter::RenderCardRewardScreen() const
{
	//draw text
	DebugAddScreenText("Reward! Pick One:", Vec2(SCREEN_CAMERA_CENTER_X, SCREEN_CAMERA_SIZE_Y - 25.0f), 50.0f, Vec2(0.5f, 1.0f), 0.0f);

	//draw background panel
	std::vector<Vertex_PCU> panelVerts;
	AABB2 panel = AABB2(150.0f, 75.0f, SCREEN_CAMERA_SIZE_X - 150.0f, SCREEN_CAMERA_SIZE_Y - 75.0f);
	AddVertsForAABB2(panelVerts, panel, Rgba8(50, 50, 50));
	g_theRenderer->BindTexture(nullptr);
	g_theRenderer->DrawVertexArray(panelVerts);

	//draw each card
	m_cardRewards[0].Render(0, true);
	m_cardRewards[1].Render(1, true);
	m_cardRewards[2].Render(2, true);
}


//
//public turn utilities
//
void Encounter::BeginEncounter()
{
	g_saveManager.RecordGameState();
	
	m_player->ShuffleDrawPileFromDeck();

	//choose starting intentions for enemies
	for (int defIndex = 0; defIndex < m_currentEnemies.size(); defIndex++)
	{
		if (m_currentEnemies[defIndex] != nullptr)
		{
			m_currentEnemies[defIndex]->ChooseNextIntention();
		}
	}

	BeginPlayerTurn();
}


void Encounter::ChangeTurnState(TurnState turnState)
{
	switch (m_turnState)
	{
	case TurnState::PLAYER: EndPlayerTurn(); break;
	case TurnState::ENEMY:	EndEnemyTurn();	 break;
	}
	
	m_turnState = turnState;

	switch (m_turnState)
	{
	case TurnState::PLAYER: BeginPlayerTurn(); break;
	case TurnState::ENEMY:	BeginEnemyTurn(); break;
	}
}


void Encounter::BeginPlayerTurn()
{
	//increment turn counter at beginning of player's turn
	m_turnNumber++;

	//get rid of any previously selected cards, just in case
	m_player->m_selectedCard = nullptr;

	//player draws five cards
	m_player->DrawCard();
	m_player->DrawCard();
	m_player->DrawCard();
	m_player->DrawCard();
	m_player->DrawCard();

	//restore energy to max
	m_player->ReturnToStartEnergy();

	//lose block from previous turn
	m_player->m_currentBlock = 0;
}


void Encounter::EndPlayerTurn()
{
	m_player->DiscardHand();
}


void Encounter::BeginEnemyTurn()
{
	//lose block from previous turn
	for (int defIndex = 0; defIndex < m_currentEnemies.size(); defIndex++)
	{
		if (m_currentEnemies[defIndex] != nullptr)
		{
			m_currentEnemies[defIndex]->m_currentBlock = 0;
		}
	}
}


void Encounter::EndEnemyTurn()
{
	//choose next intentions for enemies
	for (int enemyIndex = 0; enemyIndex < m_currentEnemies.size(); enemyIndex++)
	{
		Enemy* enemy = m_currentEnemies[enemyIndex];
		
		if (enemy != nullptr)
		{
			enemy->ChooseNextIntention();
		}
	}

	//reduce enemy effects of duration stack type by one
	for (int enemyIndex = 0; enemyIndex < m_currentEnemies.size(); enemyIndex++)
	{
		Enemy* enemy = m_currentEnemies[enemyIndex];

		for (int effectIndex = 0; effectIndex < enemy->m_effects.size(); effectIndex++)
		{
			Effect& effect = enemy->m_effects[effectIndex];

			if (effect.m_justAdded)
			{
				effect.m_justAdded = false;
			}

			if (effect.m_definition->m_stackType == StackType::DURATION)
			{
				effect.m_stack -= 1;
				
				if (effect.m_stack <= 0)
				{
					//use erase-remove idiom to remove effect
					enemy->m_effects.erase(std::remove_if(
						enemy->m_effects.begin(),
						enemy->m_effects.end(),
						[=](auto const& element)
						{
							return element == effect;
						}),
						enemy->m_effects.end()
					);
				}
			}
		}
	}

	//reduce player effects of duration stack type by one (unless they were just inflicted that turn)
	for (int effectIndex = 0; effectIndex < m_player->m_effects.size(); effectIndex++)
	{
		Effect& effect = m_player->m_effects[effectIndex];

		if (effect.m_justAdded)
		{
			effect.m_justAdded = false;
		}
		else
		{
			if (effect.m_definition->m_stackType == StackType::DURATION)
			{
				effect.m_stack -= 1;

				if (effect.m_stack <= 0)
				{
					//use erase-remove idiom to remove effect
					m_player->m_effects.erase(std::remove_if(
						m_player->m_effects.begin(),
						m_player->m_effects.end(),
						[=](auto const& element)
						{
							return effect == element;
						}),
						m_player->m_effects.end()
					);
				}
			}
		}
	}
}


void Encounter::EndEncounter()
{
	m_player->ResetCards();
	m_player->m_effects.clear();
}


//
//public enemy utilities
//
bool Encounter::AreAllEnemiesDead() const
{
	bool allEnemiesDead = true;
	for (int enemyIndex = 0; enemyIndex < m_currentEnemies.size(); enemyIndex++)
	{
		Enemy* enemy = m_currentEnemies[enemyIndex];
		if (enemy != nullptr)
		{
			if (enemy->m_currentHealth > 0)
			{
				allEnemiesDead = false;
			}
		}
	}

	return allEnemiesDead;
}


void Encounter::KillAllEnemies()
{
	for (int enemyIndex = 0; enemyIndex < m_currentEnemies.size(); enemyIndex++)
	{
		Enemy* enemy = m_currentEnemies[enemyIndex];
		if (enemy != nullptr)
		{
			enemy->m_currentHealth = 0;
		}
	}
}


void Encounter::OpenCardRewardScreen()
{
	m_cardRewardScreenOpen = true;

	g_saveManager.RecordGameState();
}


void Encounter::AcceptCardReward(int cardRewardNum)
{
	//can't have more than 3 card rewards
	if (cardRewardNum >= 3)
	{
		return;
	}
	
	m_player->m_deck.emplace_back(m_cardRewards[cardRewardNum]);
	m_map->EnterNextEncounter();
}
