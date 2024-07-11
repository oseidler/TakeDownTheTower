#include "Game/Card.hpp"
#include "Game/CardDefinition.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Game.hpp"
#include "Game/App.hpp"
#include "Game/Map.hpp"
#include "Game/Player.hpp"
#include "Game/Enemy.hpp"
#include "Game/Encounter.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Renderer/DebugRenderSystem.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Audio/AudioSystem.hpp"


//
//constructor
//
Card::Card(CardDefinition const* definition, Player* player)
	: m_definition(definition)
	, m_player(player)
{
}


//
//public game flow functions
//
void Card::Update(int cardPosition, bool& wasCardPlayed, bool isReward)
{
	if (g_theInput->WasKeyJustPressed(KEYCODE_LMB))
	{
		Vec2 mousePosition = g_theInput->GetCursorNormalizedPosition();
		Vec2 gameMousePosition = Vec2(mousePosition.x * SCREEN_CAMERA_SIZE_X, mousePosition.y * SCREEN_CAMERA_SIZE_Y);
		
		//go ahead and return if click was within bounds of a button
		if (IsPointInsideAABB2D(gameMousePosition, g_theGame->m_endTurnButton->m_bounds))
		{
			return;
		}
		
		if (cardPosition > MAX_HAND_SIZE - 1)
		{
			//don't bother if the card somehow has a higher position than what it should have
			return;
		}

		//I know this is a dumb way of doing this but I just want something functional for now
		AABB2 cardBounds = AABB2();
		if (cardPosition == 0)
		{
			if (isReward)
			{
				cardBounds = AABB2(SCREEN_CAMERA_CENTER_X - 360.0f, SCREEN_CAMERA_CENTER_Y - 120.0f, SCREEN_CAMERA_CENTER_X - 200.0f, SCREEN_CAMERA_CENTER_Y + 120.0f);
			}
			else
			{
				cardBounds = AABB2(SCREEN_CAMERA_CENTER_X - 440.0f, 10.0f, SCREEN_CAMERA_CENTER_X - 280.0f, 250.0f);
			}
		}
		else if (cardPosition == 1)
		{
			if (isReward)
			{
				cardBounds = AABB2(SCREEN_CAMERA_CENTER_X - 80.0f, SCREEN_CAMERA_CENTER_Y - 120.0f, SCREEN_CAMERA_CENTER_X + 80.0f, SCREEN_CAMERA_CENTER_Y + 120.0f);
			}
			else
			{
				cardBounds = AABB2(SCREEN_CAMERA_CENTER_X - 260.0f, 10.0f, SCREEN_CAMERA_CENTER_X - 100.0f, 250.0f);
			}
		}
		else if (cardPosition == 2)
		{
			if (isReward)
			{
				cardBounds = AABB2(SCREEN_CAMERA_CENTER_X + 200.0f, SCREEN_CAMERA_CENTER_Y - 120.0f, SCREEN_CAMERA_CENTER_X + 360.0f, SCREEN_CAMERA_CENTER_Y + 120.0f);
			}
			else
			{
				cardBounds = AABB2(SCREEN_CAMERA_CENTER_X - 80.0f, 10.0f, SCREEN_CAMERA_CENTER_X + 80.0f, 250.0f);
			}
		}
		else if (cardPosition == 3)
		{
			cardBounds = AABB2(SCREEN_CAMERA_CENTER_X + 100.0f, 10.0f, SCREEN_CAMERA_CENTER_X + 260.0f, 250.0f);
		}
		else if (cardPosition == 4)
		{
			cardBounds = AABB2(SCREEN_CAMERA_CENTER_X + 280.0f, 10.0f, SCREEN_CAMERA_CENTER_X + 440.0f, 250.0f);
		}

		//different input logic for card rewards
		if (isReward)
		{
			if (IsPointInsideAABB2D(gameMousePosition, cardBounds))
			{
				wasCardPlayed = true;	//use wasCardPlayed to pass whether card reward was chosen in this instance
				return;
			}
		}

		AABB2 playingFieldBounds = AABB2(0.0f, 255.0f, SCREEN_CAMERA_SIZE_X, SCREEN_CAMERA_SIZE_Y);

		if (IsPointInsideAABB2D(gameMousePosition, cardBounds))
		{
			if (this != m_player->m_selectedCard)
			{
				m_player->m_selectedCard = this;
				g_theAudio->StartSound(g_cardSound);
			}
			else
			{
				m_player->m_selectedCard = nullptr;
			}
		}
		else if(IsPointInsideAABB2D(gameMousePosition, playingFieldBounds))
		{
			if (this == m_player->m_selectedCard)
			{
				m_player->m_selectedCard = nullptr;

				if (!m_definition->m_isPlayable)
				{
					return;
				}
				
				if (m_definition->m_targetMode == TargetMode::ONE)
				{
					//for now, just temporarily get the first enemy
					Encounter* encounter = g_theGame->m_map->m_allEncounters[g_theGame->m_map->m_currentEncounterNumber];
					Enemy* enemyTarget = nullptr;
					for (int enemyIndex = 0; enemyIndex < encounter->m_currentEnemies.size(); enemyIndex++)
					{
						Enemy* enemy = encounter->m_currentEnemies[enemyIndex];

						if (enemy != nullptr && enemy->m_currentHealth > 0 && IsPointInsideAABB2D(gameMousePosition, enemy->m_renderBounds))
						{
							enemyTarget = enemy;
						}
					}

					if (enemyTarget != nullptr)
					{
						wasCardPlayed = m_player->PlayCard(this, enemyTarget);
					}
				}
				else
				{
					wasCardPlayed = m_player->PlayCard(this, nullptr);
				}
			}
		}
	}
}


