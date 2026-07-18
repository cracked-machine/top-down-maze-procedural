#include <Factory/MultiblockFactory.hpp>

// Include all the actual component definitions
#include <Components/Altar/AltarMultiBlock.hpp>
#include <Components/Altar/AltarSegment.hpp>
#include <Components/Crypt/CryptBuildingMultiBlock.hpp>
#include <Components/Crypt/CryptBuildingSegment.hpp>
#include <Components/Crypt/CryptEntrance.hpp>
#include <Components/Crypt/CryptInteriorMultiBlock.hpp>
#include <Components/Crypt/CryptInteriorSegment.hpp>
#include <Components/Crypt/CryptObjectiveMultiBlock.hpp>
#include <Components/Crypt/CryptObjectiveSegment.hpp>
#include <Components/Exit.hpp>
#include <Components/Grave/GraveExitMultiBlock.hpp>
#include <Components/Grave/GraveExitSegment.hpp>
#include <Components/Grave/GraveMultiBlock.hpp>
#include <Components/Grave/GraveSegment.hpp>
#include <Components/Grave/PlantMultiBlock.hpp>
#include <Components/Grave/PlantSegment.hpp>
#include <Components/Ruin/RuinBuildingMultiBlock.hpp>
#include <Components/Ruin/RuinBuildingSegment.hpp>
#include <Components/Ruin/RuinEntrance.hpp>
#include <Components/Ruin/RuinGateSegment.hpp>
#include <Components/Ruin/RuinHexagramMultiBlock.hpp>
#include <Components/Ruin/RuinHexagramSegment.hpp>
#include <Components/Ruin/RuinStairsBalustradeMultiBlock.hpp>
#include <Components/Ruin/RuinStairsGateMultiBlock.hpp>
#include <Components/Ruin/RuinStairsLowerMultiBlock.hpp>
#include <Components/Ruin/RuinStairsSegment.hpp>
#include <Components/Ruin/RuinStairsUpperMultiBlock.hpp>
#include <Components/Spring/HealingSpringBuildingMultiBlock.hpp>
#include <Components/Spring/HealingSpringBuildingSegment.hpp>
#include <Components/Spring/HealingSpringEntrance.hpp>
#include <Components/Spring/HealingSpringMultiBlock.hpp>
#include <Components/Spring/HealingSpringSegment.hpp>
#include <PathFinding/SpatialHashGrid.hpp>
#include <Sprites/SpriteSheet.hpp>
#include <Utils/Constants.hpp>

