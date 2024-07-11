#include "Game/Enemy.hpp"
#include "Game/Game.hpp"
#include "Game/App.hpp"
#include "Game/Player.hpp"
#include "Game/Encounter.hpp"
#include "Game/GameCommon.hpp"
#include "Game/CardDefinition.hpp"
#include "Game/EffectDefinition.hpp"
#include "Game/Card.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Renderer/DebugRenderSystem.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Input/InputSystem.hpp"


//
//constructor
//
Enemy::Enemy(EnemyDefinition const* definition, Encounter* encounter, AABB2 renderBounds)
	: m_definition(definition)
	, m_encounter(encounter)
	, m_renderBounds(renderBounds)
{
	m_currentHealth = m_definition->m_maxHealth;
}


//
//public game flow functions
//
void Enemy::Update()
{
	//lerp back from flashing red when hit
	if (m_renderColor.g < 255)
	{
		m_renderColor.g = static_cast<unsigned char>(GetClamped(static_cast<float>(m_renderColor.g) + 360.0f * g_theGame->m_gameClock.GetDeltaSeconds(), 0.0f, 255.0f));
	}
	if (m_renderColor.b < 255)
	{
		m_renderColor.b = static_cast<unsigned char>(GetClamped(static_cast<float>(m_renderColor.b) + 360.0f * g_theGame->m_gameClock.GetDeltaSeconds(), 0.0f, 255.0f));
	}
}


