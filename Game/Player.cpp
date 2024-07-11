#include "Game/Player.hpp"
#include "Game/Enemy.hpp"
#include "Game/Encounter.hpp"
#include "Game/GameCommon.hpp"
#include "Game/App.hpp"
#include "Game/Game.hpp"
#include "Game/Map.hpp"
#include "Game/CardDefinition.hpp"
#include "Game/EffectDefinition.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Renderer/DebugRenderSystem.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Audio/AudioSystem.hpp"


//
//constructor
//
Player::Player()
{
	InitializeDeck();
}


//
//public game flow functions
//
void Player::Update()
{
	//lerp back from flashing red when hit
	if (m_renderColor.r < 255)
	{
		m_renderColor.r = static_cast<unsigned char>(GetClamped(static_cast<float>(m_renderColor.r) + 360.0f * g_theGame->m_gameClock.GetDeltaSeconds(), 0.0f, 255.0f));
	}
	if (m_renderColor.g < 255)
	{
		m_renderColor.g = static_cast<unsigned char>(GetClamped(static_cast<float>(m_renderColor.g) + 360.0f * g_theGame->m_gameClock.GetDeltaSeconds(), 0.0f, 255.0f));
	}
	if (m_renderColor.b < 255)
	{
		m_renderColor.b = static_cast<unsigned char>(GetClamped(static_cast<float>(m_renderColor.b) + 360.0f * g_theGame->m_gameClock.GetDeltaSeconds(), 0.0f, 255.0f));
	}
	
	//check for mouse inputs for playing cards
	for (int cardIndex = 0; cardIndex < m_hand.size(); cardIndex++)
	{
		bool wasCardPlayed = false;
		m_hand[cardIndex]->Update(cardIndex, wasCardPlayed);

		if (wasCardPlayed)
		{
			//no need to check other cards if one was played
			break;
		}
	}
}


void Player::Render() const
{
	std::vector<Vertex_PCU> playerVerts;

	AddVertsForAABB2(playerVerts, m_playerBounds, m_renderColor);

	g_theRenderer->BindShader(nullptr);
	g_theRenderer->BindTexture(g_playerSprite);
	g_theRenderer->DrawVertexArray(playerVerts);

	std::string healthText = Stringf("HP: %i/%i", m_currentHealth, PLAYER_MAX_HEALTH);
	DebugAddScreenText(healthText, Vec2(375.0f, 350.0f), 25.0f, Vec2(0.5f, 1.0f), 0.0f, Rgba8(255, 0, 0), Rgba8(255, 0, 0));
	std::string blockText = Stringf("Block: %i", m_currentBlock);
	DebugAddScreenText(blockText, Vec2(375.0f, 325.0f), 25.0f, Vec2(0.5f, 1.0f), 0.0f, Rgba8(0, 100, 255), Rgba8(0, 100, 255));
	std::string energyText = Stringf("Energy:\n%i/%i", m_currentEnergy, PLAYER_START_ENERGY);
	DebugAddScreenText(energyText, Vec2(175.0f, 200.0f), 25.0f, Vec2(0.5f, 1.0f), 0.0f, Rgba8(255, 150, 0), Rgba8(255, 150, 0));

	//render effect icons
	for (int effectIndex = 0; effectIndex < m_effects.size(); effectIndex++)
	{
		Effect const& effect = m_effects[effectIndex];
		
		float minX = 250.0f + static_cast<float>(40 * effectIndex);
		float minY = m_playerBounds.m_mins.y - 90.0f;
		AABB2 iconBounds = AABB2(minX, minY, minX + 40.0f, minY + 40.0f);

		std::vector<Vertex_PCU> iconVerts;
		AddVertsForAABB2(iconVerts, iconBounds);

		g_theRenderer->BindTexture(effect.m_definition->m_sprite);
		g_theRenderer->DrawVertexArray(iconVerts);

		std::string effectStackText = Stringf("%i", effect.m_stack);
		DebugAddScreenText(effectStackText, Vec2(minX + 10.0f, minY), 20.0f, Vec2(0.0f, 1.0f), 0.0f);
	}

	//debug print effects
	/*for (int effectIndex = 0; effectIndex < m_effects.size(); effectIndex++)
	{
		Effect const& effect = m_effects[effectIndex];

		std::string effectText = Stringf("%s %i", effect.m_definition->m_name.c_str(), effect.m_stack);
		DebugAddMessage(effectText, 0.0f, Rgba8(255, 255, 0), Rgba8(255, 255, 0));
	}*/

	//render cards
	for (int handIndex = 0; handIndex < m_hand.size(); handIndex++)
	{
		m_hand[handIndex]->Render(handIndex);
	}

	//render draw and discard pile indicators
	std::vector<Vertex_PCU> drawPileVerts;
	AABB2 drawPileBox = AABB2(25.0f, 15.0f, 105.0f, 95.0f);
	AddVertsForAABB2(drawPileVerts, drawPileBox, Rgba8(0, 0, 0));
	g_theRenderer->BindTexture(nullptr);
	g_theRenderer->DrawVertexArray(drawPileVerts);
	std::string drawPileCount = Stringf("%i", m_drawPile.size());
	DebugAddScreenText(drawPileCount, Vec2(65.0f, 55.0f), 35.0f, Vec2(0.5f, 0.5f), 0.0f);

	std::vector<Vertex_PCU> discardPileVerts;
	AABB2 discardPileBox = AABB2(SCREEN_CAMERA_SIZE_X - 105.0f, 15.0f, SCREEN_CAMERA_SIZE_X - 25.0f, 95.0f);
	AddVertsForAABB2(discardPileVerts, discardPileBox, Rgba8(0, 0, 0));
	g_theRenderer->BindTexture(nullptr);
	g_theRenderer->DrawVertexArray(discardPileVerts);
	std::string discardPileCount = Stringf("%i", m_discardPile.size());
	DebugAddScreenText(discardPileCount, Vec2(SCREEN_CAMERA_SIZE_X - 65.0f, 55.0f), 35.0f, Vec2(0.5f, 0.5f), 0.0f);
}


