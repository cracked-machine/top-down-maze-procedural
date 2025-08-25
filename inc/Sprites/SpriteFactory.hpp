#ifndef __SPRITES_SPRITE_FACTORY_HPP__
#define __SPRITES_SPRITE_FACTORY_HPP__


#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/System/Vector2.hpp>
#include <algorithm>
#include <string>
#include <vector>
#include <Sprites/MultiSprite.hpp>
#include <Components/Random.hpp>
#include <Settings.hpp>

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
        ProceduralMaze::Sprites::MultiSprite get_multisprite() const { return m_multisprite; }
        
        
        std::vector<std::uint32_t> m_texture_index_choices;
    private:
        Type m_type;
        std::string m_type2string;
        ProceduralMaze::Sprites::MultiSprite m_multisprite{};
        std::string m_texture_path; 
    };

public:

    // this is a special case where these obstacles have more than one texture option
    std::tuple<SpriteFactory::MetaData, std::uint32_t> get_random_meta_obstacle()
    {
        ProceduralMaze::Cmp::Random m_meta_obstacle_random_picker{1, 3};
        auto index = m_meta_obstacle_random_picker.gen();
        ProceduralMaze::Cmp::Random texture_idx_picker(0, m_metadata_list[index].m_texture_index_choices.size() - 1);
        return {
            m_metadata_list[index], // the metadata object
           texture_idx_picker.gen() // a random index within the size of MetaData::m_texture_index_choices vector, not the value at the index, but the index itself
        };
    }

    std::vector<ProceduralMaze::Sprites::MultiSprite> create_multisprites_list() const
    {
        std::vector<ProceduralMaze::Sprites::MultiSprite> sprites;
        for (const auto& meta : m_metadata_list)
        {
            sprites.push_back(meta.get_multisprite());
        }
        return sprites;
    }



    MetaData get_wall_obstacle() const
    {
        return m_metadata_list[static_cast<int>(SpriteFactory::Type::WALL)];
    }

    std::string get_metadata_type_string(SpriteFactory::Type type) const
    {
        auto it = std::find_if(m_metadata_list.begin(), m_metadata_list.end(),
            [type](const MetaData& meta) { return meta.get_type() == type; });
        if (it != m_metadata_list.end())
        {
            return it->get_type_string();
        }
        return {};
    }

    std::vector<SpriteFactory::MetaData> m_metadata_list = {
        {SpriteFactory::Type::WALL,     "WALL",  "res/Pixel Lands Dungeons/objects.png", {247}},
        {SpriteFactory::Type::ROCK,     "ROCK",  "res/Pixel Lands Dungeons/objects.png", {147,148}},
        {SpriteFactory::Type::POT,      "POT",   "res/Pixel Lands Dungeons/objects.png", {337, 339, 341}},
        {SpriteFactory::Type::BONES,    "BONES", "res/Pixel Lands Dungeons/objects.png", {270, 271}},
        {SpriteFactory::Type::DETONATED, "DETONATED", "res/kenney_tiny-dungeon/Tilemap/tilemap_packed.png", {42}},
        {SpriteFactory::Type::PLAYER,    "PLAYER", "res/players.png", {0, 1, 2}, ProceduralMaze::Settings::PLAYER_SPRITE_SIZE},
        {SpriteFactory::Type::BOMB,      "BOMB",   "res/bomb.png", {0}}
    };
};
    
#endif // __SPRITES_SPRITE_FACTORY_HPP__