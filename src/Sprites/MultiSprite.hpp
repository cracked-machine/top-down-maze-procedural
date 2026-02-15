#ifndef __SPRITES_MULTISPRITE2_HPP__
#define __SPRITES_MULTISPRITE2_HPP__

#include <Sprites/SpriteMetaType.hpp>

#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/VertexArray.hpp>

#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

namespace ProceduralMaze::Sprites
{

typedef struct SpriteSize
{
  unsigned int width{ 1 };
  unsigned int height{ 1 };
  sf::Vector2f componentWiseMul( sf::Vector2f rhs ) const { return sf::Vector2f( width * rhs.x, height * rhs.y ); }
  sf::Vector2f componentWiseMul( sf::Vector2u rhs ) const { return sf::Vector2f( width * rhs.x, height * rhs.y ); }
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
  explicit MultiSprite( SpriteMetaType type, std::string display_name, const std::filesystem::path &tilemap_path,
                        const std::vector<uint32_t> &tilemap_picks, SpriteSize grid_size = { 1, 1 }, unsigned int sprites_per_frame = 1,
                        unsigned int sprites_per_sequence = 1, std::vector<bool> solid_mask = {} );

  explicit MultiSprite( SpriteMetaType type, std::string display_name, sf::Texture tilemap_texture, const std::vector<uint32_t> &tilemap_picks,
                        SpriteSize grid_size = { 1, 1 }, unsigned int sprites_per_frame = 1, unsigned int sprites_per_sequence = 1,
                        std::vector<bool> solid_mask = {} );

  MultiSprite( MultiSprite && ) = default;
  MultiSprite &operator=( MultiSprite && ) = default;
  ~MultiSprite() = default;

  SpriteSize get_grid_size() const { return m_grid_size; }
  std::size_t get_sprite_count() const { return m_va_list.size(); }
  unsigned int get_sprites_per_frame() const { return m_sprites_per_frame; }
  unsigned int get_sprites_per_sequence() const { return m_sprites_per_sequence; }
  const std::vector<bool> &get_solid_mask() const { return m_solid_mask; }

  const sf::Texture &get_texture() const { return *m_tilemap_texture; }
  std::string get_display_name() const { return m_display_name; }
  /**
   * @brief Default dimensions for sprite frames in pixels.
   *
   * Defines the standard width and height (16x16 pixels) used for individual
   * sprite frames within a multi-sprite texture. This constant provides a
   * consistent default size for sprite rendering and frame calculations.
   */
  // static constexpr sf::Vector2u Constants::kGridSquareSizePixels{ 16, 16 };
  void draw( sf::RenderTarget &target, sf::RenderStates states ) const override;

  // Returns the pixel bounds of first sprite in array. Assumes that all sprites in the multi-sprite
  // have the same size
  sf::Vector2f getSpriteSizePixels() const { return m_va_list[0].getBounds().size; }

  SpriteMetaType get_sprite_type() const { return m_sprite_type; }
  sf::VertexArray m_selected_vertices;
  std::vector<sf::VertexArray> m_va_list;

private:
  std::shared_ptr<sf::Texture> m_tilemap_texture;
  bool add_sprite( const std::vector<uint32_t> &tilemap_picks );

  SpriteMetaType m_sprite_type;

  std::string m_display_name{};
  // width and height grid size for the multi-sprite
  SpriteSize m_grid_size{ 1, 1 };

  // number of sprites per animation frame
  unsigned int m_sprites_per_frame{ 1 };

  unsigned int m_sprites_per_sequence{ 1 };

  // Indicates which 'sprite_indices' the player cannot traverse. Array size must match
  // sprite_indices size.
  std::vector<bool> m_solid_mask{};
};

} // namespace ProceduralMaze::Sprites

#endif // __SPRITES_MULTISPRITE2_HPP__