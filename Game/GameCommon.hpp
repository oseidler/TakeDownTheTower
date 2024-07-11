#pragma once
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Audio/AudioSystem.hpp"


//forward declarations
struct Vec2;
struct Rgba8;
class App;
class Renderer;
class InputSystem;
class Window;
class RandomNumberGenerator;
class Texture;
class BitmapFont;

//external declarations
extern App* g_theApp;
extern Renderer* g_theRenderer;
extern InputSystem* g_theInput;
extern AudioSystem* g_theAudio;
extern Window* g_theWindow;

extern RandomNumberGenerator g_rng;

extern Texture* g_playerSprite;
extern BitmapFont* g_font;

//sounds
extern SoundID g_attackSliceSound;
extern SoundID g_attackPierceSound;
extern SoundID g_attackLightImpactSound;
extern SoundID g_attackHeavyImpactSound;
extern SoundID g_attackFireSound;
extern SoundID g_attackMagicSound;
extern SoundID g_debuffSound;
extern SoundID g_buffSound;
extern SoundID g_nullifySound;
extern SoundID g_blockSound;
extern SoundID g_damageSound;
extern SoundID g_damageBlockedSound;
extern SoundID g_healSound;
extern SoundID g_cardSound;
extern SoundID g_campfireSound;
extern SoundPlaybackID g_campfireSoundPlayback;

//music
extern SoundID g_startMenuMusic;
extern SoundPlaybackID g_startMenuMusicPlayback;
extern SoundID g_battleMusic;
extern SoundPlaybackID g_battleMusicPlayback;
extern SoundID g_battle2Music;
extern SoundPlaybackID g_battle2MusicPlayback;
extern SoundID g_battle3Music;
extern SoundPlaybackID g_battle3MusicPlayback;
extern SoundID g_restStopMusic;
extern SoundPlaybackID g_restStopMusicPlayback;
extern SoundID g_bossMusic;
extern SoundPlaybackID g_bossMusicPlayback;
extern SoundID g_victoryMusic;
extern SoundPlaybackID g_victoryMusicPlayback;
extern SoundID g_finalRestStopMusic;
extern SoundPlaybackID g_finalRestStopMusicPlayback;
extern SoundID g_finalBossMusic;
extern SoundPlaybackID g_finalBossMusicPlayback;
extern SoundID g_finalVictoryMusic;
extern SoundPlaybackID g_finalVictoryMusicPlayback;


//gameplay constants
constexpr float SCREEN_CAMERA_SIZE_X = 1600.f;
constexpr float SCREEN_CAMERA_SIZE_Y = 800.f;
constexpr float SCREEN_CAMERA_CENTER_X = SCREEN_CAMERA_SIZE_X / 2.f;
constexpr float SCREEN_CAMERA_CENTER_Y = SCREEN_CAMERA_SIZE_Y / 2.f;

constexpr float SCREEN_SHAKE_MAX = 4.f;

constexpr float DEBUG_LINE_WIDTH = 0.1f;
constexpr float MAX_FRAME_SECONDS = 0.1f;

//debug drawing functions
void DebugDrawLine(Vec2 const& startPosition, Vec2 const& endPosition, float width, Rgba8 const& color);
void DebugDrawRing(Vec2 const& center, float radius, float width, Rgba8 const& color);