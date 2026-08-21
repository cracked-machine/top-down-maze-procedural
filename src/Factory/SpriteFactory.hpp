#ifndef SRC_FACTORY_SPRITEFACTORY_HPP__
#define SRC_FACTORY_SPRITEFACTORY_HPP__

#include <Sprites/SpriteSheet.hpp>

#include <SFML/System/Vector2.hpp>
#include <source_location>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace Game::Sprites
{

//! @brief Manages the creation and lookup of SpriteSheet objects in the game.
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

  //! @brief Selects a random sprite type from the given list of types.
  //! @param type_list A selection list of SpriteMetaType values
  //! @return The selected SpriteMetaType, or "ERROR_SPRITE" if `type_list` is empty.
  SpriteMetaType get_random_type( std::vector<SpriteMetaType> type_list );

  //! @brief Get the all sprite types by pattern object
  //! Supports regex and plain text matching.
  //!
  //! @param pattern partial string pattern to match sprite types
  //! @return std::vector<SpriteMetaType>
  std::vector<SpriteMetaType> get_all_sprite_types_by_pattern( const std::string &pattern );

  //! @brief  Retrieves a SpriteSheet object based on the specified sprite meta type.
  //! This method searches for and returns a SpriteSheet that corresponds to the given
  //! SpriteMetaType. If the type is not found, it returns the error sprite.
  //! @param type The SpriteMetaType to search for
  //! @param loc Call site captured for the error message if the type is not found
  //! @return Sprites::SpriteSheet& const& The SpriteSheet object if found
  //! @throws std::runtime_error if the type is not found
  const Sprites::SpriteSheet &get_spritesheet_by_type( const SpriteMetaType &type, std::source_location loc = std::source_location::current() );

  //! @brief Get a vector of all SpriteMetaType objects
  //! @return std::vector<SpriteMetaType>
  std::vector<SpriteMetaType> get_all_sprite_types();
  std::unordered_set<SpriteMetaType> get_all_sprite_types_set();

  // Returns the pixel bounds of first sprite in array. Assumes that all sprites in the multi-sprite have the same size
  sf::Vector2f get_sprite_size_by_type( const SpriteMetaType &type, std::source_location loc = std::source_location::current() )
  {
    return get_spritedata_by_type( type, loc ).get_sprite_size();
  }
  std::string get_display_name_by_type( const SpriteMetaType &type, std::source_location loc = std::source_location::current() )
  {
    return get_spritedata_by_type( type, loc ).get_display_name();
  }

private:
  //! @brief Retrieves sprite metadata by sprite type
  //! Searches for and returns the sprite metadata associated with the specified
  //! sprite type. This method allows lookup of sprite configuration data such as
  //! texture coordinates, dimensions, and other properties based on the sprite type.
  //! @param type The SpriteMetaType to search for
  //! @param loc Call site captured for the error message if the type is not found
  //! @return SpriteMetaData& The SpriteMetaData if found
  //! @throws std::runtime_error if the type is not found
  const SpriteSheet &get_spritedata_by_type( const SpriteMetaType &type, std::source_location loc = std::source_location::current() );

  //! @brief Get the SpriteSheet for a randomly chosen type from `type_list`.
  //! @param type_list A selection list of SpriteMetaType values
  //! @return The randomly selected SpriteSheet, or the error sprite if `type_list` is empty.
  const SpriteSheet &get_random_spritedata( std::vector<SpriteMetaType> type_list );

  //! @brief Map of sprite types to their corresponding metadata
  std::unordered_map<SpriteMetaType, SpriteSheet> m_sprite_metadata_map;

  //! @brief Error texture for missing sprites
  sf::Texture m_error_texture;

  //! @brief Metadata for the error texture
  //! This contains information about the error texture's properties
  SpriteSheet m_error_metadata;

}; // namespace Game::Sprites

} // namespace Game::Sprites

#endif // SRC_FACTORY_SPRITEFACTORY_HPP__
