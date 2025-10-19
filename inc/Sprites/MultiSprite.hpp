#ifndef __SPRITES_MULTISPRITE2_HPP__
#define __SPRITES_MULTISPRITE2_HPP__

#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/Transform.hpp>
#include <SFML/Graphics/Vertex.hpp>
#include <SFML/Graphics/VertexArray.hpp>
#include <SFML/System/Vector2.hpp>

#include <cstdint>
#include <filesystem>

#include <spdlog/spdlog.h>
#include <string>
#include <vector>

namespace ProceduralMaze::Sprites {

typedef struct SpriteSize
{
  unsigned int width{ 1 };
  unsigned int height{ 1 };
} SpriteSize;

class MultiSprite : public sf::Drawable, public sf::Transformable
{
public:
  // Used by SpriteFactory::SpriteMetaData struct declaration
  MultiSprite() = default;

  /**
   * @brief Constructs a MultiSprite with a single tilemap and specified tile indices.
   *
   * @param tilemap_path Path to the tilemap image file to load
   * @param tilemap_picks Vector of tile indices to extract from the tilemap
   *
   * @throws std::runtime_error If the tilemap fails to load or is invalid
   */
  explicit MultiSprite( const std::filesystem::path &tilemap_path, const std::vector<uint32_t> &tilemap_picks,
                        SpriteSize grid_size = { 1, 1 }, unsigned int sprites_per_frame = 1,
                        unsigned int sprites_per_sequence = 1, std::vector<bool> solid_mask = {} )
      : m_grid_size{ grid_size.width, grid_size.height },
        m_sprites_per_frame{ sprites_per_frame },
        m_sprites_per_sequence{ sprites_per_sequence },
        m_solid_mask{ std::move( solid_mask ) }
  {
    if ( !add_sprite( tilemap_path, tilemap_picks ) )
    {
      SPDLOG_CRITICAL( "Failed to load tilemap: {}", tilemap_path.string() );
      std::terminate();
    }
  }

  MultiSprite( const MultiSprite & ) = default;
  MultiSprite &operator=( const MultiSprite & ) = default;
  MultiSprite( MultiSprite && ) = default;
  MultiSprite &operator=( MultiSprite && ) = default;
  ~MultiSprite() = default;

  /**
 * @brief Selects a sprite from the internal list by index.
 *
 * This method picks a specific sprite from the vertex array list and sets it as
 * the currently selected vertices. If the provided index is out of bounds, it
 * defaults to index 0 and logs a warning. If the sprite list is empty, the
 * operation fails and returns false.

 * @note The index is zero-based and contiguous. For example if SpriteFactory loaded 10, 45 and 89 from the tileset,
 * these are now mapped to 0, 1 and 2 respectively.
 *
 * @param idx The index of the sprite to select from the vertex array list
 * @param caller Identifier string for the calling context, used for logging purposes
 * @return true if a sprite was successfully selected, false if the sprite list is empty
 */
  bool pick( std::size_t idx, const std::string &caller = "unknown" );
  SpriteSize get_grid_size() const { return m_grid_size; }
  std::size_t get_sprite_count() const { return m_va_list.size(); }
  unsigned int get_sprites_per_frame() const { return m_sprites_per_frame; }
  unsigned int get_sprites_per_sequence() const { return m_sprites_per_sequence; }
  const std::vector<bool> &get_solid_mask() const { return m_solid_mask; }

  void set_pick_opacity( uint8_t alpha );

  sf::Texture m_tilemap_texture;

  /**
   * @brief Default dimensions for sprite frames in pixels.
   *
   * Defines the standard width and height (16x16 pixels) used for individual
   * sprite frames within a multi-sprite texture. This constant provides a
   * consistent default size for sprite rendering and frame calculations.
   */
  static constexpr sf::Vector2u kDefaultSpriteDimensions{ 16, 16 };
  void draw( sf::RenderTarget &target, sf::RenderStates states ) const override;

private:
  bool add_sprite( const std::filesystem::path &tilemap_path, const std::vector<uint32_t> &tilemap_picks );

  std::vector<sf::VertexArray> m_va_list;

  sf::VertexArray m_selected_vertices;

  // width and height grid size for the multi-sprite
  SpriteSize m_grid_size{ 1, 1 };

  // number of sprites per animation frame
  unsigned int m_sprites_per_frame{ 1 };

  unsigned int m_sprites_per_sequence{ 1 };

  // Indicates which 'sprite_indices' the player cannot traverse. Array size must match sprite_indices size.
  std::vector<bool> m_solid_mask;
};

} // namespace ProceduralMaze::Sprites

#endif // __SPRITES_MULTISPRITE2_HPP__