#ifndef __SPRITES_SPRITE_FACTORY_HPP__
#define __SPRITES_SPRITE_FACTORY_HPP__

#include "MultiSprite.hpp"
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/System/Vector2.hpp>

#include <Sprites/MultiSprite.hpp>
#include <Components/Random.hpp>
#include <Settings.hpp>

#include <vector>
#include <string>

namespace ProceduralMaze::Sprites {

class SpriteFactory {
public:

    SpriteFactory() = default;

    enum class Type
    {
        WALL = 0,
        ROCK = 1,
        POT = 2,
        BONES = 3,
        DETONATED = 4,
        PLAYER = 5,
        BOMB = 6
    };

private:
    // This holds sprite type, tilemap texture path and tilemap indices in a single place
    class MetaData {
    public:
        MetaData(Type type, const std::string& type2string, const std::string& texture_path, const std::vector<std::uint32_t>& texture_index_choices,
            const sf::Vector2u &tileSize = sf::Vector2u(16, 16))
            : m_type(type), m_type2string(type2string), m_texture_path(texture_path), m_texture_index_choices(texture_index_choices) 
        {
            m_multisprite.add_sprite(texture_path, texture_index_choices, tileSize, get_type_string());
        }

        Type get_type() const { return m_type; }
        std::string get_type_string() const { return m_type2string; }
        Sprites::MultiSprite get_multisprite() const { return m_multisprite; }

        // Get random value from `m_texture_index_choices`.
        // Use this when emplacing components into the registry.
        // Dont use this when rendering.
        std::size_t pick_random_texture_index() const
        {
            Cmp::Random random_picker(0, m_texture_index_choices.size() - 1);
            return random_picker.gen();
        }


        std::vector<std::uint32_t> m_texture_index_choices;
    private:
        Type m_type;
        std::string m_type2string;
        ProceduralMaze::Sprites::MultiSprite m_multisprite{};
        std::string m_texture_path; 
    };

    // Metadata object list
    std::vector<SpriteFactory::MetaData> m_metadata_list = {
        {SpriteFactory::Type::WALL,     "WALL",  "res/Pixel Lands Dungeons/objects.png", {247}},
        {SpriteFactory::Type::ROCK,     "ROCK",  "res/Pixel Lands Dungeons/objects.png", {147,148}},
        {SpriteFactory::Type::POT,      "POT",   "res/Pixel Lands Dungeons/objects.png", {337, 339, 341}},
        {SpriteFactory::Type::BONES,    "BONES", "res/Pixel Lands Dungeons/objects.png", {270, 271}},
        {SpriteFactory::Type::DETONATED, "DETONATED", "res/kenney_tiny-dungeon/Tilemap/tilemap_packed.png", {42}},
        {SpriteFactory::Type::PLAYER,    "PLAYER", "res/players.png", {0, 1, 2}, Settings::PLAYER_SPRITE_SIZE},
        {SpriteFactory::Type::BOMB,      "BOMB",   "res/bomb.png", {0}}
    };


public:


    // pick a random metadata object from a list of types
    std::optional<SpriteFactory::MetaData> get_random_metadata(std::vector<Type> type_list) const
    {
        if (type_list.empty()) { SPDLOG_WARN("Type list is empty");  return std::nullopt; }

        Cmp::Random random_picker(0, type_list.size() - 1);
        auto random_index = random_picker.gen();
        auto type = type_list[random_index];

        return get_metadata_by_type(type).value();
    }

    // get metadata by type
    std::optional<SpriteFactory::MetaData> get_metadata_by_type(Type type) const
    {
        auto it = std::find_if(m_metadata_list.begin(), m_metadata_list.end(),
            [type](const MetaData& meta) { return meta.get_type() == type; });
        if (it != m_metadata_list.end())
        {
            return *it;
        }
        return std::nullopt;
    }

    // get multisprite by type
    std::optional<Sprites::MultiSprite> get_multisprite_by_type(Type type) const
    {
        if (auto meta = get_metadata_by_type(type))
        {
            return meta->get_multisprite();
        }
        return std::nullopt;
    }

    // get metadata type string
    std::string get_metadata_type_string(SpriteFactory::Type type) const
    {
        if (auto meta = get_metadata_by_type(type))
        {
            return meta->get_type_string();
        }
        return "NOTFOUND";
    }
};
    

} // namespace ProceduralMaze::Sprites
#endif // __SPRITES_SPRITE_FACTORY_HPP__