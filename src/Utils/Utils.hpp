#ifndef SRC_UTILS_UTILS_HPP__
#define SRC_UTILS_UTILS_HPP__

#include <Components/Exit.hpp>
#include <Components/Inventory/CarryItem.hpp>
#include <Components/InventoryWearLevel.hpp>
#include <Components/Persistent/DiggingDamagePerHit.hpp>
#include <Components/PlayableCharacter.hpp>
#include <Components/PlayerHealth.hpp>
#include <Components/PlayerMortality.hpp>
#include <Components/Position.hpp>
#include <Components/System.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/View.hpp>
// #include <Systems/BaseSystem.hpp>

#include <SFML/Window/Mouse.hpp>
#include <Sprites/MultiSprite.hpp>
#include <cmath>
#include <entt/entity/entity.hpp>
#include <entt/entity/fwd.hpp>
#include <entt/entity/registry.hpp>
#include <spdlog/spdlog.h>
#include <stdexcept>

namespace ProceduralMaze::Constants
{
inline constexpr sf::Vector2u kGridSquareSizePixels{ 16u, 16u };
inline static constexpr sf::Vector2f kGridSquareSizePixelsF{ 16.f, 16.f };
inline static constexpr sf::Vector2u kFallbackDisplaySize{ 1920, 1080 };
} // namespace ProceduralMaze::Constants

namespace ProceduralMaze::Utils
{

inline bool isInBounds( const sf::Vector2f &position, const sf::Vector2f &size, const sf::Vector2u &map_grid_size )
{
  float map_width = static_cast<float>( map_grid_size.x ) * Constants::kGridSquareSizePixelsF.x;
  float map_height = static_cast<float>( map_grid_size.y ) * Constants::kGridSquareSizePixelsF.y;

  if ( position.x < 0.f || position.y < 0.f ) return false;
  if ( position.x + size.x > map_width ) return false;
  if ( position.y + size.y > map_height ) return false;

  return true;
}

enum class Rounding { TOWARDS_ZERO, AWAY_ZERO };
//! @brief Snaps a rectangle's position to the nearest grid cell.
//!
//! Computes a new rectangle whose top-left corner is moved to the nearest grid
//! intersection based on BaseSystem::kGridSquareSizePixels.x (the function
//! assumes a square grid and uses the x component as the grid cell size).
//! The input rectangle's size is preserved.
//!
//! Rounding uses std::round, so coordinates are mapped to the nearest multiple
//! of the grid size (negative coordinates are handled correctly by std::round).
//!
//! @param position The rectangle to be snapped. Only its position is considered;
//!                 the rectangle's size remains unchanged.
//! @return A new sf::FloatRect with the snapped position and the original size.
//!
//! @note If the grid is not square, only the x component of
//!       BaseSystem::kGridSquareSizePixels is used for both axes.
inline constexpr sf::FloatRect snap_to_grid( const sf::FloatRect &position, Rounding rounding = Rounding::AWAY_ZERO ) noexcept
{

  float grid_size = Constants::kGridSquareSizePixels.x; // Assuming square grid
  if ( rounding == Rounding::TOWARDS_ZERO )
  {
    sf::Vector2f snapped_pos{ std::trunc( position.position.x / Constants::kGridSquareSizePixels.x ) * grid_size,
                              std::trunc( position.position.y / Constants::kGridSquareSizePixels.y ) * grid_size };
    return sf::FloatRect( snapped_pos, position.size );
  }
  else
  {
    sf::Vector2f snapped_pos{ std::round( position.position.x / Constants::kGridSquareSizePixels.x ) * grid_size,
                              std::round( position.position.y / Constants::kGridSquareSizePixels.y ) * grid_size };
    return sf::FloatRect( snapped_pos, position.size );
  }
}

//! @brief Snap a given position to the nearest grid square.
//! This function takes a 2D position and rounds its coordinates to the nearest
//! grid square based on the grid size defined in BaseSystem::kGridSquareSizePixels.
//! It assumes the grid squares are of uniform size.
//!
//! @param position The position to snap, as an sf::Vector2f.
//! @return sf::Vector2f The snapped position aligned to the grid.
inline constexpr sf::Vector2f snap_to_grid( const sf::Vector2f &position, Rounding rounding = Rounding::AWAY_ZERO ) noexcept
{
  float grid_size = Constants::kGridSquareSizePixels.x; // Assuming square grid
  if ( rounding == Rounding::TOWARDS_ZERO )
  {
    sf::Vector2f snapped_pos{ std::trunc( position.x / Constants::kGridSquareSizePixels.x ) * grid_size,
                              std::trunc( position.y / Constants::kGridSquareSizePixels.y ) * grid_size };
    return snapped_pos;
  }
  else
  {
    sf::Vector2f snapped_pos{ std::round( position.x / Constants::kGridSquareSizePixels.x ) * grid_size,
                              std::round( position.y / Constants::kGridSquareSizePixels.y ) * grid_size };
    return snapped_pos;
  }
}

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
    return std::optional<sf::Vector2<Signedness>>{ { static_cast<Signedness>( pos->position.x / Constants::kGridSquareSizePixels.x ),
                                                     static_cast<Signedness>( pos->position.y / Constants::kGridSquareSizePixels.y ) } };
  }
  else { SPDLOG_ERROR( "Entity {} does not have a Position component", static_cast<int>( entity ) ); }
  return std::nullopt;
}

