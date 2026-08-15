#ifndef SRC_FACTORY_MULTIBLOCKFACTORY_HPP__
#define SRC_FACTORY_MULTIBLOCKFACTORY_HPP__

#include <Components/Altar/MultiBlock.hpp>
#include <Components/Altar/Segment.hpp>
#include <Components/AnimData.hpp>
#include <Components/Armable.hpp>
#include <Components/Crypt/BuildingMultiBlock.hpp>
#include <Components/Crypt/BuildingSegment.hpp>
#include <Components/Crypt/Entrance.hpp>
#include <Components/Crypt/InteriorMultiBlock.hpp>
#include <Components/Crypt/InteriorSegment.hpp>
#include <Components/Crypt/ObjectiveMultiBlock.hpp>
#include <Components/Crypt/ObjectiveSegment.hpp>
#include <Components/Exit.hpp>
#include <Components/Grave/ExitMultiBlock.hpp>
#include <Components/Grave/ExitSegment.hpp>
#include <Components/Grave/MultiBlock.hpp>
#include <Components/Grave/Segment.hpp>
#include <Components/Grave/PlantMultiBlock.hpp>
#include <Components/Grave/PlantSegment.hpp>
#include <Components/Npc/NoPathFinding.hpp>
#include <Components/Player/NoPath.hpp>
#include <Components/Position.hpp>
#include <Components/ReservedPosition.hpp>
#include <Components/Ruin/BuildingMultiBlock.hpp>
#include <Components/Ruin/BuildingSegment.hpp>
#include <Components/Ruin/Entrance.hpp>
#include <Components/Ruin/GateSegment.hpp>
#include <Components/Ruin/HexagramMultiBlock.hpp>
#include <Components/Ruin/HexagramSegment.hpp>
#include <Components/Ruin/StairsBalustradeMultiBlock.hpp>
#include <Components/Ruin/StairsGateMultiBlock.hpp>
#include <Components/Ruin/StairsLowerMultiBlock.hpp>
#include <Components/Ruin/StairsSegment.hpp>
#include <Components/Ruin/StairsUpperMultiBlock.hpp>
#include <Components/Spring/HealingSpringBuildingMultiBlock.hpp>
#include <Components/Spring/HealingSpringBuildingSegment.hpp>
#include <Components/Spring/HealingSpringEntrance.hpp>
#include <Components/Spring/HealingSpringMultiBlock.hpp>
#include <Components/Spring/HealingSpringSegment.hpp>
#include <Components/UUID.hpp>
#include <Components/ZOrderValue.hpp>
#include <entt/entity/fwd.hpp>
#include <entt/fwd.hpp>

namespace Game::Factory::Multiblock
{

template <typename T>
concept IsMBSegment = requires( T t ) {
  { t.isSolidMask() } -> std::convertible_to<bool>;
  { t.set_solid_mask( true ) } -> std::same_as<void>;
};

template <typename T>
concept IsMB = std::derived_from<T, sf::FloatRect>;

namespace detail
{

template <typename MULTIBLOCK>
  requires IsMB<MULTIBLOCK>
void create_multiblock( entt::registry &reg, entt::entity entity, const Cmp::UUID &uuid, Cmp::Position pos, const Sprites::SpriteSheet &ss,
                        size_t ss_index = 0 );

template <typename MULTIBLOCK, typename MBSEGMENT>
  requires IsMB<MULTIBLOCK> && IsMBSegment<MBSEGMENT>
void update_segments( entt::registry &reg, const Sprites::SpriteSheet &ss, entt::entity mb_entt, MULTIBLOCK mb_cmp );

template <typename MULTIBLOCK, typename MBSEGMENT>
  requires IsMB<MULTIBLOCK> && IsMBSegment<MBSEGMENT>
std::vector<entt::entity> create_multiblock_segments( entt::registry &reg, entt::entity multiblock_entity, const Cmp::UUID &uuid,
                                                      Cmp::Position mb_pos_cmp, const Sprites::SpriteSheet &ss );

} // namespace detail

template <typename MULTIBLOCK, typename MBSEGMENT>
  requires IsMB<MULTIBLOCK> && IsMBSegment<MBSEGMENT>
std::pair<entt::entity, std::vector<entt::entity>>
add_multiblock_with_segments( entt::registry &reg, sf::Vector2f position, const Sprites::SpriteSheet &ss, size_t ss_index = 0, float zorder = 0 );

} // namespace Game::Factory::Multiblock