void Card::Render(int cardPosition, bool isReward) const
{
	if (cardPosition > MAX_HAND_SIZE - 1)
	{
		//don't bother if the card somehow has a higher position than what it should have
		return;
	}

	//I know this is a dumb way of doing this but I just want something functional for now
	AABB2 cardBounds = AABB2();
	if (cardPosition == 0)
	{
		if (isReward)
		{
			cardBounds = AABB2(SCREEN_CAMERA_CENTER_X - 360.0f, SCREEN_CAMERA_CENTER_Y - 120.0f, SCREEN_CAMERA_CENTER_X - 200.0f, SCREEN_CAMERA_CENTER_Y + 120.0f);
		}
		else
		{
			cardBounds = AABB2(SCREEN_CAMERA_CENTER_X - 440.0f, 10.0f, SCREEN_CAMERA_CENTER_X - 280.0f, 250.0f);
		}
	}
	else if (cardPosition == 1)
	{
		if (isReward)
		{
			cardBounds = AABB2(SCREEN_CAMERA_CENTER_X - 80.0f, SCREEN_CAMERA_CENTER_Y - 120.0f, SCREEN_CAMERA_CENTER_X + 80.0f, SCREEN_CAMERA_CENTER_Y + 120.0f);
		}
		else
		{
			cardBounds = AABB2(SCREEN_CAMERA_CENTER_X - 260.0f, 10.0f, SCREEN_CAMERA_CENTER_X - 100.0f, 250.0f);
		}
	}
	else if (cardPosition == 2)
	{
		if (isReward)
		{
			cardBounds = AABB2(SCREEN_CAMERA_CENTER_X + 200.0f, SCREEN_CAMERA_CENTER_Y - 120.0f, SCREEN_CAMERA_CENTER_X + 360.0f, SCREEN_CAMERA_CENTER_Y + 120.0f);
		}
		else
		{
			cardBounds = AABB2(SCREEN_CAMERA_CENTER_X -80.0f, 10.0f, SCREEN_CAMERA_CENTER_X + 80.0f, 250.0f);
		}
	}
	else if (cardPosition == 3)
	{
		cardBounds = AABB2(SCREEN_CAMERA_CENTER_X + 100.0f, 10.0f, SCREEN_CAMERA_CENTER_X + 260.0f, 250.0f);
	}
	else if (cardPosition == 4)
	{
		cardBounds = AABB2(SCREEN_CAMERA_CENTER_X + 280.0f, 10.0f, SCREEN_CAMERA_CENTER_X + 440.0f, 250.0f);
	}

	std::vector<Vertex_PCU> cardVerts;

	AddVertsForAABB2(cardVerts, cardBounds);

	g_theRenderer->BindTexture(m_definition->m_sprite);
	g_theRenderer->BindShader(nullptr);
	g_theRenderer->DrawVertexArray(cardVerts);

	//print cost of card
	if (m_definition->m_isPlayable)
	{
		std::string energyCost = Stringf("%i", m_definition->m_cost);
		DebugAddScreenText(energyCost, Vec2(cardBounds.m_mins.x + 10.5f, cardBounds.m_maxs.y - 17.5f), 15.0f, Vec2(0.0f, 1.0f), 0.0f, Rgba8(0, 0, 0), Rgba8(0, 0, 0));
	}

	//print card name and description
	std::vector<Vertex_PCU> textVerts;
	AABB2 nameBox = AABB2(cardBounds.m_mins.x + 35.0f, cardBounds.m_maxs.y - 40.0f, cardBounds.m_maxs.x - 30.0f, cardBounds.m_maxs.y - 25.0f);
	g_font->AddVertsForTextInBox2D(textVerts, nameBox, 15.0f, m_definition->m_name, Rgba8(0, 0, 0), 0.8f, Vec2(0.5f, 1.0f));

	AABB2 descBox = AABB2(cardBounds.m_mins.x + 30.0f, cardBounds.m_mins.y + 20.0f, cardBounds.m_maxs.x - 30.0f, (cardBounds.m_mins.y + cardBounds.m_maxs.y) * 0.5f);
	std::string const description = m_definition->m_description;
	g_font->AddVertsForTextInBox2D(textVerts, descBox, 10.0f, description, Rgba8(), 0.7f, Vec2(0.5f, 0.5f));

	g_theRenderer->BindTexture(&g_font->GetTexture());
	g_theRenderer->DrawVertexArray(textVerts);

	//if card is selected card, draw overlay on top of it
	if (m_player->m_selectedCard == this)
	{
		std::vector<Vertex_PCU> overlayVerts;
		AddVertsForAABB2(overlayVerts, cardBounds, Rgba8(255, 255, 255, 75));

		g_theRenderer->BindTexture(nullptr);
		g_theRenderer->DrawVertexArray(overlayVerts);
	}
}


