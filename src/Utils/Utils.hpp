#ifndef SRC_UTILS_UTILS_HPP__
#define SRC_UTILS_UTILS_HPP__

#include <Components/Position.hpp>
#include <Components/RectBounds.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/System/Vector2.hpp>
#include <Utils/Constants.hpp>
#include <entt/entity/fwd.hpp>
#include <optional>
#include <utility>

// Forward declarations
namespace sf
{
class RenderWindow;
class View;
} // namespace sf

namespace ProceduralMaze::Cmp
{
class RectBounds;
class PlayerHealth;
class PlayerWealth;
class PlayerBlastRadius;
class PlayerMortality;
class PlayerLastGraveyardPosition;
class PlayerRuinLocation;
class ZOrderValue;
class System;
class LerpPosition;
} // namespace ProceduralMaze::Cmp

namespace ProceduralMaze::Sprites
{
using SpriteMetaType = std::string;
} // namespace ProceduralMaze::Sprites

namespace ProceduralMaze::Utils
{

enum class Rounding { TOWARDS_ZERO, AWAY_ZERO, NEAREST };

// Pure utility functions - no registry needed
bool isInBounds( const sf::Vector2f &position, const sf::Vector2f &size, const sf::Vector2u &map_grid_size );
sf::FloatRect snap_to_grid( const sf::FloatRect &position, Rounding rounding = Rounding::NEAREST ) noexcept;
sf::Vector2f snap_to_grid( const sf::Vector2f &position, Rounding rounding = Rounding::NEAREST ) noexcept;
sf::FloatRect get_mouse_bounds_in_gameview( const sf::RenderWindow &window, const sf::View &gameview );

// Entity/registry utility functions - declarations only
std::optional<sf::Vector2f> getPixelPosition( entt::registry &registry, entt::entity entity );
bool is_graveyard_exit_locked( entt::registry &reg );
entt::entity get_player_entity( entt::registry &reg );
Cmp::Position &get_player_position( entt::registry &reg );
int get_player_ruin_location( entt::registry &reg ); // Returns enum as int to avoid include
Cmp::PlayerLastGraveyardPosition *get_player_last_graveyard_position( entt::registry &reg );
Cmp::PlayerHealth &get_player_health( entt::registry &reg );
Cmp::PlayerWealth &get_player_wealth( entt::registry &reg );
Cmp::PlayerBlastRadius &get_player_blast_radius( entt::registry &reg );
Cmp::PlayerMortality &get_player_mortality( entt::registry &reg );
Cmp::ZOrderValue &get_player_zorder( entt::registry &reg );
float get_player_speed_penalty( entt::registry &reg );
void remove_player_lerp_cmp( entt::registry &reg );
Cmp::System &getSystemCmp( entt::registry &reg );
std::pair<entt::entity, Sprites::SpriteMetaType> get_player_inventory_type( entt::registry &reg );
float get_player_inventory_wear_level( entt::registry &reg );
void reduce_player_inventory_wear_level( entt::registry &reg, float amount );

//! @brief Get the Grid Position object
//! @param entity The entity to get the grid position for.
//! @tparam Signedness The signedness of the grid position coordinates (int or unsigned int).
//! @return std::optional<sf::Vector2<Signedness>>
template <typename Signedness>
static std::optional<sf::Vector2<Signedness>> getGridPosition( entt::registry &registry, entt::entity entity )
{
  auto pos = registry.try_get<Cmp::Position>( entity );
  if ( pos )
  {
    return std::optional<sf::Vector2<Signedness>>{ { static_cast<Signedness>( pos->position.x / Constants::kGridSizePx.x ),
                                                     static_cast<Signedness>( pos->position.y / Constants::kGridSizePx.y ) } };
  }
  else { SPDLOG_ERROR( "Entity {} does not have a Position component", static_cast<int>( entity ) ); }
  return std::nullopt;
}

} // namespace ProceduralMaze::Utils

#endif // SRC_UTILS_UTILS_HPP__