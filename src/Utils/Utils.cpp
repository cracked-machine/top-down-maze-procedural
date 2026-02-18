// #include <Systems/BaseSystem.hpp>

#include <Sprites/MultiSprite.hpp>
#include <Utils/Constants.hpp>
#include <Utils/Utils.hpp>

#include <Components/Exit.hpp>
#include <Components/Inventory/CarryItem.hpp>
#include <Components/Inventory/InventoryWearLevel.hpp>
#include <Components/LerpPosition.hpp>
#include <Components/Player/PlayerBlastRadius.hpp>
#include <Components/Player/PlayerCharacter.hpp>
#include <Components/Player/PlayerHealth.hpp>
#include <Components/Player/PlayerLastGraveyardPosition.hpp>
#include <Components/Player/PlayerMortality.hpp>
#include <Components/Player/PlayerRuinLocation.hpp>
#include <Components/Player/PlayerSpeedPenalty.hpp>
#include <Components/Player/PlayerWealth.hpp>
#include <Components/Position.hpp>
#include <Components/RectBounds.hpp>
#include <Components/System.hpp>
#include <Components/ZOrderValue.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/View.hpp>
#include <SFML/Window/Mouse.hpp>
#include <Sprites/MultiSprite.hpp>
#include <cmath>
#include <entt/entity/registry.hpp>
#include <spdlog/spdlog.h>
#include <stdexcept>

namespace ProceduralMaze::Utils
{

bool isInBounds( const sf::Vector2f &position, const sf::Vector2f &size, const sf::Vector2u &map_grid_size )
{
  float map_width = static_cast<float>( map_grid_size.x ) * Constants::kGridSizePxF.x;
  float map_height = static_cast<float>( map_grid_size.y ) * Constants::kGridSizePxF.y;

  if ( position.x < 0.f || position.y < 0.f ) return false;
  if ( position.x + size.x > map_width ) return false;
  if ( position.y + size.y > map_height ) return false;

  return true;
}

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
sf::FloatRect snap_to_grid( const sf::FloatRect &position, Rounding rounding ) noexcept
{
  float grid_size = Constants::kGridSizePx.x; // Assuming square grid
  sf::Vector2f snapped_pos;

  switch ( rounding )
  {
    case Rounding::TOWARDS_ZERO:
      snapped_pos = { std::trunc( position.position.x / grid_size ) * grid_size, std::trunc( position.position.y / grid_size ) * grid_size };
      break;
    case Rounding::AWAY_ZERO: {
      auto away_from_zero = []( float val, float grid )
      {
        float divided = val / grid;
        return ( val >= 0.f ? std::ceil( divided ) : std::floor( divided ) ) * grid;
      };
      snapped_pos = { away_from_zero( position.position.x, grid_size ), away_from_zero( position.position.y, grid_size ) };
      break;
    }
    case Rounding::NEAREST:
    default:
      snapped_pos = { std::round( position.position.x / grid_size ) * grid_size, std::round( position.position.y / grid_size ) * grid_size };
      break;
  }

  return sf::FloatRect( snapped_pos, position.size );
}

//! @brief Snap a given position to the nearest grid square.
//! This function takes a 2D position and rounds its coordinates to the nearest
//! grid square based on the grid size defined in BaseSystem::kGridSquareSizePixels.
//! It assumes the grid squares are of uniform size.
//!
//! @param position The position to snap, as an sf::Vector2f.
//! @return sf::Vector2f The snapped position aligned to the grid.
sf::Vector2f snap_to_grid( const sf::Vector2f &position, Rounding rounding ) noexcept
{
  float grid_size = Constants::kGridSizePx.x; // Assuming square grid

  switch ( rounding )
  {
    case Rounding::TOWARDS_ZERO:
      return { std::trunc( position.x / grid_size ) * grid_size, std::trunc( position.y / grid_size ) * grid_size };
    case Rounding::AWAY_ZERO: {
      auto away_from_zero = []( float val, float grid )
      {
        float divided = val / grid;
        return ( val >= 0.f ? std::ceil( divided ) : std::floor( divided ) ) * grid;
      };
      return { away_from_zero( position.x, grid_size ), away_from_zero( position.y, grid_size ) };
    }
    case Rounding::NEAREST:
    default:
      return { std::round( position.x / grid_size ) * grid_size, std::round( position.y / grid_size ) * grid_size };
  }
}

//! @brief Get the Pixel Position object from an entity's Position component.
//!
//! @param entity The entity to get the pixel position for.
//! @return std::optional<sf::Vector2f>
std::optional<sf::Vector2f> getPixelPosition( entt::registry &registry, entt::entity entity )
{
  auto pos = registry.try_get<Cmp::Position>( entity );
  if ( pos ) { return ( *pos ).position; }
  return std::nullopt;
}

bool is_graveyard_exit_locked( entt::registry &reg )
{
  auto exit_view = reg.view<Cmp::Exit>();
  for ( auto [exit_entt, exit_cmp] : exit_view.each() )
  {
    return exit_cmp.m_locked;
  }
  throw std::runtime_error( "No exit was found in the game!" );
}

Cmp::System &getSystemCmp( entt::registry &reg )
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

sf::FloatRect get_mouse_bounds_in_gameview( const sf::RenderWindow &window, const sf::View &gameview )
{
  sf::Vector2i mouse_pixel_pos = sf::Mouse::getPosition( window );
  sf::Vector2f mouse_world_pos = window.mapPixelToCoords( mouse_pixel_pos, gameview );

  // Check if the mouse position intersects with the entity's grave multiblock sprite
  auto mouse_position_bounds = sf::FloatRect( mouse_world_pos, sf::Vector2f( 2.f, 2.f ) );

  return mouse_position_bounds;
}

//! @brief Get the Grid Position object
//! @param entity The entity to get the grid position for.
//! @tparam Signedness The signedness of the grid position coordinates (int or unsigned int).
//! @return std::optional<sf::Vector2<Signedness>>
template <typename Signedness>

std::optional<sf::Vector2<Signedness>> getGridPosition( entt::registry &registry, entt::entity entity )
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

template std::optional<sf::Vector2<int>> getGridPosition( entt::registry &, entt::entity );
template std::optional<sf::Vector2<unsigned int>> getGridPosition( entt::registry &, entt::entity );

} // namespace ProceduralMaze::Utils
