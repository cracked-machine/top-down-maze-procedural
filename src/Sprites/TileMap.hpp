#ifndef __SPRITES_TILEMAP_HPP__
#define __SPRITES_TILEMAP_HPP__

#include <SFML/Graphics.hpp>
#include <SFML/System/Vector2.hpp>
#include <SceneControl/SceneData.hpp>
#include <entt/entity/fwd.hpp>

namespace ProceduralMaze::PathFinding
{
class SpatialHashGrid;
}

namespace ProceduralMaze::Sprites::Containers
{

class TileMap : public sf::Drawable, public sf::Transformable
{
public:
  TileMap() = default;

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

} // namespace ProceduralMaze::Sprites::Containers

#endif // __SPRITES_TILEMAP_HPP__