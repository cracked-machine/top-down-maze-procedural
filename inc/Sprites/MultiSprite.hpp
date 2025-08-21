#ifndef __SPRITES_MULTISPRITE_HPP__
#define __SPRITES_MULTISPRITE_HPP__    

#include <Random.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/VertexArray.hpp>
#include <SFML/System/Vector2.hpp>
#include <spdlog/spdlog.h>
#include <vector>
#include <map>

namespace ProceduralMaze::Sprites {

class MultiSprite : public sf::Drawable, public sf::Transformable 
{
public:
    MultiSprite(
        const std::string &tilemap_path, 
        const std::vector<unsigned int> &tilemap_picks,
        const sf::Vector2u &tileSize = sf::Vector2u{16, 16}
    )
        : m_tilemap_picks(tilemap_picks)
    {
        if (!m_tilemap_texture.loadFromFile(tilemap_path)) {
            SPDLOG_CRITICAL("Unable to load tile map {}", tilemap_path);
        }

        for (const auto &tile_idx : tilemap_picks) {

            sf::VertexArray current_va(sf::PrimitiveType::Triangles, 6);
            const int tu = tile_idx % (m_tilemap_texture.getSize().x / tileSize.x);
            const int tv = tile_idx / (m_tilemap_texture.getSize().x / tileSize.x);

            // draw the two triangles within local space using the `tileSize`
            current_va[0].position = sf::Vector2f(0, 0);
            current_va[1].position = sf::Vector2f(tileSize.x, 0);
            current_va[2].position = sf::Vector2f(0, tileSize.y);
            current_va[3].position = sf::Vector2f(0, tileSize.y);
            current_va[4].position = sf::Vector2f(tileSize.x, 0);
            current_va[5].position = sf::Vector2f(tileSize.x, tileSize.y);

            current_va[0].texCoords = sf::Vector2f(tu * tileSize.x, tv * tileSize.y);
            current_va[1].texCoords = sf::Vector2f((tu + 1) * tileSize.x, tv * tileSize.y);
            current_va[2].texCoords = sf::Vector2f(tu * tileSize.x, (tv + 1) * tileSize.y);
            current_va[3].texCoords = sf::Vector2f(tu * tileSize.x, (tv + 1) * tileSize.y);
            current_va[4].texCoords = sf::Vector2f((tu + 1) * tileSize.x, tv * tileSize.y);
            current_va[5].texCoords = sf::Vector2f((tu + 1) * tileSize.x, (tv + 1) * tileSize.y);

            m_vertex_map[tile_idx] = current_va;
        }

        SPDLOG_INFO("MultiSprite() with {} tiles", m_tilemap_picks.size());
        for(auto [bin,va]: m_vertex_map) {
            SPDLOG_INFO("Tile #{} has {} vertices", bin, va.getVertexCount());
        }
        
    }
    // use the index from the `tilemap_picks` vector to select a loaded tile from the map
    void pick(unsigned int tile_key) 
    {
        m_selected_vertices = m_vertex_map[tile_key];
    }
private:

    void draw(sf::RenderTarget& target, sf::RenderStates states) const override
    {
        // apply the transform
        states.transform *= getTransform();

        // apply the tileset texture
        states.texture = &m_tilemap_texture;

        // draw the vertex array
        target.draw(m_selected_vertices, states);
    }
    std::vector<unsigned int> m_tilemap_picks;
    sf::VertexArray m_selected_vertices{};
    sf::Texture m_tilemap_texture;
    std::map<unsigned int, sf::VertexArray> m_vertex_map;

};

} // namespace ProceduralMaze::Sprites


#endif // __SPRITES_MULTISPRITE_HPP__