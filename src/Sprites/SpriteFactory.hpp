#ifndef __SPRITES_SPRITE_FACTORY_HPP__
#define __SPRITES_SPRITE_FACTORY_HPP__

#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/System/Vector2.hpp>
#include <nlohmann/json.hpp>

#include <Components/Random.hpp>
#include <Sprites/MultiSprite.hpp>

#include <string>
#include <unordered_map>
#include <vector>

namespace ProceduralMaze::Sprites {

// This class manages the creation MultiSprites objects in the game.
// Contains logic for random selection of sprite types based on weights.
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

  //! @brief  Selects a random sprite type and texture index based on provided weights.
  //! This function randomly selects a sprite type from the given list of types and returns
  //! it along with a corresponding texture index. The selection can be weighted if weights
  //! are provided. If no weights are provided, weights are either taken from initial values
  //! or default to 1.0f.
  //! @param type_list A vector of SpriteMetaType values to choose from
  //! @param weights Optional vector of weights corresponding to each type in type_list.
  //!               If no weights are provided, weights are either taken from initial values
  //!               or default to 1.0f.
  //! @return A pair containing the selected SpriteMetaType and its associated texture index
  std::pair<SpriteMetaType, std::size_t> get_random_type_and_texture_index( std::vector<SpriteMetaType> type_list,
                                                                            std::vector<float> weights = {} );

  //! @brief Get the all sprite types by pattern object
  //!
  //! @param pattern partial string pattern to match sprite types
  //! @return std::vector<SpriteMetaType>
  std::vector<SpriteMetaType> get_all_sprite_types_by_pattern( const std::string &pattern );

  //! @brief  Retrieves a MultiSprite object based on the specified sprite meta type.
  //! This method searches for and returns a MultiSprite that corresponds to the given
  //! SpriteMetaType. If no matching MultiSprite is found for the specified type,
  //! the method returns std::nullopt.
  //! @param type The SpriteMetaType to search for
  //! @return Sprites::MultiSprite&  The MultiSprite if found, std::nullopt otherwise
  Sprites::MultiSprite &get_multisprite_by_type( const SpriteMetaType &type );

  //! @brief Converts a SpriteMetaType enumeration value to its corresponding string representation.
  //! This function takes a SpriteMetaType enumeration value and returns a human-readable
  //! string that represents the type of sprite data. This is useful for debugging,
  //! logging, or serialization purposes where the enum value needs to be displayed
  //! or stored as text.
  //! @param type The SpriteMetaType enumeration value to convert to string
  //! @return std::string The string representation of the sprite meta type
  std::string get_spritedata_type_string( const SpriteMetaType &type );

  //! @brief Get a vector of all SpriteMetaType objects
  //! @return std::vector<SpriteMetaType>
  std::vector<SpriteMetaType> get_all_sprite_types();

  //! @brief Checks if a sprite type exists in the factory.
  //! This function checks if the specified sprite type is present in the factory's
  //! metadata map. It can be used to verify if a particular sprite type has been
  //! registered and is available for use.
  //! @param type The SpriteMetaType to check for existence
  //! @return true if the sprite type exists, false otherwise
  bool has_sprite_type( const SpriteMetaType &type ) const;

private:
  //! @brief Metadata for a single sprite. This is mainly a legacy struct to hold the MultiSprite along with its weight.
  //         Previously held more data before MultiSprite encapsulated it.
  struct SpriteMetaData
  {
    float weight;
    ProceduralMaze::Sprites::MultiSprite m_multisprite{};
  };

  //! @brief Retrieves sprite metadata by sprite type
  //! Searches for and returns the sprite metadata associated with the specified
  //! sprite type. This method allows lookup of sprite configuration data such as
  //! texture coordinates, dimensions, and other properties based on the sprite type.
  //! @param type The SpriteMetaType to search for
  //! @return SpriteMetaData&  The SpriteMetaData if found, m_error_metadata otherwise
  SpriteMetaData &get_spritedata_by_type( const SpriteMetaType &type );

  // Internal use function used by get_random_type_and_texture_index()
  const SpriteMetaData &get_random_spritedata( std::vector<SpriteMetaType> type_list, std::vector<float> weights = {} );

  //! @brief Map of sprite types to their corresponding metadata
  std::unordered_map<SpriteMetaType, SpriteMetaData> m_sprite_metadata_map{};

  //! @brief Error texture for missing sprites
  sf::Texture m_error_texture;

  //! @brief Metadata for the error texture
  //! This contains information about the error texture's properties
  SpriteMetaData m_error_metadata;

}; // namespace ProceduralMaze::Sprites

} // namespace ProceduralMaze::Sprites

#endif // __SPRITES_SPRITE_FACTORY_HPP__