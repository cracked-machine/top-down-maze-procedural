#ifndef __SPRITES_SPRITE_FACTORY_HPP__
#define __SPRITES_SPRITE_FACTORY_HPP__

#include "MultiSprite.hpp"
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/System/Vector2.hpp>

#include <Components/Random.hpp>
#include <Components/RandomFloat.hpp>
#include <Sprites/MultiSprite.hpp>

#include <string>
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

  enum class SpriteMetaType
  {
    WALL = 0,
    ROCK = 1,
    POT = 2,
    BONES = 3,
    DETONATED = 4,
    PLAYER = 5,
    BOMB = 6,
    EXTRA_HEALTH = 7,
    EXTRA_BOMBS = 8,
    INFINI_BOMBS = 9,
    CHAIN_BOMBS = 10,
    LOWER_WATER = 11,
    NPC = 12,
    EXPLOSION = 13,
    FOOTSTEPS = 14,
    SINKHOLE = 15,
    CORRUPTION = 16
  };

private:
  // This holds sprite type, tilemap texture path and tilemap indices in a
  // single place
  struct SpriteMetaData
  {
    SpriteMetaType type;
    std::string name;
    float weight;
    ProceduralMaze::Sprites::MultiSprite m_multisprite{};
  };

  // list declaring multisprite instance and their associated metadata
  std::vector<SpriteMetaData> m_sprite_metadata_list = {
      { SpriteFactory::SpriteMetaType::WALL, "WALL", 1.f,
        MultiSprite{ "res/textures/walls_and_doors.png", { 0, 1, 2, 3, 4, 5, 6 } } },
      { SpriteFactory::SpriteMetaType::ROCK, "ROCK", 40.f, MultiSprite{ "res/textures/objects.png", { 147, 148 } } },
      { SpriteFactory::SpriteMetaType::POT, "POT", 1.f, MultiSprite{ "res/textures/objects.png", { 337, 339, 341 } } },
      { SpriteFactory::SpriteMetaType::BONES, "BONES", 1.f, MultiSprite{ "res/textures/objects.png", { 270, 271 } } },
      { SpriteFactory::SpriteMetaType::NPC, "NPC", 1.f,
        MultiSprite{ "res/textures/tilemap_packed.png", { 108, 121, 111 } } },
      { SpriteFactory::SpriteMetaType::DETONATED, "DETONATED", 1.f,
        MultiSprite{ "res/textures/tilemap_packed.png", { 42 } } },
      { SpriteFactory::SpriteMetaType::PLAYER, "PLAYER", 1.f,
        MultiSprite{ "res/textures/conjurer1.png", { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11 } } },
      // MultiSprite{ "res/textures/tilemap_packed.png", { 84, 85, 86, 87, 88, 96, 97, 98, 99, 100 } } },
      { SpriteFactory::SpriteMetaType::BOMB, "BOMB", 1.f, MultiSprite{ "res/textures/bomb.png", { 0 } } },
      { SpriteFactory::SpriteMetaType::EXTRA_HEALTH, "EXTRA_HEALTH", 30.f,
        MultiSprite{ "res/textures/objects.png", { 32 } } },
      { SpriteFactory::SpriteMetaType::EXTRA_BOMBS, "EXTRA_BOMBS", 40.f,
        MultiSprite{ "res/textures/objects.png", { 67 } } },
      { SpriteFactory::SpriteMetaType::INFINI_BOMBS, "INFINI_BOMBS", 1.f,
        MultiSprite{ "res/textures/objects.png", { 35 } } },
      { SpriteFactory::SpriteMetaType::CHAIN_BOMBS, "CHAIN_BOMBS", 20.f,
        MultiSprite{ "res/textures/objects.png", { 34 } } },
      { SpriteFactory::SpriteMetaType::LOWER_WATER, "LOWER_WATER", 40.f,
        MultiSprite{ "res/textures/objects.png", { 33 } } },
      { SpriteFactory::SpriteMetaType::EXPLOSION, "EXPLOSION", 1.f,
        MultiSprite{ "res/textures/fire_effect_and_bullet_16x16.png", { 66, 67, 68, 69, 70, 71 } } },
      { SpriteFactory::SpriteMetaType::FOOTSTEPS, "FOOTSTEPS", 1.f,
        MultiSprite{ "res/textures/footsteps.png", { 0 } } },
      { SpriteFactory::SpriteMetaType::SINKHOLE, "SINKHOLE", 1.f, MultiSprite{ "res/textures/sinkhole.png", { 0 } } },
      { SpriteFactory::SpriteMetaType::CORRUPTION, "CORRUPTION", 1.f,
        MultiSprite{ "res/textures/corruption.png", { 0 } } } };

public:
  /**
   * @brief Selects a random sprite type and texture index based on provided weights.
   *
   * This function randomly selects a sprite type from the given list of types and returns
   * it along with a corresponding texture index. The selection can be weighted if weights
   * are provided. If no weights are provided, weights are either taken from initial values
   * or default to 1.0f.
   *
   * @param type_list A vector of SpriteMetaType values to choose from
   * @param weights Optional vector of weights corresponding to each type in type_list.
   *               If no weights are provided, weights are either taken from initial values
   *               or default to 1.0f.
   *
   * @return A pair containing the selected SpriteMetaType and its associated texture index
   *
   * @throws std::invalid_argument if weights vector size doesn't match type_list size (when weights
   * provided)
   * @throws std::runtime_error if type_list is empty
   */
  std::pair<SpriteMetaType, std::size_t> get_random_type_and_texture_index( std::vector<SpriteMetaType> type_list,
                                                                            std::vector<float> weights = {} ) const;

  /**
   * @brief Retrieves a MultiSprite object based on the specified sprite meta type.
   *
   * This method searches for and returns a MultiSprite that corresponds to the given
   * SpriteMetaType. If no matching MultiSprite is found for the specified type,
   * the method returns std::nullopt.
   *
   * @param type The SpriteMetaType to search for
   * @return std::optional<Sprites::MultiSprite> The MultiSprite if found, std::nullopt otherwise
   */
  std::optional<Sprites::MultiSprite> get_multisprite_by_type( SpriteMetaType type ) const;

  /**
   * @brief Converts a SpriteMetaType enumeration value to its corresponding string representation.
   *
   * This function takes a SpriteMetaType enumeration value and returns a human-readable
   * string that represents the type of sprite data. This is useful for debugging,
   * logging, or serialization purposes where the enum value needs to be displayed
   * or stored as text.
   *
   * @param type The SpriteMetaType enumeration value to convert to string
   * @return std::string The string representation of the sprite meta type
   */
  std::string get_spritedata_type_string( SpriteFactory::SpriteMetaType type ) const;

private:
  // get metadata by type
  /**
   * @brief Retrieves sprite metadata by sprite type
   *
   * Searches for and returns the sprite metadata associated with the specified
   * sprite type. This method allows lookup of sprite configuration data such as
   * texture coordinates, dimensions, and other properties based on the sprite type.
   *
   * @param type The sprite type to search for
   * @return std::optional<SpriteMetaData> containing the sprite metadata if found,
   *         or std::nullopt if no metadata exists for the given type
   */
  std::optional<SpriteMetaData> get_spritedata_by_type( SpriteMetaType type ) const;

  // Internal use function used by get_random_type_and_texture_index()
  std::optional<SpriteMetaData> get_random_spritedata( std::vector<SpriteMetaType> type_list,
                                                       std::vector<float> weights = {} ) const;
}; // namespace ProceduralMaze::Sprites

} // namespace ProceduralMaze::Sprites

#endif // __SPRITES_SPRITE_FACTORY_HPP__