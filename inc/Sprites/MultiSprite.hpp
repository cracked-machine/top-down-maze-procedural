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
  explicit MultiSprite( const std::filesystem::path &tilemap_path, const std::vector<uint32_t> &tilemap_picks )
  {
    if ( !add_sprite( tilemap_path, tilemap_picks ) )
    {
      throw std::runtime_error( "Failed to load tilemap: " + tilemap_path.string() );
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
  std::size_t get_sprite_count() const { return m_va_list.size(); }
  void set_pick_opacity( uint8_t alpha );

  sf::Texture m_tilemap_texture;
  static constexpr sf::Vector2u kDefaultSpriteDimensions{ 16, 16 };

private:
  bool add_sprite( const std::filesystem::path &tilemap_path, const std::vector<uint32_t> &tilemap_picks );

  void draw( sf::RenderTarget &target, sf::RenderStates states ) const override;

  std::vector<sf::VertexArray> m_va_list;

  sf::VertexArray m_selected_vertices;

  sf::Vector2u m_tile_size;
};

} // namespace ProceduralMaze::Sprites

#endif // __SPRITES_MULTISPRITE2_HPP__