void Enemy::Render() const
{
	//draw enemy sprite
	std::vector<Vertex_PCU> enemyVerts;
	AddVertsForAABB2(enemyVerts, m_renderBounds, m_renderColor);

	g_theRenderer->BindTexture(m_definition->m_sprite);
	g_theRenderer->DrawVertexArray(enemyVerts);

	//draw overlay if selected as card target
	Vec2 mousePosition = g_theInput->GetCursorNormalizedPosition();
	Vec2 gameMousePosition = Vec2(mousePosition.x * SCREEN_CAMERA_SIZE_X, mousePosition.y * SCREEN_CAMERA_SIZE_Y);

	Card* selectedCard = g_theGame->m_player->m_selectedCard;
	if (selectedCard != nullptr && selectedCard->m_definition->m_targetMode == TargetMode::ONE && IsPointInsideAABB2D(gameMousePosition, m_renderBounds))
	{
		std::vector<Vertex_PCU> overlayVerts;
		AddVertsForAABB2(overlayVerts, m_renderBounds, Rgba8(255, 255, 255, 75));
		g_theRenderer->BindTexture(nullptr);
		g_theRenderer->DrawVertexArray(overlayVerts);
	}

	float boundsMidX = (m_renderBounds.m_mins.x + m_renderBounds.m_maxs.x) * 0.5f;

	//display health and block
	std::string healthText = Stringf("HP: %i/%i", m_currentHealth, m_definition->m_maxHealth);
	DebugAddScreenText(healthText, Vec2(boundsMidX, m_renderBounds.m_mins.y), 25.0f, Vec2(0.5f, 1.0f), 0.0f, Rgba8(255, 0, 0), Rgba8(255, 0, 0));
	std::string blockText = Stringf("Block: %i", m_currentBlock);
	DebugAddScreenText(blockText, Vec2(boundsMidX, m_renderBounds.m_mins.y - 25.0f), 25.0f, Vec2(0.5f, 1.0f), 0.0f, Rgba8(0, 100, 255), Rgba8(0, 100, 255));

	int finalDamage = m_currentIntention->m_damage;
	if (finalDamage != 0)
	{
		for (int effectIndex = 0; effectIndex < m_effects.size(); effectIndex++)
		{
			Effect const& effect = m_effects[effectIndex];

			if (effect.m_definition->m_modDealtDamage)
			{
				if (effect.m_definition->m_usePercentage)
				{
					finalDamage = static_cast<int>(static_cast<float>(finalDamage) * effect.m_definition->m_percentModifier);
				}
				else
				{
					finalDamage += effect.m_stack;
				}
			}
		}

		for (int effectIndex = 0; effectIndex < g_theGame->m_player->m_effects.size(); effectIndex++)
		{
			Effect const& effect = g_theGame->m_player->m_effects[effectIndex];

			if (effect.m_definition->m_modReceivedDamage)
			{
				if (effect.m_definition->m_usePercentage)
				{
					finalDamage = static_cast<int>(static_cast<float>(finalDamage) * effect.m_definition->m_percentModifier);
				}
				else
				{
					finalDamage += effect.m_stack;
				}
			}
		}
	}

	int finalBlock = m_currentIntention->m_block;
	if (finalBlock != 0)
	{
		for (int effectIndex = 0; effectIndex < m_effects.size(); effectIndex++)
		{
			Effect const& effect = m_effects[effectIndex];

			if (effect.m_definition->m_modBlock)
			{
				if (effect.m_definition->m_usePercentage)
				{
					finalBlock = static_cast<int>(static_cast<float>(finalBlock) * effect.m_definition->m_percentModifier);
				}
				else
				{
					finalBlock += effect.m_stack;
				}
			}
		}
	}

	//display intention
	//-------------------------
	//NOTE: Intention text currently does not support enemies being able to do more than one bonus effect per action
	//-------------------------
	std::string intentionText = Stringf("Next:\n%i damage,\n%i block", finalDamage, finalBlock);
	if (m_encounter->m_turnState == TurnState::ENEMY)
	{
		DebugAddScreenText(intentionText, Vec2(boundsMidX, m_renderBounds.m_maxs.y + 25.0f), 25.0f, Vec2(0.5f, 0.0f), 0.0f, Rgba8(), Rgba8());
		if (m_currentIntention->m_cardToAdd != nullptr)
		{
			std::string intentionStatusText = Stringf("Inflict: %s", m_currentIntention->m_cardToAdd->m_name.c_str());
			DebugAddScreenText(intentionStatusText, Vec2(boundsMidX, m_renderBounds.m_maxs.y), 20.0f, Vec2(0.5f, 0.0f), 0.0f, Rgba8(), Rgba8());
		}
		else if (m_currentIntention->m_inflictEffect != nullptr)
		{
			std::string inflictEffectText = Stringf("Inflict: %i %s", m_currentIntention->m_inflictEffectStack, m_currentIntention->m_inflictEffect->m_name.c_str());
			DebugAddScreenText(inflictEffectText, Vec2(boundsMidX, m_renderBounds.m_maxs.y), 20.0f, Vec2(0.5f, 0.0f), 0.0f, Rgba8(), Rgba8());
		}
		else if (m_currentIntention->m_gainEffect != nullptr)
		{
			std::string gainEffectText = Stringf("Gain: %i %s", m_currentIntention->m_gainEffectStack, m_currentIntention->m_gainEffect->m_name.c_str());
			DebugAddScreenText(gainEffectText, Vec2(boundsMidX, m_renderBounds.m_maxs.y), 20.0f, Vec2(0.5f, 0.0f), 0.0f, Rgba8(), Rgba8());
		}
		else if (m_currentIntention->m_preparing)
		{
			DebugAddScreenText("Preparing...", Vec2(boundsMidX, m_renderBounds.m_maxs.y), 20.0f, Vec2(0.5f, 0.0f), 0.0f, Rgba8(), Rgba8());
		}
	}
	else
	{
		DebugAddScreenText(intentionText, Vec2(boundsMidX, m_renderBounds.m_maxs.y + 25.0f), 25.0f, Vec2(0.5f, 0.0f), 0.0f, Rgba8(255, 100, 100), Rgba8(255, 100, 100));
		if (m_currentIntention->m_cardToAdd != nullptr)
		{
			std::string intentionStatusText = Stringf("Inflict: %s", m_currentIntention->m_cardToAdd->m_name.c_str());
			DebugAddScreenText(intentionStatusText, Vec2(boundsMidX, m_renderBounds.m_maxs.y), 20.0f, Vec2(0.5f, 0.0f), 0.0f, Rgba8(255, 100, 100), Rgba8(255, 100, 100));
		}
		else if (m_currentIntention->m_inflictEffect != nullptr)
		{
			std::string inflictEffectText = Stringf("Inflict: %i %s", m_currentIntention->m_inflictEffectStack, m_currentIntention->m_inflictEffect->m_name.c_str());
			DebugAddScreenText(inflictEffectText, Vec2(boundsMidX, m_renderBounds.m_maxs.y), 20.0f, Vec2(0.5f, 0.0f), 0.0f, Rgba8(255, 100, 100), Rgba8(255, 100, 100));
		}
		else if (m_currentIntention->m_gainEffect != nullptr)
		{
			std::string gainEffectText = Stringf("Gain: %i %s", m_currentIntention->m_gainEffectStack, m_currentIntention->m_gainEffect->m_name.c_str());
			DebugAddScreenText(gainEffectText, Vec2(boundsMidX, m_renderBounds.m_maxs.y), 20.0f, Vec2(0.5f, 0.0f), 0.0f, Rgba8(255, 100, 100), Rgba8(255, 100, 100));
		}
		else if (m_currentIntention->m_preparing)
		{
			DebugAddScreenText("Preparing...", Vec2(boundsMidX, m_renderBounds.m_maxs.y), 20.0f, Vec2(0.5f, 0.0f), 0.0f, Rgba8(255, 100, 100), Rgba8(255, 100, 100));
		}
	}

	//render effect icons
	for (int effectIndex = 0; effectIndex < m_effects.size(); effectIndex++)
	{
		Effect const& effect = m_effects[effectIndex];

		float minX = m_renderBounds.m_mins.x + static_cast<float>(40 * effectIndex);
		float minY = m_renderBounds.m_mins.y - 90.0f;
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
		DebugAddMessage(effectText, 0.0f, Rgba8(255, 0, 0), Rgba8(255, 0, 0));
	}*/
}


