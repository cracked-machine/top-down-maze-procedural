
#include <Audio/SoundBank.hpp>
#include <SFML/Audio/PlaybackDevice.hpp>
#include <SFML/Audio/SoundChannel.hpp>
#include <spdlog/spdlog.h>

namespace ProceduralMaze::Audio
{

SoundBank::SoundBank() { sounds.emplace( "fallback", EffectsData{ "res/audio/fallback.wav" } ); }

void SoundBank::init()
{
  SPDLOG_DEBUG( "Initializing SoundBank..." );
  // Initialize sound effects
  sounds.emplace( "wormhole_jump", EffectsData{ "res/audio/wormhole_jump.wav" } );
  sounds.emplace( "shrine_lighting", EffectsData{ "res/audio/shrine_lighting.wav" } );

  sounds.emplace( "secret", EffectsData{ "res/audio/secret.wav" } );
  sounds.emplace( "drop_relic", EffectsData{ "res/audio/drop_relic.wav" } );
  sounds.emplace( "spawn_ghost", EffectsData{ "res/audio/spawn_ghost.wav" } );
  sounds.emplace( "spawn_skeleton", EffectsData{ "res/audio/spawn_skeleton.wav" } );
  sounds.emplace( "damage_player", EffectsData{ "res/audio/damage_player.wav" } );

  sounds.emplace( "footsteps", EffectsData{ "res/audio/footsteps.wav" } );
  sounds.emplace( "bomb_fuse", EffectsData{ "res/audio/fuse.wav" } );
  sounds.emplace( "bomb_detonate", EffectsData{ "res/audio/detonate.wav" } );
  sounds.emplace( "drop_loot", EffectsData{ "res/audio/drop_loot.wav" } );
  sounds.emplace( "get_loot", EffectsData{ "res/audio/get_loot.wav" } );
  sounds.emplace( "get_key", EffectsData{ "res/audio/get_key.wav" } );
  sounds.emplace( "break_pot", EffectsData{ "res/audio/break_pot.wav" } );
  sounds.emplace( "hit_pot", EffectsData{ "res/audio/hit_pot.mp3" } );
  sounds.emplace( "hit_grave", EffectsData{ "res/audio/hit_grave.mp3" } );
  sounds.emplace( "digging_earth", EffectsData{ "res/audio/digging_earth.mp3" } );
  sounds.emplace( "chopping", EffectsData{ "res/audio/chopping.mp3" } );
  sounds.emplace( "chopping_final", EffectsData{ "res/audio/chopping_final.mp3" } );
  sounds.emplace( "axe_whip", EffectsData{ "res/audio/axe_whip.wav" } );
  sounds.emplace( "skele_death", EffectsData{ "res/audio/skele_death.wav" } );

  sounds.emplace( "pickaxe1", EffectsData{ "res/audio/pickaxe1.wav" } );
  sounds.emplace( "pickaxe2", EffectsData{ "res/audio/pickaxe2.wav" } );
  sounds.emplace( "pickaxe3", EffectsData{ "res/audio/pickaxe3.wav" } );
  sounds.emplace( "pickaxe4", EffectsData{ "res/audio/pickaxe4.wav" } );
  sounds.emplace( "pickaxe5", EffectsData{ "res/audio/pickaxe5.wav" } );
  sounds.emplace( "pickaxe6", EffectsData{ "res/audio/pickaxe6.wav" } );
  sounds.emplace( "pickaxe_final", EffectsData{ "res/audio/pickaxe_final.wav" } );

  sounds.emplace( "crypt_open", EffectsData{ "res/audio/crypt_open2.wav" } );
  sounds.emplace( "crypt_locked", EffectsData{ "res/audio/crypt_locked.wav" } );
  sounds.emplace( "crypt_room_shuffle", EffectsData{ "res/audio/crypt_room_shuffle.wav" } );
  sounds.emplace( "crypt_lever_open", EffectsData{ "res/audio/crypt_lever_open.mp3" } );
  sounds.emplace( "crypt_chest_open", EffectsData{ "res/audio/crypt_chest_open.wav" } );

  sounds.emplace( "player_blood_splat", EffectsData{ "res/audio/player_blood_splat.mp3" } );
  sounds.emplace( "bubbling_lava", EffectsData{ "res/audio/underwater.wav" } );
  sounds.emplace( "spike_trap", EffectsData{ "res/audio/spike_trap.wav" } );

  // Initialize music (can't use initializer list because sf::Music is move-only)
  music.emplace( "title_music", MusicData{ "res/audio/EerieScifi.mp3" } );
  music.emplace( "game_music", MusicData{ "res/audio/SadWindyOrgan.mp3" } );
}

void SoundBank::update_effects_volume( float volume )
{
  for ( auto &[name, sound_data] : sounds )
  {
    sound_data.control.setVolume( volume );
  }
}

void SoundBank::update_music_volume( float volume )
{
  for ( auto &[name, music_data] : music )
  {
    music_data.control.setVolume( volume );
  }
}

sf::Sound &SoundBank::get_effect( const std::string &name )
{
  if ( sounds.find( name ) != sounds.end() ) { return sounds.at( name ).control; }
  SPDLOG_CRITICAL( "Audio effect {} not found!", name );
  throw std::runtime_error( "Audio effect not found: " + name );
}

sf::Music &SoundBank::get_music( const std::string &name )
{
  if ( music.find( name ) != music.end() ) { return music.at( name ).control; }
  SPDLOG_CRITICAL( "Music {} not found!", name );
  throw std::runtime_error( "Music not found: " + name );
}

// Generate a simple tone
sf::SoundBuffer SoundBank::generate_tone( float frequency, float duration, unsigned int sample_rate )
{
  unsigned int sample_count = static_cast<unsigned int>( duration * sample_rate );
  std::vector<int16_t> samples( sample_count );

  const float amplitude = 30000.0f; // Max amplitude for 16-bit audio
  const float increment = frequency / sample_rate;
  float x = 0.0f;

  for ( unsigned int i = 0; i < sample_count; ++i )
  {
    samples[i] = static_cast<int16_t>( amplitude * std::sin( x * 2.0f * 3.14159f ) );
    x += increment;
  }

  sf::SoundBuffer buffer;
  if ( buffer.loadFromSamples( samples.data(), sample_count, 1, sample_rate, { sf::SoundChannel::Mono } ) ) { return buffer; }
  SPDLOG_CRITICAL( "Failed to generate tone" );
  throw std::runtime_error( "Failed to generate tone" );
}

} // namespace ProceduralMaze::Audio