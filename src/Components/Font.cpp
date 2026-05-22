

#include <Components/Font.hpp>
#include <spdlog/spdlog.h>

namespace Game::Cmp
{

Font::Font( std::string font_path )
{

  if ( !openFromFile( font_path ) ) { SPDLOG_ERROR( "Unable to load {}", font_path ); }
}

} // namespace Game::Cmp