//
//public player utilities
//
bool Player::PlayCard(Card* cardToPlay, Enemy* enemyTarget)
{
	//can't play card if you can't afford it
	if (m_currentEnergy < cardToPlay->m_definition->m_cost)
	{
		return false;
	}

	//subtract cost from player's energy
	m_currentEnergy -= cardToPlay->m_definition->m_cost;

	//add card to discard pile if card doesn't exhaust
	if (!cardToPlay->m_definition->m_exhaust)
	{
		m_discardPile.emplace_back(cardToPlay);
	}

	//use erase-remove idiom to remove that card from the hand
	m_hand.erase(std::remove_if(
		m_hand.begin(),
		m_hand.end(),
		[=](auto const& element)
		{
			return element == cardToPlay;
		}),
		m_hand.end()
	);

	//then actually cause card effects
	cardToPlay->Play(enemyTarget, g_theGame->m_map->m_allEncounters[g_theGame->m_map->m_currentEncounterNumber]);

	return true;
}


void Player::GainBlock(int blockAmount)
{
	m_currentBlock += blockAmount;

	if (blockAmount > 0)
	{
		std::string blockText = Stringf("+%i", blockAmount);
		DebugAddScreenText(blockText, Vec2(250.0f, 325.0f), 30.0f, Vec2(1.0f, 1.0f), 2.0f, Rgba8(0, 100, 255), Rgba8(0, 100, 255));
		g_theAudio->StartSound(g_blockSound);
	}
}


void Player::RestoreHealth(int healthAmount)
{
	m_currentHealth = GetClamped(m_currentHealth + healthAmount, 0, m_maxHealth);

	if (healthAmount > 0)
	{
		std::string damageText = Stringf("+%i", healthAmount);
		DebugAddScreenText(damageText, Vec2(250.0f, 350.0f), 30.0f, Vec2(1.0f, 1.0f), 2.0f, Rgba8(0, 255, 0), Rgba8(0, 255, 0));
		g_theAudio->StartSound(g_healSound);

		m_renderColor.r = 0;
		m_renderColor.b = 0;
	}
}


