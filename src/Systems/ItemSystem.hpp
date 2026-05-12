#ifndef SRC_SYSTEMS_ITEMSYSTEM_HPP_
#define SRC_SYSTEMS_ITEMSYSTEM_HPP_

#include <Events/CreateItemEvent.hpp>
#include <Systems/BaseSystem.hpp>

namespace ProceduralMaze::Cmp
{
class Position;
} // namespace ProceduralMaze::Cmp

namespace ProceduralMaze::Sys
{

class ItemSystem : public BaseSystem
{
public:
  ItemSystem( entt::registry &reg, sf::RenderWindow &window, Sprites::SpriteFactory &sprite_factory, Audio::SoundBank &sound_bank );
  ~ItemSystem() {}

  void on_create_item_event( ProceduralMaze::Events::CreateItemEvent ev );

  void create_world_item( Cmp::Position pos, const std::string &item, std::string sfx, float zorder = 0.f );
  void create_seeing_stone( Cmp::Position pos, const std::string &item, float zorder );
  void create_explosive( Cmp::Position pos, const std::string &item, float zorder );

  //! @brief event handlers for pausing system clocks
  void on_pause() override {}
  //! @brief event handlers for resuming system clocks
  void on_resume() override {}
};

} // namespace ProceduralMaze::Sys

#endif // SRC_SYSTEMS_ITEMSYSTEM_HPP_