#include <Sprites/MultiSprite.hpp>
#include <Systems/BaseSystem.hpp>
#include <spdlog/spdlog.h>

namespace ProceduralMaze::Sprites
{

MultiSprite::MultiSprite( SpriteMetaType type, const std::filesystem::path &tilemap_path, const std::vector<uint32_t> &tilemap_picks,
                          SpriteSize grid_size, unsigned int sprites_per_frame, unsigned int sprites_per_sequence, std::vector<bool> solid_mask )
    : m_sprite_type{ type },
      m_grid_size{ grid_size.width, grid_size.height },
      m_sprites_per_frame{ sprites_per_frame },
      m_sprites_per_sequence{ sprites_per_sequence },
      m_solid_mask{ std::move( solid_mask ) }
{
  m_tilemap_texture = std::make_unique<sf::Texture>();
  if ( !m_tilemap_texture->loadFromFile( tilemap_path ) )
  {
    SPDLOG_ERROR( "Unable to load tile map {}", tilemap_path.string() );
    throw std::runtime_error( "Unable to load tile map: " + tilemap_path.string() );
  }
  SPDLOG_DEBUG( "Loaded tilemap texture: {}", tilemap_path.string() );
  if ( !add_sprite( tilemap_picks ) )
  {
    SPDLOG_CRITICAL( "Failed to load tilemap: {}", tilemap_path.string() );
    throw std::runtime_error( "Failed to load tilemap: " + tilemap_path.string() );
  }
}

MultiSprite::MultiSprite( SpriteMetaType type, sf::Texture tilemap_texture, const std::vector<uint32_t> &tilemap_picks, SpriteSize grid_size,
                          unsigned int sprites_per_frame, unsigned int sprites_per_sequence, std::vector<bool> solid_mask )
    : m_sprite_type{ type },
      m_grid_size{ grid_size.width, grid_size.height },
      m_sprites_per_frame{ sprites_per_frame },
      m_sprites_per_sequence{ sprites_per_sequence },
      m_solid_mask{ std::move( solid_mask ) }
{
  SPDLOG_DEBUG( "Loaded tilemap texture" );
  m_tilemap_texture = std::make_shared<sf::Texture>( std::move( tilemap_texture ) );
  if ( !add_sprite( tilemap_picks ) )
  {
    SPDLOG_CRITICAL( "Failed to load tilemap" );
    throw std::runtime_error( "Failed to load tilemap" );
  }
}

bool MultiSprite::add_sprite( const std::vector<uint32_t> &tilemap_picks )
{
  if ( tilemap_picks.empty() )
  {
    SPDLOG_WARN( "Empty tilemap_picks provided" );
    return false;
  }

  SPDLOG_INFO( "{} requested {} tiles", m_sprite_type, tilemap_picks.size() );
  for ( const auto &tile_idx : tilemap_picks )
  {
    sf::Vector2u kGridSquareSizePixels{ Sys::BaseSystem::kGridSquareSizePixels.x * m_grid_size.width,
                                        Sys::BaseSystem::kGridSquareSizePixels.y * m_grid_size.height };

    sf::VertexArray current_va( sf::PrimitiveType::Triangles, 6 );

    // Calculate texture coordinates based on 16x16 base tile grid (not sprite grid)
    const int base_tiles_per_row = m_tilemap_texture->getSize().x / Sys::BaseSystem::kGridSquareSizePixels.x;
    const int base_tile_x = tile_idx % base_tiles_per_row;
    const int base_tile_y = tile_idx / base_tiles_per_row;

    const int tu = base_tile_x * Sys::BaseSystem::kGridSquareSizePixels.x;
    const int tv = base_tile_y * Sys::BaseSystem::kGridSquareSizePixels.y;

    // draw the two triangles within local space using the sprite dimensions
    current_va[0].position = sf::Vector2f( 0, 0 );
    current_va[1].position = sf::Vector2f( kGridSquareSizePixels.x, 0 );
    current_va[2].position = sf::Vector2f( 0, kGridSquareSizePixels.y );
    current_va[3].position = sf::Vector2f( 0, kGridSquareSizePixels.y );
    current_va[4].position = sf::Vector2f( kGridSquareSizePixels.x, 0 );
    current_va[5].position = sf::Vector2f( kGridSquareSizePixels.x, kGridSquareSizePixels.y );

    current_va[0].texCoords = sf::Vector2f( tu, tv );
    current_va[1].texCoords = sf::Vector2f( tu + kGridSquareSizePixels.x, tv );
    current_va[2].texCoords = sf::Vector2f( tu, tv + kGridSquareSizePixels.y );
    current_va[3].texCoords = sf::Vector2f( tu, tv + kGridSquareSizePixels.y );
    current_va[4].texCoords = sf::Vector2f( tu + kGridSquareSizePixels.x, tv );
    current_va[5].texCoords = sf::Vector2f( tu + kGridSquareSizePixels.x, tv + kGridSquareSizePixels.y );

    SPDLOG_DEBUG( "Added sprite index {} at texture coords ({}, {})", tile_idx, tu, tv );
    SPDLOG_DEBUG( "Sprite vertex positions: [({}, {}) to  ({}, {})]", current_va[0].position.x, current_va[0].position.y, current_va[5].position.x,
                  current_va[5].position.y );

    m_va_list.push_back( current_va );
  }
  SPDLOG_DEBUG( "Created {} sprites ", m_va_list.size() );
  return true;
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
  states.texture = m_tilemap_texture.get();

  // draw the vertex array
  target.draw( m_selected_vertices, states );
}
} // namespace ProceduralMaze::Sprites