void Player::GainEnergy(int energyAmount)
{
	m_currentEnergy += energyAmount;
}


void Player::ReturnToStartEnergy()
{
	m_currentEnergy = m_startEnergy;
}


void Player::TakeDamage(int damageAmount)
{
	//do damage to block first
	int startingBlock = m_currentBlock;
	m_currentBlock = GetClamped(m_currentBlock - damageAmount, 0, m_currentBlock);

	int damageReduction = startingBlock - m_currentBlock;
	int finalDamageAmount = GetClamped(damageAmount - damageReduction, 0, damageAmount);

	//then do damage to health
	m_currentHealth = GetClamped(m_currentHealth - finalDamageAmount, 0, m_maxHealth);

	if (damageReduction > 0)
	{
		std::string blockDamageText = Stringf("-%i", damageReduction);
		DebugAddScreenText(blockDamageText, Vec2(250.0f, 325.0f), 30.0f, Vec2(1.0f, 1.0f), 2.0f, Rgba8(0, 100, 255), Rgba8(0, 100, 255));
		g_theAudio->StartSound(g_damageBlockedSound);
	}
	if (finalDamageAmount > 0)
	{
		std::string damageText = Stringf("-%i", finalDamageAmount);
		DebugAddScreenText(damageText, Vec2(250.0f, 350.0f), 30.0f, Vec2(1.0f, 1.0f), 2.0f, Rgba8(255, 0, 0), Rgba8(255, 0, 0));
		g_theAudio->StartSound(g_damageSound);

		m_renderColor.g = 0;
		m_renderColor.b = 0;
	}

	g_theGame->BeginScreenShake(static_cast<float>(damageAmount));
}


void Player::ReceiveEffect(EffectDefinition const* definition, int stack)
{
	//block debuffs with artifact
	if (definition->m_type == EffectType::DEBUFF)
	{
		for (int effectIndex = 0; effectIndex < m_effects.size(); effectIndex++)
		{
			Effect& effect = m_effects[effectIndex];

			if (effect.m_definition->m_blockDebuff)
			{
				effect.m_stack -= 1;
				std::string artifactMessage = "Debuff Blocked";
				DebugAddScreenText(artifactMessage, Vec2(375.0f, 700.0f), 27.5f, Vec2(0.5f, 1.0f), 2.0f, Rgba8(255, 100, 0), Rgba8(255, 100, 0));

				if (effect.m_stack <= 0)
				{
					//use erase-remove idiom to remove effect
					m_effects.erase(std::remove_if(
						m_effects.begin(),
						m_effects.end(),
						[=](auto const& element)
						{
							return effect == element;
						}),
						m_effects.end()
					);
				}

				return;
			}
		}

		g_theAudio->StartSound(g_debuffSound);
	}
	else if (definition->m_type == EffectType::BUFF)
	{
		g_theAudio->StartSound(g_buffSound);
	}
	
	bool newEffect = true;
	for (int effectIndex = 0; effectIndex < m_effects.size(); effectIndex++)
	{
		Effect& effect = m_effects[effectIndex];

		//if the enemy already has the effect, just increase its stack
		if (effect.m_definition->m_name == definition->m_name)
		{
			newEffect = false;
			effect.m_stack += stack;
			break;
		}
	}

	//if the enemy doesn't have this effect, add it to the list of effects
	if (newEffect)
	{
		m_effects.emplace_back(Effect(definition, stack, this, nullptr));
	}
}


