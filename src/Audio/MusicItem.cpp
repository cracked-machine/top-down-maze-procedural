#include <Audio/MusicItem.hpp>
#include <spdlog/spdlog.h>

namespace Game::Audio
{

MusicData::MusicData( const std::filesystem::path &filepath )
    : control(
          [&filepath]()
          {
            if ( not std::filesystem::exists( filepath ) )
            {
              SPDLOG_CRITICAL( "Music file {} does not exist!", filepath.string() );
              throw std::runtime_error( "Music file not found: " + filepath.string() );
            }
            return filepath;
          }() )
{
}

} // namespace Game::Audio