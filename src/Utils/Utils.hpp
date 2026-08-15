#ifndef SRC_UTILS_UTILS_HPP__
#define SRC_UTILS_UTILS_HPP__

#include <Sprites/SpriteMetaType.hpp>
#include <Utils/Constants.hpp>

#include <cstddef>
#include <entt/entity/fwd.hpp>
#include <optional>
#include <string>

// Forward declarations
namespace sf
{
class RenderWindow;
class View;
} // namespace sf

namespace Game::Cmp
{
class Position;
class RectBounds;
class PlayerHealth;
class ZOrderValue;
class System;
class LerpPosition;
class CurrentScene;
} // namespace Game::Cmp

namespace Game::Cmp::Player
{
class Wealth;
class BlastRadius;
class Mortality;
class LastGraveyardPosition;
class RuinLocation;
} // namespace Game::Cmp::Player

namespace Game::Utils
{

//! @brief
enum class Rounding { TOWARDS_ZERO, AWAY_ZERO, NEAREST };

// Pure utility functions - no registry needed
bool is_in_bounds( const sf::Vector2f &position, const sf::Vector2f &size, const sf::Vector2u &map_grid_size );
sf::FloatRect snap_to_grid( const sf::FloatRect &position, Rounding rounding = Rounding::NEAREST ) noexcept;
sf::Vector2f snap_to_grid( const sf::Vector2f &position, Rounding rounding = Rounding::NEAREST ) noexcept;
sf::FloatRect snap_to_grid( const Cmp::Position &position, Rounding rounding ) noexcept;
sf::FloatRect get_mouse_bounds_in_gameview( const sf::RenderWindow &window, const sf::View &gameview );
entt::entity get_world_pos_entt( entt::registry &reg, Cmp::Position match );

//! @brief Word-wraps text to fit within max_chars_per_line, preserving existing newlines as hard breaks.
//! @note Approximate (assumes roughly-uniform character width) - fine for a fatal-error debug screen.
std::string wrap_text_to_width( const std::string &text, std::size_t max_chars_per_line );

// Entity/registry utility functions - declarations only
std::optional<sf::Vector2f> get_pixel_position( entt::registry &registry, entt::entity entity );
bool is_graveyard_exit_locked( entt::registry &reg );

//! @brief Get the scene setting cmp object
//! @tparam SCENESETTING
//! @param reg
//! @return SCENESETTING&
template <typename SCENESETTING>
SCENESETTING &scene_setting( entt::registry &reg )
{
  entt::entity scene_entt = entt::null;
  SCENESETTING *scene_setting_cmp = nullptr;
  auto scene_setting_view = reg.view<SCENESETTING>();
  if ( scene_setting_view->empty() ) { throw std::runtime_error( "Unable to get scene setting component!" ); }
  scene_entt = scene_setting_view.front();
  if ( scene_entt == entt::null ) { throw std::runtime_error( "Unable to get entity for scene setting component!" ); }
  scene_setting_cmp = reg.try_get<SCENESETTING>( scene_entt );
  return *scene_setting_cmp;
}

//! @brief Get the Grid Position object
//! @param entity The entity to get the grid position for.
//! @tparam Signedness The signedness of the grid position coordinates (int or unsigned int).
//! @return std::optional<sf::Vector2<Signedness>>
template <typename Signedness>
std::optional<sf::Vector2<Signedness>> get_grid_position( entt::registry &registry, entt::entity entity );

extern template std::optional<sf::Vector2<int>> get_grid_position( entt::registry &, entt::entity );
extern template std::optional<sf::Vector2<unsigned int>> get_grid_position( entt::registry &, entt::entity );

} // namespace Game::Utils

#endif // SRC_UTILS_UTILS_HPP__
