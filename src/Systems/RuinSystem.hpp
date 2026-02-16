#ifndef SRC_SYSTEMS_RUINSYSTEM_HPP_
#define SRC_SYSTEMS_RUINSYSTEM_HPP_

#include <Components/Ruin/RuinFloorAccess.hpp>
#include <Factory/MultiblockFactory.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <Systems/BaseSystem.hpp>

#include <Sprites/SpriteMetaType.hpp>
namespace ProceduralMaze::Sprites
{
class MultiSprite;
class SpriteFactory;
} // namespace ProceduralMaze::Sprites

namespace ProceduralMaze::Sys
{

class RuinSystem : public BaseSystem
{
public:
  RuinSystem( entt::registry &reg, sf::RenderWindow &window, Sprites::SpriteFactory &sprite_factory, Audio::SoundBank &sound_bank,
              entt::dispatcher &scenemanager_event_dispatcher );

  //! @brief Update the player system.
  void update();

  //! @brief event handlers for pausing system clocks
  void onPause() override {}
  //! @brief event handlers for resuming system clocks
  void onResume() override {}

  void spawn_floor_access( sf::Vector2f spawn_position, sf::Vector2f size, Cmp::RuinFloorAccess::Direction dir );

  template <typename COMPONENT>
  void add_stairs( sf::Vector2f spawn_position, const Sprites::MultiSprite &stairs_ms, float zorder = 0 )
  {
    auto stairs_entt = getReg().create();
    Cmp::Position stairs_pos( spawn_position, stairs_ms.getSpriteSizePixels() );
    getReg().emplace_or_replace<Cmp::Position>( stairs_entt, spawn_position, stairs_ms.getSpriteSizePixels() );
    Factory::createMultiblock<COMPONENT>( getReg(), stairs_entt, stairs_pos, stairs_ms );
    Factory::createMultiblockSegments<COMPONENT, Cmp::RuinStairsSegment>( getReg(), stairs_entt, stairs_pos, stairs_ms );

    for ( auto [stairs_entt, stairs_cmp, stairs_zorder] : getReg().view<COMPONENT, Cmp::ZOrderValue>().each() )
    {
      stairs_zorder.setZOrder( zorder );
    }
  }

  void check_floor_access_collision( Cmp::RuinFloorAccess::Direction direction );

  //! @brief Reset the floor access cooldown (call when entering a ruin scene)
  void reset_floor_access_cooldown()
  {
    m_floor_access_cooldown.restart();
    m_was_on_floor_access = true; // Assume we just landed on a floor access
  }

  void gen_lowerfloor_bookcases( sf::FloatRect scene_dimensions );
  void add_lowerfloor_cobwebs( sf::FloatRect scene_dimensions );
  void check_movement_slowdowns();

private:
  //! @brief Dispatcher reference for scene management events
  entt::dispatcher &m_scenemanager_event_dispatcher;

  //! @brief Cooldown clock to prevent immediate floor access re-triggering
  sf::Clock m_floor_access_cooldown{};
  static constexpr float kFloorAccessCooldownSeconds = 0.5f;

  //! @brief Track if player was on floor access last frame (requires leaving before re-triggering)
  bool m_was_on_floor_access{ false };
};

} // namespace ProceduralMaze::Sys

#endif // SRC_SYSTEMS_RUINSYSTEM_HPP_