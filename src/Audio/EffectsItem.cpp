#include <Audio/EffectsItem.hpp>
#include <spdlog/spdlog.h>
namespace ProceduralMaze::Audio {

EffectsData::EffectsData( const std::filesystem::path &filepath )
    : buffer( [&filepath]() {
        if ( not std::filesystem::exists( filepath ) )
        {
          SPDLOG_CRITICAL( "Sound effect file {} does not exist!", filepath.string() );
          std::terminate();
        }
        return std::make_unique<sf::SoundBuffer>( filepath );
      }() ),
      control( *buffer )
{
}

EffectsData::EffectsData( EffectsData &&other ) noexcept
    : buffer( std::move( other.buffer ) ),
      control( *buffer )
{
}

EffectsData &EffectsData::operator=( EffectsData &&other ) noexcept
{
  buffer = std::move( other.buffer );
  control.setBuffer( *buffer );
  return *this;
}

} // namespace ProceduralMaze::Audio