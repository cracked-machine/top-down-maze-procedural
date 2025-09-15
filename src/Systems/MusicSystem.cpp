#include <BaseSystem.hpp>
#include <MusicSystem.hpp>
#include <Persistent/MusicVolume.hpp>
#include <SFML/Audio/Sound.hpp>

namespace ProceduralMaze::Sys {

MusicSystem::MusicSystem( std::shared_ptr<entt::basic_registry<entt::entity>> reg,
                          std::filesystem::path music_path )
    : BaseSystem( reg ),
      m_music_buffer( std::move( music_path ) )
{
  m_music.setLooping( true );
}

void MusicSystem::init_context()
{
  if ( not m_reg->ctx().contains<Cmp::Persistent::MusicVolume>() )
  {
    m_reg->ctx().emplace<Cmp::Persistent::MusicVolume>();
  }
}

void MusicSystem::update_music_playback( Function func )
{
  if ( func == Function::PLAY && m_music.getStatus() != sf::Sound::Status::Playing )
  {
    m_music.play();
  }
  else if ( func == Function::STOP && m_music.getStatus() == sf::Sound::Status::Playing )
  {
    m_music.stop();
  }
}

void MusicSystem::start_music_fade_out()
{
  if ( !m_music_fading && m_music.getStatus() == sf::Sound::Status::Playing )
  {
    SPDLOG_DEBUG( " Starting music fade out" );
    m_music_fading = true;
    m_prefade_volume = m_reg->ctx().get<Cmp::Persistent::MusicVolume>()();
    m_music_fade_clock.restart();
  }
}

void MusicSystem::update_volume()
{
  if ( not m_music_fading )
  {
    float target_volume = m_reg->ctx().get<Cmp::Persistent::MusicVolume>()();
    m_music.setVolume( target_volume );
  }
  else
  {
    float elapsed = m_music_fade_clock.getElapsedTime().asSeconds();
    float fade_progress = elapsed / kFadeDuration;
    SPDLOG_DEBUG( " Music fade progress: {}", fade_progress );

    if ( fade_progress >= 1.0f )
    {
      // Fade complete - stop music
      m_music.setVolume( 0.0f );
      m_music.stop();
      m_music_fading = false;
    }
    else
    {
      // Interpolate volume from original to 0
      float current_volume = m_prefade_volume * ( 1.0f - fade_progress );
      m_music.setVolume( current_volume );
    }
  }
}

} // namespace ProceduralMaze::Sys