//
//public enemy utilities
//
void Enemy::PerformCurrentIntention()
{
	//calculate amount of damage to deal
	int finalDamage = m_currentIntention->m_damage;
	if (finalDamage != 0)
	{
		for (int effectIndex = 0; effectIndex < m_effects.size(); effectIndex++)
		{
			Effect const& effect = m_effects[effectIndex];

			if (effect.m_definition->m_modDealtDamage)
			{
				if (effect.m_definition->m_usePercentage)
				{
					finalDamage = static_cast<int>(static_cast<float>(finalDamage) * effect.m_definition->m_percentModifier);
				}
				else
				{
					finalDamage += effect.m_stack;
				}
			}
		}
		for (int effectIndex = 0; effectIndex < g_theGame->m_player->m_effects.size(); effectIndex++)
		{
			Effect const& effect = g_theGame->m_player->m_effects[effectIndex];

			if (effect.m_definition->m_modReceivedDamage)
			{
				if (effect.m_definition->m_usePercentage)
				{
					finalDamage = static_cast<int>(static_cast<float>(finalDamage) * effect.m_definition->m_percentModifier);
				}
				else
				{
					finalDamage += effect.m_stack;
				}
			}
		}
	}
	g_theGame->m_player->TakeDamage(finalDamage);

	//calculate amount of block to gain
	int finalBlock = m_currentIntention->m_block;
	if (finalBlock != 0)
	{
		for (int effectIndex = 0; effectIndex < m_effects.size(); effectIndex++)
		{
			Effect const& effect = m_effects[effectIndex];

			if (effect.m_definition->m_modBlock)
			{
				if (effect.m_definition->m_usePercentage)
				{
					finalBlock = static_cast<int>(static_cast<float>(finalBlock) * effect.m_definition->m_percentModifier);
				}
				else
				{
					finalBlock += effect.m_stack;
				}
			}
		}
	}
	GainBlock(finalBlock);

	if (m_currentIntention->m_cardToAdd != nullptr)
	{
		Card* addedCard = new Card(m_currentIntention->m_cardToAdd, g_theGame->m_player);
		if (g_theGame->m_player->m_drawPile.size() == 0)
		{
			g_theGame->m_player->m_drawPile.emplace_back(addedCard);
		}
		else
		{
			int randomPos = g_rng.RollRandomIntLessThan(static_cast<int>(g_theGame->m_player->m_drawPile.size()));
			g_theGame->m_player->m_drawPile.emplace(g_theGame->m_player->m_drawPile.begin() + randomPos, addedCard);
		}
		g_theGame->m_player->m_tempAddedCards.emplace_back(addedCard);

		std::string statusText = Stringf("Added %s to\ndraw pile", m_currentIntention->m_cardToAdd->m_name.c_str());
		DebugAddScreenText(statusText, Vec2(375.0f, 700.0f), 27.5f, Vec2(0.5f, 1.0f), 2.0f, Rgba8(255, 100, 0), Rgba8(255, 100, 0));
	}

	if (m_currentIntention->m_gainEffect != nullptr)
	{
		ReceiveEffect(m_currentIntention->m_gainEffect, m_currentIntention->m_gainEffectStack);
	}

	if (m_currentIntention->m_inflictEffect != nullptr)
	{
		g_theGame->m_player->ReceiveEffect(m_currentIntention->m_inflictEffect, m_currentIntention->m_inflictEffectStack);
	}
}