//
//public card actions
//
void Card::Play(Enemy* enemyTarget, Encounter* currentEncounter) const
{
	//deal damage
	if (m_definition->m_targetMode == TargetMode::ONE)
	{
		//calculate final damage amount
		int finalDamage = m_definition->m_damage;
		for (int effectIndex = 0; effectIndex < m_player->m_effects.size(); effectIndex++)
		{
			Effect const& effect = m_player->m_effects[effectIndex];

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
		for (int effectIndex = 0; effectIndex < enemyTarget->m_effects.size(); effectIndex++)
		{
			Effect const& effect = enemyTarget->m_effects[effectIndex];

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

		for (int hitNum = 0; hitNum < m_definition->m_numHits; hitNum++)
		{
			enemyTarget->TakeDamage(finalDamage);
		}

		//inflict effect
		if (m_definition->m_inflictEffect != nullptr)
		{
			enemyTarget->ReceiveEffect(m_definition->m_inflictEffect, m_definition->m_inflictEffectStack);
		}
	}
	else if (m_definition->m_targetMode == TargetMode::ALL)
	{
		for (int enemyIndex = 0; enemyIndex < currentEncounter->m_currentEnemies.size(); enemyIndex++)
		{
			Enemy* enemy = currentEncounter->m_currentEnemies[enemyIndex];
			if (enemy != nullptr && enemy->m_currentHealth > 0)
			{
				//calculate final damage amount
				int finalDamage = m_definition->m_damage;
				for (int effectIndex = 0; effectIndex < m_player->m_effects.size(); effectIndex++)
				{
					Effect const& effect = m_player->m_effects[effectIndex];

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
				for (int effectIndex = 0; effectIndex < enemy->m_effects.size(); effectIndex++)
				{
					Effect const& effect = enemy->m_effects[effectIndex];

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

				for (int hitNum = 0; hitNum < m_definition->m_numHits; hitNum++)
				{
					enemy->TakeDamage(finalDamage);
				}
			}

			//inflict effect
			if (m_definition->m_inflictEffect != nullptr)
			{
				enemy->ReceiveEffect(m_definition->m_inflictEffect, m_definition->m_inflictEffectStack);
			}
		}
	}

	switch (m_definition->m_attackType)
	{
	case AttackType::SLICE:		   g_theAudio->StartSound(g_attackSliceSound); break;
	case AttackType::PIERCE:	   g_theAudio->StartSound(g_attackPierceSound); break;
	case AttackType::LIGHT_IMPACT: g_theAudio->StartSound(g_attackLightImpactSound); break;
	case AttackType::HEAVY_IMPACT: g_theAudio->StartSound(g_attackHeavyImpactSound); break;
	case AttackType::FIRE:		   g_theAudio->StartSound(g_attackFireSound); break;
	case AttackType::MAGIC:		   g_theAudio->StartSound(g_attackMagicSound); break;
	}

	//calculate final block amount
	int finalBlock = m_definition->m_block;
	if (finalBlock != 0)
	{
		for (int effectIndex = 0; effectIndex < m_player->m_effects.size(); effectIndex++)
		{
			Effect const& effect = m_player->m_effects[effectIndex];

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

	//gain block
	m_player->GainBlock(finalBlock);

	//restore health
	m_player->RestoreHealth(m_definition->m_restoreHP);

	//draw card
	for (int drawNum = 0; drawNum < m_definition->m_cardsDrawn; drawNum++)
	{
		m_player->DrawCard();
	}

	//gain energy
	m_player->GainEnergy(m_definition->m_energyGain);

	//gain effect
	if (m_definition->m_gainEffect != nullptr)
	{
		m_player->ReceiveEffect(m_definition->m_gainEffect, m_definition->m_gainEffectStack);
	}
}
