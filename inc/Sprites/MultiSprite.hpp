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
    if ( !add_sprite( tilemap_path, tilemap_picks ) ) { throw std::runtime_error( "Failed to load tilemap: " + tilemap_path.string() ); }
  }

  MultiSprite( const MultiSprite & ) = default;
  MultiSprite &operator=( const MultiSprite & ) = default;
  MultiSprite( MultiSprite && ) = default;
  MultiSprite &operator=( MultiSprite && ) = default;
  ~MultiSprite() = default;

  bool pick( std::size_t idx, const std::string &caller = "unknown" );
  std::size_t get_sprite_count() const { return m_va_list.size(); }

  sf::Texture m_tilemap_texture;
  static const sf::Vector2u DEFAULT_SPRITE_SIZE;

private:
  bool add_sprite( const std::filesystem::path &tilemap_path, const std::vector<uint32_t> &tilemap_picks );

  void draw( sf::RenderTarget &target, sf::RenderStates states ) const override;

  std::vector<sf::VertexArray> m_va_list;

  sf::VertexArray m_selected_vertices;

  sf::Vector2u m_tile_size;
};

} // namespace ProceduralMaze::Sprites

#endif // __SPRITES_MULTISPRITE2_HPP__