#include <MultiSprite.hpp>

namespace ProceduralMaze::Sprites {
const sf::Vector2u MultiSprite::kDefaultSpriteDimensions{ 16, 16 };

bool MultiSprite::pick( std::size_t idx, const std::string &caller )
{
  if ( m_va_list.empty() )
  {
    SPDLOG_WARN( "pick() called on empty sprite list from {}", caller );
    return false;
  }

  if ( idx >= m_va_list.size() )
  {
    SPDLOG_WARN( "{}: pick() index {} out of range (size: {}), using index 0", caller, idx, m_va_list.size() );
    idx = 0;
  }

  m_selected_vertices = m_va_list[idx];
  return true;
}

bool MultiSprite::add_sprite( const std::filesystem::path &tilemap_path, const std::vector<uint32_t> &tilemap_picks )
{
  if ( tilemap_picks.empty() )
  {
    SPDLOG_WARN( "Empty tilemap_picks provided" );
    return false;
  }

  if ( !m_tilemap_texture.loadFromFile( tilemap_path ) )
  {
    SPDLOG_ERROR( "Unable to load tile map {}", tilemap_path.string() );
    return false; // Don't terminate, let caller handle
  }

  for ( const auto &tile_idx : tilemap_picks )
  {

    sf::VertexArray current_va( sf::PrimitiveType::Triangles, 6 );
    const int tu = tile_idx % ( m_tilemap_texture.getSize().x / kDefaultSpriteDimensions.x );
    const int tv = tile_idx / ( m_tilemap_texture.getSize().x / kDefaultSpriteDimensions.x );

    // draw the two triangles within local space using the `kDefaultSpriteDimensions`
    current_va[0].position = sf::Vector2f( 0, 0 );
    current_va[1].position = sf::Vector2f( kDefaultSpriteDimensions.x, 0 );
    current_va[2].position = sf::Vector2f( 0, kDefaultSpriteDimensions.y );
    current_va[3].position = sf::Vector2f( 0, kDefaultSpriteDimensions.y );
    current_va[4].position = sf::Vector2f( kDefaultSpriteDimensions.x, 0 );
    current_va[5].position = sf::Vector2f( kDefaultSpriteDimensions.x, kDefaultSpriteDimensions.y );

    current_va[0].texCoords = sf::Vector2f( tu * kDefaultSpriteDimensions.x, tv * kDefaultSpriteDimensions.y );
    current_va[1].texCoords = sf::Vector2f( ( tu + 1 ) * kDefaultSpriteDimensions.x, tv * kDefaultSpriteDimensions.y );
    current_va[2].texCoords = sf::Vector2f( tu * kDefaultSpriteDimensions.x, ( tv + 1 ) * kDefaultSpriteDimensions.y );
    current_va[3].texCoords = sf::Vector2f( tu * kDefaultSpriteDimensions.x, ( tv + 1 ) * kDefaultSpriteDimensions.y );
    current_va[4].texCoords = sf::Vector2f( ( tu + 1 ) * kDefaultSpriteDimensions.x, tv * kDefaultSpriteDimensions.y );
    current_va[5].texCoords = sf::Vector2f( ( tu + 1 ) * kDefaultSpriteDimensions.x,
                                            ( tv + 1 ) * kDefaultSpriteDimensions.y );
    SPDLOG_TRACE( "  - Added tile index {} (tu={},tv={})", tile_idx, tu, tv );

    m_va_list.push_back( current_va );
  }
  SPDLOG_INFO( "Requested {} tiles ... Created {} sprites from texture {}: ", tilemap_picks.size(), m_va_list.size(),
               tilemap_path.string() );
  return true;
}

void MultiSprite::set_pick_opacity( uint8_t alpha )
{
  for ( std::size_t idx = 0; idx < m_selected_vertices.getVertexCount(); ++idx )
    m_selected_vertices[idx].color.a = alpha;
}

void MultiSprite::draw( sf::RenderTarget &target, sf::RenderStates states ) const
{
  if ( m_va_list.empty() || m_selected_vertices.getVertexCount() == 0 )
  {
    SPDLOG_WARN( "No sprites to draw" );
    return; // Don't terminate, just skip drawing
  }

  // apply the transform
  states.transform *= getTransform();

  // apply the tileset texture
  states.texture = &m_tilemap_texture;

  // draw the vertex array
  target.draw( m_selected_vertices, states );
}
} // namespace ProceduralMaze::Sprites