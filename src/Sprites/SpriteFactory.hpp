#ifndef SRC_SPRITES_SPRITE_FACTORY_HPP_
#define SRC_SPRITES_SPRITE_FACTORY_HPP_

#include <Sprites/MultiSprite.hpp>

#include <SFML/Graphics/Texture.hpp>
#include <SFML/System/Vector2.hpp>

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace ProceduralMaze::Sprites
{

// This class manages the creation MultiSprites objects in the game.
// The MultiSprite objects are stored in `m_multisprite_list`.
// TODO initialize `m_multisprite_list` from a JSON file
class SpriteFactory
{
public:
  SpriteFactory() = default;

  //! @brief Initializes the sprite factory
  //! This function loads sprite metadata from a JSON file and initializes the factory.
  void init();

  //! @brief Create a error sprite object
  //! This function creates a special error sprite to prevent engine crashes when handling invalid sprite data.
  void create_error_sprite();

  //! @brief  Selects a random sprite type and texture index.
  //! This function randomly selects a sprite type from the given list of types and returns
  //! it along with a corresponding texture index.
  //! @param type_list A selection list of SpriteMetaType values
  //! @return A pair containing the selected SpriteMetaType and its associated texture index
  std::pair<SpriteMetaType, std::size_t> get_random_type_and_texture_index( std::vector<SpriteMetaType> type_list );

  SpriteMetaType get_random_type( std::vector<SpriteMetaType> type_list );

  //! @brief Get the all sprite types by pattern object
  //! Supports regex and plain text matching.
  //!
  //! @param pattern partial string pattern to match sprite types
  //! @return std::vector<SpriteMetaType>
  std::vector<SpriteMetaType> get_all_sprite_types_by_pattern( const std::string &pattern );

  //! @brief  Retrieves a MultiSprite object based on the specified sprite meta type.
  //! This method searches for and returns a MultiSprite that corresponds to the given
  //! SpriteMetaType. If the type is not found, it returns the error sprite.
  //! @param type The SpriteMetaType to search for
  //! @return Sprites::MultiSprite& const& The MultiSprite object if found, error sprite otherwise
  const Sprites::MultiSprite &get_multisprite_by_type( const SpriteMetaType &type );

  //! @brief Get a vector of all SpriteMetaType objects
  //! @return std::vector<SpriteMetaType>
  std::vector<SpriteMetaType> get_all_sprite_types();
  std::unordered_set<SpriteMetaType> get_all_sprite_types_set();

  // Returns the pixel bounds of first sprite in array. Assumes that all sprites in the multi-sprite have the same size
  sf::Vector2f get_sprite_size_by_type( const SpriteMetaType &type ) { return get_spritedata_by_type( type ).getSpriteSizePixels(); }
  std::string get_display_name_by_type( const SpriteMetaType &type ) { return get_spritedata_by_type( type ).get_display_name(); }

private:
  //! @brief Retrieves sprite metadata by sprite type
  //! Searches for and returns the sprite metadata associated with the specified
  //! sprite type. This method allows lookup of sprite configuration data such as
  //! texture coordinates, dimensions, and other properties based on the sprite type.
  //! @param type The SpriteMetaType to search for
  //! @return SpriteMetaData&  The SpriteMetaData if found, m_error_metadata otherwise
  const MultiSprite &get_spritedata_by_type( const SpriteMetaType &type );

  // Internal use function used by get_random_type_and_texture_index()
  const MultiSprite &get_random_spritedata( std::vector<SpriteMetaType> type_list );

  //! @brief Map of sprite types to their corresponding metadata
  std::unordered_map<SpriteMetaType, MultiSprite> m_sprite_metadata_map{};

  //! @brief Error texture for missing sprites
  sf::Texture m_error_texture;

  //! @brief Metadata for the error texture
  //! This contains information about the error texture's properties
  MultiSprite m_error_metadata;

}; // namespace ProceduralMaze::Sprites

} // namespace ProceduralMaze::Sprites

#endif // SRC_SPRITES_SPRITE_FACTORY_HPP_