// Explicit instantiations are defined in MultiblockFactory.cpp for every MULTIBLOCK/MBSEGMENT
// combination used across the codebase.
// clang-format off
namespace Game::Factory::Multiblock::detail
{

extern template void create_multiblock<Cmp::PlantMultiBlock>( entt::registry &, entt::entity, const Cmp::UUID &, Cmp::Position, const Sprites::SpriteSheet &, size_t );
extern template void create_multiblock<Cmp::Grave::ExitMultiBlock>( entt::registry &, entt::entity, const Cmp::UUID &, Cmp::Position, const Sprites::SpriteSheet &, size_t );
extern template void create_multiblock<Cmp::HealingSpringMultiBlock>( entt::registry &, entt::entity, const Cmp::UUID &, Cmp::Position, const Sprites::SpriteSheet &, size_t );
extern template void create_multiblock<Cmp::Crypt::ObjectiveMultiBlock>( entt::registry &, entt::entity, const Cmp::UUID &, Cmp::Position, const Sprites::SpriteSheet &, size_t );
extern template void create_multiblock<Cmp::Ruin::StairsLowerMultiBlock>( entt::registry &, entt::entity, const Cmp::UUID &, Cmp::Position, const Sprites::SpriteSheet &, size_t );
extern template void create_multiblock<Cmp::Ruin::StairsUpperMultiBlock>( entt::registry &, entt::entity, const Cmp::UUID &, Cmp::Position, const Sprites::SpriteSheet &, size_t );
extern template void create_multiblock<Cmp::Ruin::StairsBalustradeMultiBlock>( entt::registry &, entt::entity, const Cmp::UUID &, Cmp::Position, const Sprites::SpriteSheet &, size_t );
extern template void create_multiblock<Cmp::Ruin::StairsGateMultiBlock>( entt::registry &, entt::entity, const Cmp::UUID &, Cmp::Position, const Sprites::SpriteSheet &, size_t );
extern template void create_multiblock<Cmp::Ruin::HexagramMultiBlock>( entt::registry &, entt::entity, const Cmp::UUID &, Cmp::Position, const Sprites::SpriteSheet &, size_t );
extern template void create_multiblock<Cmp::Altar::MultiBlock>( entt::registry &, entt::entity, const Cmp::UUID &, Cmp::Position, const Sprites::SpriteSheet &, size_t );
extern template void create_multiblock<Cmp::Grave::MultiBlock>( entt::registry &, entt::entity, const Cmp::UUID &, Cmp::Position, const Sprites::SpriteSheet &, size_t );
extern template void create_multiblock<Cmp::Crypt::BuildingMultiBlock>( entt::registry &, entt::entity, const Cmp::UUID &, Cmp::Position, const Sprites::SpriteSheet &, size_t );
extern template void create_multiblock<Cmp::HealingSpringBuildingMultiBlock>( entt::registry &, entt::entity, const Cmp::UUID &, Cmp::Position, const Sprites::SpriteSheet &, size_t );
extern template void create_multiblock<Cmp::Ruin::BuildingMultiBlock>( entt::registry &, entt::entity, const Cmp::UUID &, Cmp::Position, const Sprites::SpriteSheet &, size_t );

extern template void update_segments<Cmp::PlantMultiBlock, Cmp::PlantSegment>( entt::registry &, const Sprites::SpriteSheet &, entt::entity, Cmp::PlantMultiBlock );
extern template void update_segments<Cmp::Grave::ExitMultiBlock, Cmp::Grave::ExitSegment>( entt::registry &, const Sprites::SpriteSheet &, entt::entity, Cmp::Grave::ExitMultiBlock );
extern template void update_segments<Cmp::HealingSpringMultiBlock, Cmp::HealingSpringSegment>( entt::registry &, const Sprites::SpriteSheet &, entt::entity, Cmp::HealingSpringMultiBlock );
extern template void update_segments<Cmp::Crypt::ObjectiveMultiBlock, Cmp::Crypt::ObjectiveSegment>( entt::registry &, const Sprites::SpriteSheet &, entt::entity, Cmp::Crypt::ObjectiveMultiBlock );
extern template void update_segments<Cmp::Ruin::StairsLowerMultiBlock, Cmp::Ruin::StairsSegment>( entt::registry &, const Sprites::SpriteSheet &, entt::entity, Cmp::Ruin::StairsLowerMultiBlock );
extern template void update_segments<Cmp::Ruin::StairsUpperMultiBlock, Cmp::Ruin::StairsSegment>( entt::registry &, const Sprites::SpriteSheet &, entt::entity, Cmp::Ruin::StairsUpperMultiBlock );
extern template void update_segments<Cmp::Ruin::StairsBalustradeMultiBlock, Cmp::Ruin::StairsSegment>( entt::registry &, const Sprites::SpriteSheet &, entt::entity, Cmp::Ruin::StairsBalustradeMultiBlock );
extern template void update_segments<Cmp::Ruin::StairsGateMultiBlock, Cmp::Ruin::GateSegment>( entt::registry &, const Sprites::SpriteSheet &, entt::entity, Cmp::Ruin::StairsGateMultiBlock );
extern template void update_segments<Cmp::Ruin::HexagramMultiBlock, Cmp::Ruin::HexagramSegment>( entt::registry &, const Sprites::SpriteSheet &, entt::entity, Cmp::Ruin::HexagramMultiBlock );
extern template void update_segments<Cmp::Altar::MultiBlock, Cmp::Altar::Segment>( entt::registry &, const Sprites::SpriteSheet &, entt::entity, Cmp::Altar::MultiBlock );
extern template void update_segments<Cmp::Grave::MultiBlock, Cmp::Grave::Segment>( entt::registry &, const Sprites::SpriteSheet &, entt::entity, Cmp::Grave::MultiBlock );
extern template void update_segments<Cmp::Crypt::BuildingMultiBlock, Cmp::Crypt::BuildingSegment>( entt::registry &, const Sprites::SpriteSheet &, entt::entity, Cmp::Crypt::BuildingMultiBlock );
extern template void update_segments<Cmp::HealingSpringBuildingMultiBlock, Cmp::HealingSpringBuildingSegment>( entt::registry &, const Sprites::SpriteSheet &, entt::entity, Cmp::HealingSpringBuildingMultiBlock );
extern template void update_segments<Cmp::Ruin::BuildingMultiBlock, Cmp::Ruin::BuildingSegment>( entt::registry &, const Sprites::SpriteSheet &, entt::entity, Cmp::Ruin::BuildingMultiBlock );

extern template std::vector<entt::entity> create_multiblock_segments<Cmp::PlantMultiBlock, Cmp::PlantSegment>( entt::registry &, entt::entity, const Cmp::UUID &, Cmp::Position, const Sprites::SpriteSheet & );
extern template std::vector<entt::entity> create_multiblock_segments<Cmp::Grave::ExitMultiBlock, Cmp::Grave::ExitSegment>( entt::registry &, entt::entity, const Cmp::UUID &, Cmp::Position, const Sprites::SpriteSheet & );
extern template std::vector<entt::entity> create_multiblock_segments<Cmp::HealingSpringMultiBlock, Cmp::HealingSpringSegment>( entt::registry &, entt::entity, const Cmp::UUID &, Cmp::Position, const Sprites::SpriteSheet & );
extern template std::vector<entt::entity> create_multiblock_segments<Cmp::Crypt::ObjectiveMultiBlock, Cmp::Crypt::ObjectiveSegment>( entt::registry &, entt::entity, const Cmp::UUID &, Cmp::Position, const Sprites::SpriteSheet & );
extern template std::vector<entt::entity> create_multiblock_segments<Cmp::Ruin::StairsLowerMultiBlock, Cmp::Ruin::StairsSegment>( entt::registry &, entt::entity, const Cmp::UUID &, Cmp::Position, const Sprites::SpriteSheet & );
extern template std::vector<entt::entity> create_multiblock_segments<Cmp::Ruin::StairsUpperMultiBlock, Cmp::Ruin::StairsSegment>( entt::registry &, entt::entity, const Cmp::UUID &, Cmp::Position, const Sprites::SpriteSheet & );
extern template std::vector<entt::entity> create_multiblock_segments<Cmp::Ruin::StairsBalustradeMultiBlock, Cmp::Ruin::StairsSegment>( entt::registry &, entt::entity, const Cmp::UUID &, Cmp::Position, const Sprites::SpriteSheet & );
extern template std::vector<entt::entity> create_multiblock_segments<Cmp::Ruin::StairsGateMultiBlock, Cmp::Ruin::GateSegment>( entt::registry &, entt::entity, const Cmp::UUID &, Cmp::Position, const Sprites::SpriteSheet & );
extern template std::vector<entt::entity> create_multiblock_segments<Cmp::Ruin::HexagramMultiBlock, Cmp::Ruin::HexagramSegment>( entt::registry &, entt::entity, const Cmp::UUID &, Cmp::Position, const Sprites::SpriteSheet & );
extern template std::vector<entt::entity> create_multiblock_segments<Cmp::Altar::MultiBlock, Cmp::Altar::Segment>( entt::registry &, entt::entity, const Cmp::UUID &, Cmp::Position, const Sprites::SpriteSheet & );
extern template std::vector<entt::entity> create_multiblock_segments<Cmp::Grave::MultiBlock, Cmp::Grave::Segment>( entt::registry &, entt::entity, const Cmp::UUID &, Cmp::Position, const Sprites::SpriteSheet & );
extern template std::vector<entt::entity> create_multiblock_segments<Cmp::Crypt::BuildingMultiBlock, Cmp::Crypt::BuildingSegment>( entt::registry &, entt::entity, const Cmp::UUID &, Cmp::Position, const Sprites::SpriteSheet & );
extern template std::vector<entt::entity> create_multiblock_segments<Cmp::HealingSpringBuildingMultiBlock, Cmp::HealingSpringBuildingSegment>( entt::registry &, entt::entity, const Cmp::UUID &, Cmp::Position, const Sprites::SpriteSheet & );
extern template std::vector<entt::entity> create_multiblock_segments<Cmp::Ruin::BuildingMultiBlock, Cmp::Ruin::BuildingSegment>( entt::registry &, entt::entity, const Cmp::UUID &, Cmp::Position, const Sprites::SpriteSheet & );

} // namespace Game::Factory::Multiblock::detail

