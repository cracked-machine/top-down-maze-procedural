#ifndef _SYS_DIGGING_SYSTEM_HPP__
#define _SYS_DIGGING_SYSTEM_HPP__

#include <BaseSystem.hpp>
#include <Events/PlayerActionEvent.hpp>
#include <SFML/System/Clock.hpp>
namespace ProceduralMaze::Sys {

class DiggingSystem : public BaseSystem
{
public:
  DiggingSystem( ProceduralMaze::SharedEnttRegistry reg );

  void on_player_action( const Events::PlayerActionEvent &event );

private:
  void check_player_dig_obstacle_collision();
  sf::Clock m_dig_cooldown_clock;
};
} // namespace ProceduralMaze::Sys

#endif // __SYSTEMS_DIGGING_SYSTEM_HPP__