#ifndef SRC_FACTORY_FLOORMAPFACTORY_HPP__
#define SRC_FACTORY_FLOORMAPFACTORY_HPP__

#include <Sprites/TileMap.hpp>

namespace ProceduralMaze::Factory
{

class FloormapFactory
{
public:
  static void CreateFloormap( entt::registry &registry, Sprites::Containers::TileMap &floormap,
                              sf::Vector2u map_dimensions, std::string config_path )
  {
    floormap.load( registry, map_dimensions, config_path );
  }

  static void ClearFloormap( Sprites::Containers::TileMap &floormap ) { floormap.clear(); }
};

} // namespace ProceduralMaze::Factory

#endif // SRC_FACTORY_FLOORMAPFACTORY_HPP__