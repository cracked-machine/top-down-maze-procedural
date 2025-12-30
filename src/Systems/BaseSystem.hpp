#ifndef SRC_SYSTEMS_BASESYSTEM_HPP__
#define SRC_SYSTEMS_BASESYSTEM_HPP__

#include <Audio/SoundBank.hpp>
#include <_mingw_stat64.h>
#include <entt/entity/fwd.hpp>
#include <entt/signal/dispatcher.hpp>

#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Vector2.hpp>

#include <Sprites/SpriteFactory.hpp>

namespace ProceduralMaze
{
// using SharedEnttRegistry = std::shared_ptr<entt::basic_registry<entt::entity>>;

namespace Sys
{

class BaseSystem
{
public:
  //! @brief Construct a new Base System object
  BaseSystem( entt::registry &reg, sf::RenderWindow &window, Sprites::SpriteFactory &sprite_factory, Audio::SoundBank &sound_bank );

  //! @brief polymorphic destructor for derived classes
  virtual ~BaseSystem() { SPDLOG_INFO( "BaseSystem destructor called for system at {}", static_cast<void *>( this ) ); };

  //! @brief Event handler for pausing system clocks. Must be implemented by derived classes.
  //! @note If you register this handler with the event dispatcher, this function is automcatically
  //! called when the game is paused.
  /// For example:
  /// `std::ignore =
  /// getEventDispatcher().sink<Events::PauseClocksEvent>().connect<&Sys::DerivedSystem::onPause>(this);`
  virtual void onPause() = 0;

  //! @brief Event handler for resuming system clocks. Must be implemented by derived classes.
  //! @note If you register this handler with the event dispatcher, this function is automcatically
  //! called when the game is resumed.
  /// For example:
  /// `std::ignore =
  /// getEventDispatcher().sink<Events::ResumeClocksEvent>().connect<&Sys::DerivedSystem::onResume>(this);`
  virtual void onResume() = 0;

  //! @brief Checks if the player's movement to a given position is valid
  //! Validates whether the player can move to the specified position by checking
  //! for collisions with walls, boundaries, or other obstacles in the game world.
  //! @param player_position The target position to validate for player movement
  //! @return true if the movement is valid and allowed, false otherwise
  bool is_valid_move( const sf::FloatRect &player_position );

  //! @brief Checks if a diagonal movement would pass between two obstacles.
  //! This function determines whether a diagonal movement from the current position
  //! in the specified direction would result in the player squeezing between two
  //! obstacles (e.g., moving diagonally through a corner where two walls meet).
  //! This is typically used to prevent unrealistic movement through tight spaces.
  //!
  //! @param current_pos The current position of the player as a 2D vector
  //! @param direction The direction vector representing the intended diagonal movement
  //! @return true if the diagonal movement would pass between obstacles, false otherwise
  bool isDiagonalMovementBetweenObstacles( const sf::FloatRect &current_pos, const sf::Vector2f &direction );

  // singleton event dispatcher
  // Use this to get temporary access to the dispatcher to register event handlers
  static entt::dispatcher &get_systems_event_queue()
  {
    if ( !m_systems_event_queue ) { m_systems_event_queue = std::make_unique<entt::dispatcher>(); }
    return *m_systems_event_queue;
  }

  entt::registry &getReg() { return m_reg.get(); }
  const entt::registry &getReg() const { return m_reg.get(); }
  void setReg( entt::registry &reg ) { m_reg = std::ref( reg ); }

protected:
  // Entity registry: non-owning, re-assignable reference (by SceneManager)
  // The registry is owned by the current Scene.
  std::reference_wrapper<entt::registry> m_reg;

  //! @brief Non-owning reference to the OpenGL window
  sf::RenderWindow &m_window;

  //! @brief Non-owning reference to the shared sprite factory
  Sprites::SpriteFactory &m_sprite_factory;

  //! @brief Non-owning reference to the shared sound effects system
  Audio::SoundBank &m_sound_bank;

private:
  // Prevent access to uninitialised dispatcher - use getEventDispatcher()
  static std::unique_ptr<entt::dispatcher> m_systems_event_queue;
};

} // namespace Sys
} // namespace ProceduralMaze

#endif // SRC_SYSTEMS_BASESYSTEM_HPP__