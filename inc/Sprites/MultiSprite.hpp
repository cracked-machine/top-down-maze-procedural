#ifndef __SPRITES_MULTISPRITE2_HPP__
#define __SPRITES_MULTISPRITE2_HPP__

// #include <Components/Random.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/Transform.hpp>
#include <SFML/Graphics/Vertex.hpp>
#include <SFML/Graphics/VertexArray.hpp>
#include <SFML/System/Vector2.hpp>

#include <algorithm>
#include <cstdint>
#include <filesystem>

#include <spdlog/spdlog.h>
#include <string>
#include <vector>

namespace ProceduralMaze::Sprites {

class MultiSprite : public sf::Drawable, public sf::Transformable {
public:
  MultiSprite() = default;

  void add_sprite(const std::filesystem::path &tilemap_path,
                  std::vector<uint32_t> tilemap_picks,
                  const sf::Vector2u &tileSize = sf::Vector2u{16, 16},
                  std::string name = "") {
    if (!m_tilemap_texture.loadFromFile(tilemap_path)) {
      SPDLOG_CRITICAL("Unable to load tile map {}", tilemap_path.string());
      std::terminate();
    }

    SPDLOG_DEBUG("Processing multisprite for {}", name);

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
      current_va[1].texCoords =
          sf::Vector2f((tu + 1) * tileSize.x, tv * tileSize.y);
      current_va[2].texCoords =
          sf::Vector2f(tu * tileSize.x, (tv + 1) * tileSize.y);
      current_va[3].texCoords =
          sf::Vector2f(tu * tileSize.x, (tv + 1) * tileSize.y);
      current_va[4].texCoords =
          sf::Vector2f((tu + 1) * tileSize.x, tv * tileSize.y);
      current_va[5].texCoords =
          sf::Vector2f((tu + 1) * tileSize.x, (tv + 1) * tileSize.y);
      SPDLOG_TRACE("  - Added tile index {} (tu={},tv={})", tile_idx, tu, tv);

      m_va_list.push_back(current_va);
    }
    SPDLOG_DEBUG(
        "Requested {} tiles ... Created {} sprites from texture {}: ",
        std::distance(tilemap_picks.begin(),
                      std::unique(tilemap_picks.begin(), tilemap_picks.end())),
        m_va_list.size(), tilemap_path.string());
  }

  void pick(unsigned int idx, std::string caller) {
    if (m_va_list.empty()) {
      SPDLOG_WARN("pick() called on empty sprite list");
      return;
    }

    if (idx > m_va_list.size() - 1) {
      m_selected_vertices = m_va_list[0];
      SPDLOG_WARN("{}: pick() index {} out of range, selecting index 0 "
                  "instead. m_va_list size is {}",
                  caller, idx, m_va_list.size());
    } else {
      m_selected_vertices = m_va_list[idx];
    }
  }

  sf::Texture m_tilemap_texture;

private:
  void draw(sf::RenderTarget &target, sf::RenderStates states) const override {
    if (m_selected_vertices.getVertexCount() == 0) {
      SPDLOG_CRITICAL("No vertex array selected. Use pick() to select one.");
      std::terminate();
    }
    // apply the transform
    states.transform *= getTransform();

    // apply the tileset texture
    states.texture = &m_tilemap_texture;

    // draw the vertex array
    target.draw(m_selected_vertices, states);
  }

private:
  std::vector<sf::VertexArray> m_va_list;
  // select the first vertex array by default
  sf::VertexArray m_selected_vertices;
};

} // namespace ProceduralMaze::Sprites

#endif // __SPRITES_MULTISPRITE2_HPP__