//! @brief Get the Pixel Position object from an entity's Position component.
//!
//! @param entity The entity to get the pixel position for.
//! @return std::optional<sf::Vector2f>
static std::optional<sf::Vector2f> getPixelPosition( entt::registry &registry, entt::entity entity )
{
  auto pos = registry.try_get<Cmp::Position>( entity );
  if ( pos ) { return ( *pos ).position; }
  return std::nullopt;
}

static bool is_graveyard_exit_locked( entt::registry &reg )
{
  auto exit_view = reg.view<Cmp::Exit>();
  for ( auto [exit_entt, exit_cmp] : exit_view.each() )
  {
    return exit_cmp.m_locked;
  }
  throw std::runtime_error( "No exit was found in the game!" );
}

static entt::entity get_player_entity( entt::registry &reg )
{
  auto player_view = reg.view<Cmp::PlayableCharacter, Cmp::Position>();
  if ( player_view.front() == entt::null ) throw std::runtime_error( "Player entity could not be found" );
  return player_view.front();
}

static Cmp::Position &get_player_position( entt::registry &reg )
{
  auto player_view = reg.view<Cmp::PlayableCharacter, Cmp::Position>();
  return player_view.get<Cmp::Position>( get_player_entity( reg ) );
}

static Cmp::PlayerHealth &get_player_health( entt::registry &reg )
{
  auto player_view = reg.view<Cmp::PlayerHealth>();
  return player_view.get<Cmp::PlayerHealth>( get_player_entity( reg ) );
}

static Cmp::PlayerMortality &get_player_mortality( entt::registry &reg )
{
  auto player_view = reg.view<Cmp::PlayerMortality>();
  return player_view.get<Cmp::PlayerMortality>( get_player_entity( reg ) );
}

static Cmp::System &getSystemCmp( entt::registry &reg )
{
  entt::entity system_entt = entt::null;
  Cmp::System *system_cmp = nullptr;
  auto system_view = reg.view<Cmp::System>();
  if ( system_view->empty() ) { throw std::runtime_error( "Unable to get component Cmp::System!" ); }
  system_entt = system_view.front();
  if ( system_entt == entt::null ) { throw std::runtime_error( "Unable to get entity for Cmp::System!" ); }
  system_cmp = reg.try_get<Cmp::System>( system_entt );
  return *system_cmp;
}

static std::pair<entt::entity, Sprites::SpriteMetaType> get_player_inventory_type( entt::registry &reg )
{
  auto inv_view = reg.view<Cmp::PlayerInventorySlot>();
  Sprites::SpriteMetaType found_type = "";
  entt::entity found_entt = entt::null;
  // this assumes there is only one slot in the inventory, so warn if there is a bug somewhere
  if ( inv_view.size() > 1 ) SPDLOG_WARN( "Found multiple slots in signle slot inventory" );
  for ( auto [inv_entt, inv_cmp] : inv_view.each() )
  {
    found_type = inv_cmp.type;
  }
  return { found_entt, found_type };
}

static float get_player_inventory_wear_level( entt::registry &reg )
{
  auto inventory_wear_view = reg.view<Cmp::PlayerInventorySlot, Cmp::InventoryWearLevel>();
  for ( auto [inventory_entity, inventory_slot, wear_level] : inventory_wear_view.each() )
  {
    return wear_level.m_level;
  }
  SPDLOG_DEBUG( "Player Inventory slot has no appropriate InventoryWearLevel component" );
  return -1;
}

static void reduce_player_inventory_wear_level( entt::registry &reg, float amount )
{
  auto inventory_wear_view = reg.view<Cmp::PlayerInventorySlot, Cmp::InventoryWearLevel>();
  for ( auto [inventory_entity, inventory_slot, wear_level] : inventory_wear_view.each() )
  {
    wear_level.m_level -= amount;
  }
  SPDLOG_DEBUG( "Player Inventory slot has no appropriate InventoryWearLevel component" );
}

static sf::FloatRect get_mouse_bounds_in_gameview( const sf::RenderWindow &window, const sf::View &gameview )
{
  sf::Vector2i mouse_pixel_pos = sf::Mouse::getPosition( window );
  sf::Vector2f mouse_world_pos = window.mapPixelToCoords( mouse_pixel_pos, gameview );

  // Check if the mouse position intersects with the entity's grave multiblock sprite
  auto mouse_position_bounds = sf::FloatRect( mouse_world_pos, sf::Vector2f( 2.f, 2.f ) );

  return mouse_position_bounds;
}

static uint8_t to_percent( float max_value, uint8_t convert )
{
  auto converted = std::round( ( max_value / 100 ) * convert );
  SPDLOG_DEBUG( "Converted {} (max: {}) to {}%", convert, max_value, converted );
  return converted;
}

} // namespace ProceduralMaze::Utils

#endif // SRC_UTILS_UTILS_HPP__