//
//public deck management functions
//
void Player::InitializeDeck()
{
	//give the player their starter cards
	CardDefinition const* slash = CardDefinition::GetCardDefinition("Slash");
	CardDefinition const* guard = CardDefinition::GetCardDefinition("Guard");
	CardDefinition const* shieldBash = CardDefinition::GetCardDefinition("Shield Bash");
	//CardDefinition const* piercingWave = CardDefinition::GetCardDefinition("Piercing Wave");
	CardDefinition const* bash = CardDefinition::GetCardDefinition("Bash N' Smash");
	//CardDefinition const* strength = CardDefinition::GetCardDefinition("Build Muscle");
	//CardDefinition const* glare = CardDefinition::GetCardDefinition("Frightening Glare");
	//CardDefinition const* dis = CardDefinition::GetCardDefinition("Discombobulate");

	m_deck.push_back(Card(slash, this));
	m_deck.push_back(Card(slash, this));
	m_deck.push_back(Card(slash, this));
	m_deck.push_back(Card(slash, this));
	m_deck.push_back(Card(guard, this));
	m_deck.push_back(Card(guard, this));
	m_deck.push_back(Card(guard, this));
	m_deck.push_back(Card(guard, this));
	m_deck.push_back(Card(shieldBash, this));
	//m_deck.push_back(Card(piercingWave, this));
	m_deck.push_back(Card(bash, this));
	/*m_deck.push_back(Card(dis, this));
	m_deck.push_back(Card(glare, this));*/
	//m_deck.push_back(Card(strength, this));
}


void Player::ShuffleDrawPileFromDeck()
{
	std::vector<Card*> tempDeck;
	for (int cardIndex = 0; cardIndex < m_deck.size(); cardIndex++)
	{
		tempDeck.emplace_back(&m_deck[cardIndex]);
	}

	while (tempDeck.size() > 0)
	{
		//move random card from temp deck to draw pile
		int cardToMoveToDrawPile = g_rng.RollRandomIntLessThan(static_cast<int>(tempDeck.size()));
		m_drawPile.emplace_back(tempDeck[cardToMoveToDrawPile]);

		//use erase-remove idiom to remove card from temp deck
		tempDeck.erase(std::remove_if(
			tempDeck.begin(),
			tempDeck.end(),
			[=](auto const& element)
			{
				return element == tempDeck[cardToMoveToDrawPile];
			}),
			tempDeck.end()
		);
	}
}


void Player::ShuffleDrawPileFromDiscardPile()
{
	while (m_discardPile.size() > 0)
	{
		//move random card from discard pile to draw pile
		int cardToMoveToDrawPile = g_rng.RollRandomIntLessThan(static_cast<int>(m_discardPile.size()));
		m_drawPile.emplace_back(m_discardPile[cardToMoveToDrawPile]);

		//use erase-remove idiom to remove card from temp deck
		m_discardPile.erase(std::remove_if(
			m_discardPile.begin(),
			m_discardPile.end(),
			[=](auto const& element)
			{
				return element == m_discardPile[cardToMoveToDrawPile];
			}),
			m_discardPile.end()
		);
	}
}


void Player::DrawCard()
{
	//if at max hand size, don't draw more cards
	if (m_hand.size() == MAX_HAND_SIZE)
	{
		return;
	}
	
	//if draw pile is empty, refill it from the discard pile
	if (m_drawPile.size() == 0)
	{
		//go ahead and return prematurely if we don't even have any cards in the discard pile somehow
		if (m_discardPile.size() == 0)
		{
			return;
		}
		
		ShuffleDrawPileFromDiscardPile();
	}

	//draw first card from draw pile
	m_hand.emplace_back(m_drawPile[0]);
	
	//use erase-remove idiom to remove that card from the draw pile
	m_drawPile.erase(std::remove_if(
		m_drawPile.begin(),
		m_drawPile.end(),
		[=](auto const& element)
		{
			return element == m_drawPile[0];
		}),
		m_drawPile.end()
	);
}


void Player::DiscardHand()
{
	while (m_hand.size() > 0)
	{
		//add card to discard pile
		m_discardPile.emplace_back(m_hand[0]);

		//use erase-remove idiom to remove that card from the hand
		m_hand.erase(std::remove_if(
			m_hand.begin(),
			m_hand.end(),
			[=](auto const& element)
			{
				return element == m_hand[0];
			}),
			m_hand.end()
		);
	}
}


void Player::ResetCards()
{
	m_hand.clear();
	m_drawPile.clear();
	m_discardPile.clear();

	for (int cardIndex = 0; cardIndex < m_tempAddedCards.size(); cardIndex++)
	{
		delete m_tempAddedCards[cardIndex];
	}

	m_tempAddedCards.clear();
}