namespace Game::Factory::Multiblock
{

extern template std::pair<entt::entity, std::vector<entt::entity>> add_multiblock_with_segments<Cmp::PlantMultiBlock, Cmp::PlantSegment>( entt::registry &, sf::Vector2f, const Sprites::SpriteSheet &, size_t, float );
extern template std::pair<entt::entity, std::vector<entt::entity>> add_multiblock_with_segments<Cmp::Grave::ExitMultiBlock, Cmp::Grave::ExitSegment>( entt::registry &, sf::Vector2f, const Sprites::SpriteSheet &, size_t, float );
extern template std::pair<entt::entity, std::vector<entt::entity>> add_multiblock_with_segments<Cmp::HealingSpringMultiBlock, Cmp::HealingSpringSegment>( entt::registry &, sf::Vector2f, const Sprites::SpriteSheet &, size_t, float );
extern template std::pair<entt::entity, std::vector<entt::entity>> add_multiblock_with_segments<Cmp::Crypt::ObjectiveMultiBlock, Cmp::Crypt::ObjectiveSegment>( entt::registry &, sf::Vector2f, const Sprites::SpriteSheet &, size_t, float );
extern template std::pair<entt::entity, std::vector<entt::entity>> add_multiblock_with_segments<Cmp::Ruin::StairsLowerMultiBlock, Cmp::Ruin::StairsSegment>( entt::registry &, sf::Vector2f, const Sprites::SpriteSheet &, size_t, float );
extern template std::pair<entt::entity, std::vector<entt::entity>> add_multiblock_with_segments<Cmp::Ruin::StairsUpperMultiBlock, Cmp::Ruin::StairsSegment>( entt::registry &, sf::Vector2f, const Sprites::SpriteSheet &, size_t, float );
extern template std::pair<entt::entity, std::vector<entt::entity>> add_multiblock_with_segments<Cmp::Ruin::StairsBalustradeMultiBlock, Cmp::Ruin::StairsSegment>( entt::registry &, sf::Vector2f, const Sprites::SpriteSheet &, size_t, float );
extern template std::pair<entt::entity, std::vector<entt::entity>> add_multiblock_with_segments<Cmp::Ruin::StairsGateMultiBlock, Cmp::Ruin::GateSegment>( entt::registry &, sf::Vector2f, const Sprites::SpriteSheet &, size_t, float );
extern template std::pair<entt::entity, std::vector<entt::entity>> add_multiblock_with_segments<Cmp::Ruin::HexagramMultiBlock, Cmp::Ruin::HexagramSegment>( entt::registry &, sf::Vector2f, const Sprites::SpriteSheet &, size_t, float );
extern template std::pair<entt::entity, std::vector<entt::entity>> add_multiblock_with_segments<Cmp::Altar::MultiBlock, Cmp::Altar::Segment>( entt::registry &, sf::Vector2f, const Sprites::SpriteSheet &, size_t, float );
extern template std::pair<entt::entity, std::vector<entt::entity>> add_multiblock_with_segments<Cmp::Grave::MultiBlock, Cmp::Grave::Segment>( entt::registry &, sf::Vector2f, const Sprites::SpriteSheet &, size_t, float );
extern template std::pair<entt::entity, std::vector<entt::entity>> add_multiblock_with_segments<Cmp::Crypt::BuildingMultiBlock, Cmp::Crypt::BuildingSegment>( entt::registry &, sf::Vector2f, const Sprites::SpriteSheet &, size_t, float );
extern template std::pair<entt::entity, std::vector<entt::entity>> add_multiblock_with_segments<Cmp::HealingSpringBuildingMultiBlock, Cmp::HealingSpringBuildingSegment>( entt::registry &, sf::Vector2f, const Sprites::SpriteSheet &, size_t, float );
extern template std::pair<entt::entity, std::vector<entt::entity>> add_multiblock_with_segments<Cmp::Ruin::BuildingMultiBlock, Cmp::Ruin::BuildingSegment>( entt::registry &, sf::Vector2f, const Sprites::SpriteSheet &, size_t, float );

} // namespace Game::Factory::Multiblock
// clang-format on

#endif // SRC_FACTORY_MULTIBLOCKFACTORY_HPP__