namespace Game::Factory
{

namespace detail
{

template <typename MULTIBLOCK>
  requires IsMB<MULTIBLOCK>
void create_multiblock( entt::registry &reg, entt::entity entity, const Cmp::UUID &uuid, Cmp::Position pos, const Sprites::SpriteSheet &ss,
                        size_t ss_index )
{
  reg.emplace_or_replace<MULTIBLOCK>( entity, pos.position, ss.get_px_size() );
  // clang-format off
  reg.emplace_or_replace<Cmp::AnimData>( entity, Cmp::AnimData::Config{
        .sprite_type = ss.get_sprite_type(),
        .frame_index_offset = ss_index,
        .enabled = true
  });
  // clang-format on
  reg.emplace_or_replace<Cmp::ZOrderValue>( entity, pos.position.y );
  reg.emplace_or_replace<Cmp::ReservedPosition>( entity );
  reg.emplace_or_replace<Cmp::UUID>( entity, uuid );
  reg.emplace_or_replace<Cmp::Position>( entity, pos.position, ss.get_px_size() );

  [[maybe_unused]] auto zorder_cmp = reg.get<Cmp::ZOrderValue>( entity );
  // clang-format off
  SPDLOG_DEBUG( "Placed {} at position ({}, {}). Grid size: {}x{} at z-order {} - {}",
    ss.get_sprite_type(),
    pos.position.x,
    pos.position.y,
    ss.get_grid_size().x,
    ss.get_grid_size().y,
    zorder_cmp.getZOrder(),
    uuid.str()
  );
  // clang-format on
}

template <typename MULTIBLOCK, typename MBSEGMENT>
  requires IsMB<MULTIBLOCK> && IsMBSegment<MBSEGMENT>
void update_segments( entt::registry &reg, const Sprites::SpriteSheet &ss, [[maybe_unused]] entt::entity mb_entt, MULTIBLOCK mb_cmp )
{
  auto solid_masks = ss.get_solid_mask();

  for ( auto [entity, segment_cmp, pos_cmp] : reg.view<MBSEGMENT, Cmp::Position>().each() )
  {
    if ( not pos_cmp.findIntersection( mb_cmp ) ) continue;

    float rel_x = pos_cmp.position.x - mb_cmp.position.x;
    float rel_y = pos_cmp.position.y - mb_cmp.position.y;

    int rel_grid_x = static_cast<int>( rel_x / Constants::kGridSizePx.x );
    int rel_grid_y = static_cast<int>( rel_y / Constants::kGridSizePx.y );

    std::size_t calculated_grid_index = ( rel_grid_y * ss.get_grid_size().x ) + rel_grid_x;

    bool new_solid_mask = true;
    if ( not solid_masks.empty() && solid_masks.size() > calculated_grid_index ) { new_solid_mask = solid_masks.at( calculated_grid_index ); }

    segment_cmp.set_solid_mask( new_solid_mask );
    reg.emplace_or_replace<Cmp::ZOrderValue>( entity, pos_cmp.position.y + ss.get_zorder( calculated_grid_index ) );

    if ( new_solid_mask )
    {
      reg.emplace_or_replace<Cmp::NpcNoPathFinding>( entity );
      reg.emplace_or_replace<Cmp::PlayerNoPath>( entity );
    }
    else
    {
      reg.remove<Cmp::NpcNoPathFinding>( entity );
      reg.remove<Cmp::PlayerNoPath>( entity );
    }
  }
}

template <typename MULTIBLOCK, typename MBSEGMENT>
  requires IsMB<MULTIBLOCK> && IsMBSegment<MBSEGMENT>
std::vector<entt::entity> create_multiblock_segments( entt::registry &reg, entt::entity multiblock_entity, const Cmp::UUID &uuid,
                                                      Cmp::Position mb_pos_cmp, const Sprites::SpriteSheet &ss )
{

  MULTIBLOCK new_multiblock_bounds = reg.get<MULTIBLOCK>( multiblock_entity );
  std::size_t door_grid_index = static_cast<std::size_t>( ( ss.get_door_position().y * ss.get_grid_size().x ) + ss.get_door_position().x );

  // track which grid positions are already assigned segments
  PathFinding::SpatialHashGrid segment_map;

  // cache upfront so we dont add mutate the view whilst iterating it.
  std::vector<entt::entity> world_pos_entt_list;
  for ( auto [pos_entity, pos_cmp] : reg.view<Cmp::Position>().each() )
  {
    if ( not pos_cmp.findIntersection( new_multiblock_bounds ) ) continue;
    if ( reg.any_of<MULTIBLOCK, MBSEGMENT, Cmp::ReservedPosition>( pos_entity ) ) continue;
    world_pos_entt_list.push_back( pos_entity );
  }

  std::vector<entt::entity> created_entts;
  for ( auto world_pos_entity : world_pos_entt_list )
  {

    // mark this world entt as reserved to prevent repeat visits
    reg.emplace_or_replace<Cmp::ReservedPosition>( world_pos_entity );

    // Calculate relative pixel positions within the large obstacle grid
    auto pos_cmp = reg.get<Cmp::Position>( world_pos_entity );
    float rel_x = pos_cmp.position.x - mb_pos_cmp.position.x;
    float rel_y = pos_cmp.position.y - mb_pos_cmp.position.y;

    // Convert to relative grid coordinates
    int rel_grid_x = static_cast<int>( rel_x / Constants::kGridSizePx.x );
    int rel_grid_y = static_cast<int>( rel_y / Constants::kGridSizePx.y );

    std::size_t calculated_grid_index = ( rel_grid_y * ss.get_grid_size().x ) + rel_grid_x;
    SPDLOG_DEBUG( "  - Creating segment at ({}, {}) with sprite_index {}", pos_cmp.position.x, pos_cmp.position.y, calculated_grid_index );

    // Don't allow duplicate segments at a given position
    if ( not segment_map.at( pos_cmp ).empty() ) continue;

    auto new_segment_entt = reg.create();
    reg.emplace_or_replace<MBSEGMENT>( new_segment_entt, true );
    reg.emplace_or_replace<Cmp::Armable>( new_segment_entt );
    reg.emplace_or_replace<Cmp::UUID>( new_segment_entt, uuid );
    reg.emplace_or_replace<Cmp::Position>( new_segment_entt, pos_cmp.position, pos_cmp.size );
    reg.emplace_or_replace<Cmp::ReservedPosition>( new_segment_entt );

    if constexpr ( std::is_same_v<MULTIBLOCK, Cmp::CryptBuildingMultiBlock> )
    {
      if ( calculated_grid_index == door_grid_index ) { reg.emplace_or_replace<Cmp::CryptEntrance>( new_segment_entt ); }
    }
    else if constexpr ( std::is_same_v<MULTIBLOCK, Cmp::HealingSpringBuildingMultiBlock> )
    {
      if ( calculated_grid_index == door_grid_index ) { reg.emplace_or_replace<Cmp::HealingSpringEntrance>( new_segment_entt ); }
    }
    else if constexpr ( std::is_same_v<MULTIBLOCK, Cmp::RuinBuildingMultiBlock> )
    {
      if ( calculated_grid_index == door_grid_index ) { reg.emplace_or_replace<Cmp::RuinEntrance>( new_segment_entt ); }
    }
    else if constexpr ( std::is_same_v<MULTIBLOCK, Cmp::GraveExitMultiBlock> )
    {
      if ( calculated_grid_index == door_grid_index ) { reg.emplace_or_replace<Cmp::Exit>( new_segment_entt ); }
    }

    created_entts.push_back( new_segment_entt );
    segment_map.insert( new_segment_entt, pos_cmp );
  }

  update_segments<MULTIBLOCK, MBSEGMENT>( reg, ss, multiblock_entity, new_multiblock_bounds );

  return created_entts;
}

// clang-format off
template void create_multiblock<Cmp::PlantMultiBlock>( entt::registry &, entt::entity, const Cmp::UUID &, Cmp::Position, const Sprites::SpriteSheet &, size_t );
template void create_multiblock<Cmp::GraveExitMultiBlock>( entt::registry &, entt::entity, const Cmp::UUID &, Cmp::Position, const Sprites::SpriteSheet &, size_t );
template void create_multiblock<Cmp::HealingSpringMultiBlock>( entt::registry &, entt::entity, const Cmp::UUID &, Cmp::Position, const Sprites::SpriteSheet &, size_t );
template void create_multiblock<Cmp::CryptObjectiveMultiBlock>( entt::registry &, entt::entity, const Cmp::UUID &, Cmp::Position, const Sprites::SpriteSheet &, size_t );
template void create_multiblock<Cmp::RuinStairsLowerMultiBlock>( entt::registry &, entt::entity, const Cmp::UUID &, Cmp::Position, const Sprites::SpriteSheet &, size_t );
template void create_multiblock<Cmp::RuinStairsUpperMultiBlock>( entt::registry &, entt::entity, const Cmp::UUID &, Cmp::Position, const Sprites::SpriteSheet &, size_t );
template void create_multiblock<Cmp::RuinStairsBalustradeMultiBlock>( entt::registry &, entt::entity, const Cmp::UUID &, Cmp::Position, const Sprites::SpriteSheet &, size_t );
template void create_multiblock<Cmp::RuinStairsGateMultiBlock>( entt::registry &, entt::entity, const Cmp::UUID &, Cmp::Position, const Sprites::SpriteSheet &, size_t );
template void create_multiblock<Cmp::RuinHexagramMultiBlock>( entt::registry &, entt::entity, const Cmp::UUID &, Cmp::Position, const Sprites::SpriteSheet &, size_t );
template void create_multiblock<Cmp::AltarMultiBlock>( entt::registry &, entt::entity, const Cmp::UUID &, Cmp::Position, const Sprites::SpriteSheet &, size_t );
template void create_multiblock<Cmp::GraveMultiBlock>( entt::registry &, entt::entity, const Cmp::UUID &, Cmp::Position, const Sprites::SpriteSheet &, size_t );
template void create_multiblock<Cmp::CryptBuildingMultiBlock>( entt::registry &, entt::entity, const Cmp::UUID &, Cmp::Position, const Sprites::SpriteSheet &, size_t );
template void create_multiblock<Cmp::HealingSpringBuildingMultiBlock>( entt::registry &, entt::entity, const Cmp::UUID &, Cmp::Position, const Sprites::SpriteSheet &, size_t );
template void create_multiblock<Cmp::RuinBuildingMultiBlock>( entt::registry &, entt::entity, const Cmp::UUID &, Cmp::Position, const Sprites::SpriteSheet &, size_t );

template void update_segments<Cmp::PlantMultiBlock, Cmp::PlantSegment>( entt::registry &, const Sprites::SpriteSheet &, entt::entity, Cmp::PlantMultiBlock );
template void update_segments<Cmp::GraveExitMultiBlock, Cmp::GraveExitSegment>( entt::registry &, const Sprites::SpriteSheet &, entt::entity, Cmp::GraveExitMultiBlock );
template void update_segments<Cmp::HealingSpringMultiBlock, Cmp::HealingSpringSegment>( entt::registry &, const Sprites::SpriteSheet &, entt::entity, Cmp::HealingSpringMultiBlock );
template void update_segments<Cmp::CryptObjectiveMultiBlock, Cmp::CryptObjectiveSegment>( entt::registry &, const Sprites::SpriteSheet &, entt::entity, Cmp::CryptObjectiveMultiBlock );
template void update_segments<Cmp::RuinStairsLowerMultiBlock, Cmp::RuinStairsSegment>( entt::registry &, const Sprites::SpriteSheet &, entt::entity, Cmp::RuinStairsLowerMultiBlock );
template void update_segments<Cmp::RuinStairsUpperMultiBlock, Cmp::RuinStairsSegment>( entt::registry &, const Sprites::SpriteSheet &, entt::entity, Cmp::RuinStairsUpperMultiBlock );
template void update_segments<Cmp::RuinStairsBalustradeMultiBlock, Cmp::RuinStairsSegment>( entt::registry &, const Sprites::SpriteSheet &, entt::entity, Cmp::RuinStairsBalustradeMultiBlock );
template void update_segments<Cmp::RuinStairsGateMultiBlock, Cmp::RuinGateSegment>( entt::registry &, const Sprites::SpriteSheet &, entt::entity, Cmp::RuinStairsGateMultiBlock );
template void update_segments<Cmp::RuinHexagramMultiBlock, Cmp::RuinHexagramSegment>( entt::registry &, const Sprites::SpriteSheet &, entt::entity, Cmp::RuinHexagramMultiBlock );
template void update_segments<Cmp::AltarMultiBlock, Cmp::AltarSegment>( entt::registry &, const Sprites::SpriteSheet &, entt::entity, Cmp::AltarMultiBlock );
template void update_segments<Cmp::GraveMultiBlock, Cmp::GraveSegment>( entt::registry &, const Sprites::SpriteSheet &, entt::entity, Cmp::GraveMultiBlock );
template void update_segments<Cmp::CryptBuildingMultiBlock, Cmp::CryptBuildingSegment>( entt::registry &, const Sprites::SpriteSheet &, entt::entity, Cmp::CryptBuildingMultiBlock );
template void update_segments<Cmp::HealingSpringBuildingMultiBlock, Cmp::HealingSpringBuildingSegment>( entt::registry &, const Sprites::SpriteSheet &, entt::entity, Cmp::HealingSpringBuildingMultiBlock );
template void update_segments<Cmp::RuinBuildingMultiBlock, Cmp::RuinBuildingSegment>( entt::registry &, const Sprites::SpriteSheet &, entt::entity, Cmp::RuinBuildingMultiBlock );

template std::vector<entt::entity> create_multiblock_segments<Cmp::PlantMultiBlock, Cmp::PlantSegment>( entt::registry &, entt::entity, const Cmp::UUID &, Cmp::Position, const Sprites::SpriteSheet & );
template std::vector<entt::entity> create_multiblock_segments<Cmp::GraveExitMultiBlock, Cmp::GraveExitSegment>( entt::registry &, entt::entity, const Cmp::UUID &, Cmp::Position, const Sprites::SpriteSheet & );
template std::vector<entt::entity> create_multiblock_segments<Cmp::HealingSpringMultiBlock, Cmp::HealingSpringSegment>( entt::registry &, entt::entity, const Cmp::UUID &, Cmp::Position, const Sprites::SpriteSheet & );
template std::vector<entt::entity> create_multiblock_segments<Cmp::CryptObjectiveMultiBlock, Cmp::CryptObjectiveSegment>( entt::registry &, entt::entity, const Cmp::UUID &, Cmp::Position, const Sprites::SpriteSheet & );
template std::vector<entt::entity> create_multiblock_segments<Cmp::RuinStairsLowerMultiBlock, Cmp::RuinStairsSegment>( entt::registry &, entt::entity, const Cmp::UUID &, Cmp::Position, const Sprites::SpriteSheet & );
template std::vector<entt::entity> create_multiblock_segments<Cmp::RuinStairsUpperMultiBlock, Cmp::RuinStairsSegment>( entt::registry &, entt::entity, const Cmp::UUID &, Cmp::Position, const Sprites::SpriteSheet & );
template std::vector<entt::entity> create_multiblock_segments<Cmp::RuinStairsBalustradeMultiBlock, Cmp::RuinStairsSegment>( entt::registry &, entt::entity, const Cmp::UUID &, Cmp::Position, const Sprites::SpriteSheet & );
template std::vector<entt::entity> create_multiblock_segments<Cmp::RuinStairsGateMultiBlock, Cmp::RuinGateSegment>( entt::registry &, entt::entity, const Cmp::UUID &, Cmp::Position, const Sprites::SpriteSheet & );
template std::vector<entt::entity> create_multiblock_segments<Cmp::RuinHexagramMultiBlock, Cmp::RuinHexagramSegment>( entt::registry &, entt::entity, const Cmp::UUID &, Cmp::Position, const Sprites::SpriteSheet & );
template std::vector<entt::entity> create_multiblock_segments<Cmp::AltarMultiBlock, Cmp::AltarSegment>( entt::registry &, entt::entity, const Cmp::UUID &, Cmp::Position, const Sprites::SpriteSheet & );
template std::vector<entt::entity> create_multiblock_segments<Cmp::GraveMultiBlock, Cmp::GraveSegment>( entt::registry &, entt::entity, const Cmp::UUID &, Cmp::Position, const Sprites::SpriteSheet & );
template std::vector<entt::entity> create_multiblock_segments<Cmp::CryptBuildingMultiBlock, Cmp::CryptBuildingSegment>( entt::registry &, entt::entity, const Cmp::UUID &, Cmp::Position, const Sprites::SpriteSheet & );
template std::vector<entt::entity> create_multiblock_segments<Cmp::HealingSpringBuildingMultiBlock, Cmp::HealingSpringBuildingSegment>( entt::registry &, entt::entity, const Cmp::UUID &, Cmp::Position, const Sprites::SpriteSheet & );
template std::vector<entt::entity> create_multiblock_segments<Cmp::RuinBuildingMultiBlock, Cmp::RuinBuildingSegment>( entt::registry &, entt::entity, const Cmp::UUID &, Cmp::Position, const Sprites::SpriteSheet & );
// clang-format on

} // namespace detail

template <typename MULTIBLOCK, typename MBSEGMENT>
  requires IsMB<MULTIBLOCK> && IsMBSegment<MBSEGMENT>
std::pair<entt::entity, std::vector<entt::entity>> add_multiblock_with_segments( entt::registry &reg, sf::Vector2f position,
                                                                                 const Sprites::SpriteSheet &ss, size_t ss_index, float zorder )
{
  auto mb_entt = reg.create();
  Cmp::Position pos( position, ss.get_sprite_size() );
  reg.emplace_or_replace<Cmp::Position>( mb_entt, position, ss.get_sprite_size() );
  auto uuid = Cmp::UUID::generate();
  Factory::detail::create_multiblock<MULTIBLOCK>( reg, mb_entt, uuid, pos, ss, ss_index );
  auto segment_entt_list = Factory::detail::create_multiblock_segments<MULTIBLOCK, MBSEGMENT>( reg, mb_entt, uuid, pos, ss );

  for ( auto [mb_entt, mb_cmp, mb_zorder_cmp] : reg.view<MULTIBLOCK, Cmp::ZOrderValue>().each() )
  {
    if ( zorder != 0 )
    {
      // Use the function param if explicitly set
      mb_zorder_cmp.setZOrder( zorder );
    }
    else if ( ss.get_zorder( ss_index ) != 0 )
    {
      // Use the y-axis position plus the json zorder value
      mb_zorder_cmp.setZOrder( static_cast<sf::FloatRect>( mb_cmp ).position.y + ss.get_zorder( ss_index ) );
    }
    else
    {
      // fallback to the y-axis position only
      mb_zorder_cmp.setZOrder( static_cast<sf::FloatRect>( mb_cmp ).position.y );
    }
  }
  return { mb_entt, segment_entt_list };
}

// clang-format off
template std::pair<entt::entity, std::vector<entt::entity>> add_multiblock_with_segments<Cmp::PlantMultiBlock, Cmp::PlantSegment>( entt::registry &, sf::Vector2f, const Sprites::SpriteSheet &, size_t, float );
template std::pair<entt::entity, std::vector<entt::entity>> add_multiblock_with_segments<Cmp::GraveExitMultiBlock, Cmp::GraveExitSegment>( entt::registry &, sf::Vector2f, const Sprites::SpriteSheet &, size_t, float );
template std::pair<entt::entity, std::vector<entt::entity>> add_multiblock_with_segments<Cmp::HealingSpringMultiBlock, Cmp::HealingSpringSegment>( entt::registry &, sf::Vector2f, const Sprites::SpriteSheet &, size_t, float );
template std::pair<entt::entity, std::vector<entt::entity>> add_multiblock_with_segments<Cmp::CryptObjectiveMultiBlock, Cmp::CryptObjectiveSegment>( entt::registry &, sf::Vector2f, const Sprites::SpriteSheet &, size_t, float );
template std::pair<entt::entity, std::vector<entt::entity>> add_multiblock_with_segments<Cmp::RuinStairsLowerMultiBlock, Cmp::RuinStairsSegment>( entt::registry &, sf::Vector2f, const Sprites::SpriteSheet &, size_t, float );
template std::pair<entt::entity, std::vector<entt::entity>> add_multiblock_with_segments<Cmp::RuinStairsUpperMultiBlock, Cmp::RuinStairsSegment>( entt::registry &, sf::Vector2f, const Sprites::SpriteSheet &, size_t, float );
template std::pair<entt::entity, std::vector<entt::entity>> add_multiblock_with_segments<Cmp::RuinStairsBalustradeMultiBlock, Cmp::RuinStairsSegment>( entt::registry &, sf::Vector2f, const Sprites::SpriteSheet &, size_t, float );
template std::pair<entt::entity, std::vector<entt::entity>> add_multiblock_with_segments<Cmp::RuinStairsGateMultiBlock, Cmp::RuinGateSegment>( entt::registry &, sf::Vector2f, const Sprites::SpriteSheet &, size_t, float );
template std::pair<entt::entity, std::vector<entt::entity>> add_multiblock_with_segments<Cmp::RuinHexagramMultiBlock, Cmp::RuinHexagramSegment>( entt::registry &, sf::Vector2f, const Sprites::SpriteSheet &, size_t, float );
template std::pair<entt::entity, std::vector<entt::entity>> add_multiblock_with_segments<Cmp::AltarMultiBlock, Cmp::AltarSegment>( entt::registry &, sf::Vector2f, const Sprites::SpriteSheet &, size_t, float );
template std::pair<entt::entity, std::vector<entt::entity>> add_multiblock_with_segments<Cmp::GraveMultiBlock, Cmp::GraveSegment>( entt::registry &, sf::Vector2f, const Sprites::SpriteSheet &, size_t, float );
template std::pair<entt::entity, std::vector<entt::entity>> add_multiblock_with_segments<Cmp::CryptBuildingMultiBlock, Cmp::CryptBuildingSegment>( entt::registry &, sf::Vector2f, const Sprites::SpriteSheet &, size_t, float );
template std::pair<entt::entity, std::vector<entt::entity>> add_multiblock_with_segments<Cmp::HealingSpringBuildingMultiBlock, Cmp::HealingSpringBuildingSegment>( entt::registry &, sf::Vector2f, const Sprites::SpriteSheet &, size_t, float );
template std::pair<entt::entity, std::vector<entt::entity>> add_multiblock_with_segments<Cmp::RuinBuildingMultiBlock, Cmp::RuinBuildingSegment>( entt::registry &, sf::Vector2f, const Sprites::SpriteSheet &, size_t, float );
// clang-format on

} // namespace Game::Factory
