#ifndef SRC_SPRITES_TILEMAP_HPP_
#define SRC_SPRITES_TILEMAP_HPP_

#include <SceneControl/SmartPointers.hpp>

#include <SFML/System/Vector2.hpp>
#include <entt/entity/fwd.hpp>

namespace Game::PathFinding
{
class SpatialHashGrid;
}

namespace Game::Sprites::Containers
{

class VertexFloor : public sf::Drawable, public sf::Transformable
{
public:
  VertexFloor() = default;

  // Draw the sf::VertexArray to the render target (with optional state for shader)
  void draw( sf::RenderTarget &target, sf::RenderStates states ) const override;

  void clear() { m_vertices.clear(); }
  sf::Vector2u world_grid_offset{ 0, 0 };

  // Create the tile map using a single large sf::VertexArray
  void create( const PathFinding::SpatialHashGrid &void_sm, const Scene::SceneMapSharedPtr &scene_map );

  void remove( sf::Vector2f pos );

private:
  sf::VertexArray m_vertices;
  sf::Texture m_tileset;
  sf::Clock m_clock{};
};

} // namespace Game::Sprites::Containers

#endif // SRC_SPRITES_TILEMAP_HPP_