void Enemy::ChooseNextIntention()
{
	int intentionIndex = 0;
	
	if (m_definition->m_intentionMode == IntentionMode::RANDOM)
	{
		intentionIndex = g_rng.RollRandomIntLessThan(static_cast<int>(m_definition->m_intentions.size()));
	}
	else if (m_definition->m_intentionMode == IntentionMode::LOOP_ALL)
	{
		intentionIndex = m_encounter->m_turnNumber % m_definition->m_intentions.size();
	}
	else if (m_definition->m_intentionMode == IntentionMode::LOOP_LAST)
	{
		if (m_encounter->m_turnNumber < m_definition->m_intentions.size())
		{
			intentionIndex = m_encounter->m_turnNumber;
		}
		else
		{
			intentionIndex = static_cast<int>(m_definition->m_intentions.size()) - 1;
		}
	}

	m_currentIntention = &m_definition->m_intentions[intentionIndex];
}


void Enemy::TakeDamage(int damageAmount)
{
	//do damage to block first
	int startingBlock = m_currentBlock;
	m_currentBlock = GetClamped(m_currentBlock - damageAmount, 0, m_currentBlock);

	int damageReduction = startingBlock - m_currentBlock;
	int finalDamageAmount = damageAmount - damageReduction;

	//then do damage to health
	m_currentHealth = GetClamped(m_currentHealth - finalDamageAmount, 0, m_definition->m_maxHealth);

	float boundsMidX = (m_renderBounds.m_mins.x + m_renderBounds.m_maxs.x) * 0.5f;

	if (damageReduction > 0)
	{
		std::string blockDamageText = Stringf("-%i", damageReduction);
		DebugAddScreenText(blockDamageText, Vec2(boundsMidX - 120.0f, 325.0f), 30.0f, Vec2(1.0f, 1.0f), 2.0f, Rgba8(0, 100, 255), Rgba8(0, 100, 255));
	}
	if (finalDamageAmount > 0)
	{
		std::string damageText = Stringf("-%i", finalDamageAmount);
		DebugAddScreenText(damageText, Vec2(boundsMidX - 120.0f, 350.0f), 30.0f, Vec2(1.0f, 1.0f), 2.0f, Rgba8(255, 0, 0), Rgba8(255, 0, 0));

		m_renderColor.g = 0;
		m_renderColor.b = 0;
	}

	g_theGame->BeginScreenShake(static_cast<float>(damageAmount * 0.4f));
}


void Enemy::GainBlock(int blockAmount)
{
	m_currentBlock += blockAmount;

	float boundsMidX = (m_renderBounds.m_mins.x + m_renderBounds.m_maxs.x) * 0.5f;

	if (blockAmount > 0)
	{
		std::string blockText = Stringf("+%i", blockAmount);
		DebugAddScreenText(blockText, Vec2(boundsMidX - 120.0f, 325.0f), 30.0f, Vec2(1.0f, 1.0f), 2.0f, Rgba8(0, 100, 255), Rgba8(0, 100, 255));
		g_theAudio->StartSound(g_blockSound);
	}
}


void Enemy::ReceiveEffect(EffectDefinition const* definition, int stack)
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
				std::string artifactMessage = "Debuff\nBlocked";
				DebugAddScreenText(artifactMessage, Vec2(m_renderBounds.GetCenter().x + 300.0f, m_renderBounds.m_maxs.y + 75.0f), 27.5f, Vec2(0.5f, 1.0f), 2.0f, Rgba8(255, 100, 0), Rgba8(255, 100, 0));

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
		m_effects.emplace_back(Effect(definition, stack, nullptr, this));
	}
}
