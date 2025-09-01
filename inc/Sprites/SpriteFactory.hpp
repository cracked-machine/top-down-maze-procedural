#ifndef __SPRITES_SPRITE_FACTORY_HPP__
#define __SPRITES_SPRITE_FACTORY_HPP__

#include "MultiSprite.hpp"
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/System/Vector2.hpp>

#include <Sprites/MultiSprite.hpp>
#include <Components/Random.hpp>
#include <Components/RandomFloat.hpp>
#include <Settings.hpp>

#include <numeric>
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
        BOMB = 6,
        EXTRA_HEALTH = 7,
        EXTRA_BOMBS = 8,
        INFINI_BOMBS = 9,
        CHAIN_BOMBS = 10,
        LOWER_WATER = 11,
        NPC = 12
    };

    const sf::Vector2u PLAYER_SPRITE_SIZE = {16, 16};
    const sf::Vector2u DEFAULT_SPRITE_SIZE = {16, 16};

private:
    // This holds sprite type, tilemap texture path and tilemap indices in a single place
    class MetaData {
    public:
        MetaData(
            Type type, 
            const std::string& type2string, 
            const std::string& texture_path, 
            const std::vector<std::uint32_t>& texture_index_choices,
            const sf::Vector2u &tileSize = sf::Vector2u(16, 16),
            float weight = 1.0f
        )
        : 
            m_texture_index_choices(texture_index_choices), 
            m_type(type), 
            m_type2string(type2string), 
            m_texture_path(texture_path), 
            m_weight(weight) 
        {
            m_multisprite.add_sprite(texture_path, texture_index_choices, tileSize, get_type_string());
        }

        Type get_type() const { return m_type; }
        std::string get_type_string() const { return m_type2string; }
        Sprites::MultiSprite get_multisprite() const { return m_multisprite; }
        float get_weight() const { return m_weight; }

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
        float m_weight{1.0f}; // Default weight of 1.0
    };

    // Metadata object list
    std::vector<SpriteFactory::MetaData> m_metadata_list = {
        {SpriteFactory::Type::WALL,           "WALL",  "res/walls_and_doors.png", {0,1,2,3,4, 5, 6}, DEFAULT_SPRITE_SIZE},
        {SpriteFactory::Type::ROCK,           "ROCK",  "res/Pixel Lands Dungeons/objects.png", {147,148}, DEFAULT_SPRITE_SIZE, 40.f,},
        {SpriteFactory::Type::POT,            "POT",   "res/Pixel Lands Dungeons/objects.png", {337, 339, 341}, DEFAULT_SPRITE_SIZE, 1.f},
        {SpriteFactory::Type::BONES,          "BONES", "res/Pixel Lands Dungeons/objects.png", {270, 271}, DEFAULT_SPRITE_SIZE, 1.f},
        {SpriteFactory::Type::NPC,            "NPC",           "res/kenney_tiny-dungeon/Tilemap/tilemap_packed.png", {108, 121, 111}, DEFAULT_SPRITE_SIZE, 1.f},
        {SpriteFactory::Type::DETONATED,      "DETONATED", "res/kenney_tiny-dungeon/Tilemap/tilemap_packed.png", {42}, DEFAULT_SPRITE_SIZE},
        {SpriteFactory::Type::PLAYER,         "PLAYER", "res/kenney_tiny-dungeon/Tilemap/tilemap_packed.png", 
            {84, 85,86,87,88,96,97,98,99,100}, PLAYER_SPRITE_SIZE},
        {SpriteFactory::Type::BOMB,           "BOMB",   "res/bomb.png", {0}, DEFAULT_SPRITE_SIZE},
        {SpriteFactory::Type::EXTRA_HEALTH,   "EXTRA_HEALTH",  "res/Pixel Lands Dungeons/objects.png", {32}, DEFAULT_SPRITE_SIZE, 30.f},
        {SpriteFactory::Type::EXTRA_BOMBS,    "EXTRA_BOMBS",   "res/Pixel Lands Dungeons/objects.png", {67 }, DEFAULT_SPRITE_SIZE, 40.f},
        {SpriteFactory::Type::INFINI_BOMBS,  "INFINI_BOMBS",  "res/Pixel Lands Dungeons/objects.png", {35 }, DEFAULT_SPRITE_SIZE, 1.f},
        {SpriteFactory::Type::CHAIN_BOMBS,   "CHAIN_BOMBS",   "res/Pixel Lands Dungeons/objects.png", {34 }, DEFAULT_SPRITE_SIZE, 20.f},
        {SpriteFactory::Type::LOWER_WATER,   "LOWER_WATER",   "res/Pixel Lands Dungeons/objects.png", {33 }, DEFAULT_SPRITE_SIZE, 40.f}
    };

public:

    // Pick a random metadata object from a list of types using a "roulette wheel selection" method:
    // Each Type has an associated weight. We get a random number between 0 and total weights.
    // We then accumulate the weights in turn comparing it against the random number
    std::optional<SpriteFactory::MetaData> get_random_metadata(std::vector<Type> type_list, std::vector<float> weights = {}) const
    {
        if (type_list.empty()) { SPDLOG_WARN("Type list is empty");  return std::nullopt; }

        // If weights aren't provided, use weights from metadata
        if (weights.empty()) {
            weights.reserve(type_list.size());
            for (auto type : type_list) {
                auto meta = get_metadata_by_type(type);
                if (meta) {
                    weights.push_back(meta->get_weight());
                } else {
                    weights.push_back(1.0f); // Default weight
                }
            }
        }
        
        // Ensure weights and type_list have same size
        if (weights.size() != type_list.size()) {
            weights.resize(type_list.size(), 1.0f);
        }

        float total_weight = std::accumulate(weights.begin(), weights.end(), 0.0f);

        // Generate random value between 0 and total weight
        Cmp::RandomFloat random_float(0.0f, total_weight);
        float random_val = random_float.gen();

        // Select based on weights
        float cumulative_weight = 0.0f;
        for (size_t i = 0; i < type_list.size(); ++i) {
            cumulative_weight += weights[i];
            if (random_val <= cumulative_weight) {
                return get_metadata_by_type(type_list[i]);
            }
        }

        // Fallback (shouldn't reach here normally)
        return get_metadata_by_type(type_list.back());
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