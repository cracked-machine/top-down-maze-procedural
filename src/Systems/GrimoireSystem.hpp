#ifndef SRC_SYSTEMS_GRIMOIRESYSTEM_HPP
#define SRC_SYSTEMS_GRIMOIRESYSTEM_HPP

#include <Events/PlayerActionEvent.hpp>
#include <Systems/BaseSystem.hpp>

namespace Game::Sys
{

class GrimoireSystem : public BaseSystem
{
public:
  GrimoireSystem( entt::registry &reg, sf::RenderWindow &window, Sprites::SpriteFactory &sprite_factory, Audio::SoundBank &sound_bank );

  void update( sf::Time dt );

  //! @brief event handlers for pausing system clocks
  void on_pause() override {}
  //! @brief event handlers for resuming system clocks
  void on_resume() override {}

  void enable_grimoire_overlay();
  void disable_grimoire_overlay();
  [[nodiscard]] bool is_grimoire_overlay_open() const { return m_grimoire_overlay_open; }

private:
  void on_player_action( Events::PlayerActionEvent ev );
  bool m_grimoire_overlay_open{ false };
  sf::Time overlay_debounce;
};

} // namespace Game::Sys

#endif // SRC_SYSTEMS_GRIMOIRESYSTEM_HPP