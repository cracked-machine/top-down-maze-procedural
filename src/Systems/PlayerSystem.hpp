#ifndef SRC_SYSTEMS_PLAYERSYSTEM_HPP__
#define SRC_SYSTEMS_PLAYERSYSTEM_HPP__

#include <Components/Persistent/EffectsVolume.hpp>
#include <Components/PlayerMortality.hpp>
#include <Components/Position.hpp>
#include <Systems/BaseSystem.hpp>

#include <SFML/Audio/Sound.hpp>
#include <SFML/Audio/SoundBuffer.hpp>
#include <entt/entity/registry.hpp>

namespace ProceduralMaze::Sys
{

class PlayerSystem : public BaseSystem
{
public:
  PlayerSystem( entt::registry &reg, sf::RenderWindow &window, Sprites::SpriteFactory &sprite_factory, Audio::SoundBank &sound_bank,
                entt::dispatcher &scenemanager_event_dispatcher );

  //! @brief Update the player system.
  void update( sf::Time globalDeltaTime, bool skip_collision_check = false );

  //! @brief Create and add the player entity to the registry
  void addPlayerEntity();

  void playFootstepsSound();
  void stopFootstepsSound();

  //! @brief event handlers for pausing system clocks
  void onPause() override {}
  //! @brief event handlers for resuming system clocks
  void onResume() override {}

private:
  //! @brief Rotation, scaling, offset, alpha, etc
  void localTransforms();

  //! @brief Position updates, lerping, etc
  void globalTranslations( sf::Time globalDeltaTime, bool skip_collision_check = false );

  //! @brief Refresh the PlayerDistance components for positions near the player
  void refreshPlayerDistances();

  //! @brief Check if the player is mortal or dead
  void checkPlayerMortality();

  //! @brief Use this to send events to the scene manager
  entt::dispatcher &m_scenemanager_event_dispatcher;

  sf::Clock m_debug_info_timer;
};

} // namespace ProceduralMaze::Sys

#endif // SRC_SYSTEMS_PLAYERSYSTEM_HPP__
