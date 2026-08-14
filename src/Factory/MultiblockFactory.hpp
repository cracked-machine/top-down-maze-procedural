#ifndef SRC_FACTORY_MULTIBLOCKFACTORY_HPP__
#define SRC_FACTORY_MULTIBLOCKFACTORY_HPP__

#include <Components/Altar/AltarMultiBlock.hpp>
#include <Components/Altar/AltarSegment.hpp>
#include <Components/AnimData.hpp>
#include <Components/Armable.hpp>
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
#include <Components/Npc/NoPathFinding.hpp>
#include <Components/Player/PlayerNoPath.hpp>
#include <Components/Position.hpp>
#include <Components/ReservedPosition.hpp>
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
extern template void create_multiblock<Cmp::GraveExitMultiBlock>( entt::registry &, entt::entity, const Cmp::UUID &, Cmp::Position, const Sprites::SpriteSheet &, size_t );
extern template void create_multiblock<Cmp::HealingSpringMultiBlock>( entt::registry &, entt::entity, const Cmp::UUID &, Cmp::Position, const Sprites::SpriteSheet &, size_t );
extern template void create_multiblock<Cmp::CryptObjectiveMultiBlock>( entt::registry &, entt::entity, const Cmp::UUID &, Cmp::Position, const Sprites::SpriteSheet &, size_t );
extern template void create_multiblock<Cmp::RuinStairsLowerMultiBlock>( entt::registry &, entt::entity, const Cmp::UUID &, Cmp::Position, const Sprites::SpriteSheet &, size_t );
extern template void create_multiblock<Cmp::RuinStairsUpperMultiBlock>( entt::registry &, entt::entity, const Cmp::UUID &, Cmp::Position, const Sprites::SpriteSheet &, size_t );
extern template void create_multiblock<Cmp::RuinStairsBalustradeMultiBlock>( entt::registry &, entt::entity, const Cmp::UUID &, Cmp::Position, const Sprites::SpriteSheet &, size_t );
extern template void create_multiblock<Cmp::RuinStairsGateMultiBlock>( entt::registry &, entt::entity, const Cmp::UUID &, Cmp::Position, const Sprites::SpriteSheet &, size_t );
extern template void create_multiblock<Cmp::RuinHexagramMultiBlock>( entt::registry &, entt::entity, const Cmp::UUID &, Cmp::Position, const Sprites::SpriteSheet &, size_t );
extern template void create_multiblock<Cmp::AltarMultiBlock>( entt::registry &, entt::entity, const Cmp::UUID &, Cmp::Position, const Sprites::SpriteSheet &, size_t );
extern template void create_multiblock<Cmp::GraveMultiBlock>( entt::registry &, entt::entity, const Cmp::UUID &, Cmp::Position, const Sprites::SpriteSheet &, size_t );
extern template void create_multiblock<Cmp::CryptBuildingMultiBlock>( entt::registry &, entt::entity, const Cmp::UUID &, Cmp::Position, const Sprites::SpriteSheet &, size_t );
extern template void create_multiblock<Cmp::HealingSpringBuildingMultiBlock>( entt::registry &, entt::entity, const Cmp::UUID &, Cmp::Position, const Sprites::SpriteSheet &, size_t );
extern template void create_multiblock<Cmp::RuinBuildingMultiBlock>( entt::registry &, entt::entity, const Cmp::UUID &, Cmp::Position, const Sprites::SpriteSheet &, size_t );

extern template void update_segments<Cmp::PlantMultiBlock, Cmp::PlantSegment>( entt::registry &, const Sprites::SpriteSheet &, entt::entity, Cmp::PlantMultiBlock );
extern template void update_segments<Cmp::GraveExitMultiBlock, Cmp::GraveExitSegment>( entt::registry &, const Sprites::SpriteSheet &, entt::entity, Cmp::GraveExitMultiBlock );
extern template void update_segments<Cmp::HealingSpringMultiBlock, Cmp::HealingSpringSegment>( entt::registry &, const Sprites::SpriteSheet &, entt::entity, Cmp::HealingSpringMultiBlock );
extern template void update_segments<Cmp::CryptObjectiveMultiBlock, Cmp::CryptObjectiveSegment>( entt::registry &, const Sprites::SpriteSheet &, entt::entity, Cmp::CryptObjectiveMultiBlock );
extern template void update_segments<Cmp::RuinStairsLowerMultiBlock, Cmp::RuinStairsSegment>( entt::registry &, const Sprites::SpriteSheet &, entt::entity, Cmp::RuinStairsLowerMultiBlock );
extern template void update_segments<Cmp::RuinStairsUpperMultiBlock, Cmp::RuinStairsSegment>( entt::registry &, const Sprites::SpriteSheet &, entt::entity, Cmp::RuinStairsUpperMultiBlock );
extern template void update_segments<Cmp::RuinStairsBalustradeMultiBlock, Cmp::RuinStairsSegment>( entt::registry &, const Sprites::SpriteSheet &, entt::entity, Cmp::RuinStairsBalustradeMultiBlock );
extern template void update_segments<Cmp::RuinStairsGateMultiBlock, Cmp::RuinGateSegment>( entt::registry &, const Sprites::SpriteSheet &, entt::entity, Cmp::RuinStairsGateMultiBlock );
extern template void update_segments<Cmp::RuinHexagramMultiBlock, Cmp::RuinHexagramSegment>( entt::registry &, const Sprites::SpriteSheet &, entt::entity, Cmp::RuinHexagramMultiBlock );
extern template void update_segments<Cmp::AltarMultiBlock, Cmp::AltarSegment>( entt::registry &, const Sprites::SpriteSheet &, entt::entity, Cmp::AltarMultiBlock );
extern template void update_segments<Cmp::GraveMultiBlock, Cmp::GraveSegment>( entt::registry &, const Sprites::SpriteSheet &, entt::entity, Cmp::GraveMultiBlock );
extern template void update_segments<Cmp::CryptBuildingMultiBlock, Cmp::CryptBuildingSegment>( entt::registry &, const Sprites::SpriteSheet &, entt::entity, Cmp::CryptBuildingMultiBlock );
extern template void update_segments<Cmp::HealingSpringBuildingMultiBlock, Cmp::HealingSpringBuildingSegment>( entt::registry &, const Sprites::SpriteSheet &, entt::entity, Cmp::HealingSpringBuildingMultiBlock );
extern template void update_segments<Cmp::RuinBuildingMultiBlock, Cmp::RuinBuildingSegment>( entt::registry &, const Sprites::SpriteSheet &, entt::entity, Cmp::RuinBuildingMultiBlock );

extern template std::vector<entt::entity> create_multiblock_segments<Cmp::PlantMultiBlock, Cmp::PlantSegment>( entt::registry &, entt::entity, const Cmp::UUID &, Cmp::Position, const Sprites::SpriteSheet & );
extern template std::vector<entt::entity> create_multiblock_segments<Cmp::GraveExitMultiBlock, Cmp::GraveExitSegment>( entt::registry &, entt::entity, const Cmp::UUID &, Cmp::Position, const Sprites::SpriteSheet & );
extern template std::vector<entt::entity> create_multiblock_segments<Cmp::HealingSpringMultiBlock, Cmp::HealingSpringSegment>( entt::registry &, entt::entity, const Cmp::UUID &, Cmp::Position, const Sprites::SpriteSheet & );
extern template std::vector<entt::entity> create_multiblock_segments<Cmp::CryptObjectiveMultiBlock, Cmp::CryptObjectiveSegment>( entt::registry &, entt::entity, const Cmp::UUID &, Cmp::Position, const Sprites::SpriteSheet & );
extern template std::vector<entt::entity> create_multiblock_segments<Cmp::RuinStairsLowerMultiBlock, Cmp::RuinStairsSegment>( entt::registry &, entt::entity, const Cmp::UUID &, Cmp::Position, const Sprites::SpriteSheet & );
extern template std::vector<entt::entity> create_multiblock_segments<Cmp::RuinStairsUpperMultiBlock, Cmp::RuinStairsSegment>( entt::registry &, entt::entity, const Cmp::UUID &, Cmp::Position, const Sprites::SpriteSheet & );
extern template std::vector<entt::entity> create_multiblock_segments<Cmp::RuinStairsBalustradeMultiBlock, Cmp::RuinStairsSegment>( entt::registry &, entt::entity, const Cmp::UUID &, Cmp::Position, const Sprites::SpriteSheet & );
extern template std::vector<entt::entity> create_multiblock_segments<Cmp::RuinStairsGateMultiBlock, Cmp::RuinGateSegment>( entt::registry &, entt::entity, const Cmp::UUID &, Cmp::Position, const Sprites::SpriteSheet & );
extern template std::vector<entt::entity> create_multiblock_segments<Cmp::RuinHexagramMultiBlock, Cmp::RuinHexagramSegment>( entt::registry &, entt::entity, const Cmp::UUID &, Cmp::Position, const Sprites::SpriteSheet & );
extern template std::vector<entt::entity> create_multiblock_segments<Cmp::AltarMultiBlock, Cmp::AltarSegment>( entt::registry &, entt::entity, const Cmp::UUID &, Cmp::Position, const Sprites::SpriteSheet & );
extern template std::vector<entt::entity> create_multiblock_segments<Cmp::GraveMultiBlock, Cmp::GraveSegment>( entt::registry &, entt::entity, const Cmp::UUID &, Cmp::Position, const Sprites::SpriteSheet & );
extern template std::vector<entt::entity> create_multiblock_segments<Cmp::CryptBuildingMultiBlock, Cmp::CryptBuildingSegment>( entt::registry &, entt::entity, const Cmp::UUID &, Cmp::Position, const Sprites::SpriteSheet & );
extern template std::vector<entt::entity> create_multiblock_segments<Cmp::HealingSpringBuildingMultiBlock, Cmp::HealingSpringBuildingSegment>( entt::registry &, entt::entity, const Cmp::UUID &, Cmp::Position, const Sprites::SpriteSheet & );
extern template std::vector<entt::entity> create_multiblock_segments<Cmp::RuinBuildingMultiBlock, Cmp::RuinBuildingSegment>( entt::registry &, entt::entity, const Cmp::UUID &, Cmp::Position, const Sprites::SpriteSheet & );

} // namespace Game::Factory::Multiblock::detail

namespace Game::Factory::Multiblock
{

extern template std::pair<entt::entity, std::vector<entt::entity>> add_multiblock_with_segments<Cmp::PlantMultiBlock, Cmp::PlantSegment>( entt::registry &, sf::Vector2f, const Sprites::SpriteSheet &, size_t, float );
extern template std::pair<entt::entity, std::vector<entt::entity>> add_multiblock_with_segments<Cmp::GraveExitMultiBlock, Cmp::GraveExitSegment>( entt::registry &, sf::Vector2f, const Sprites::SpriteSheet &, size_t, float );
extern template std::pair<entt::entity, std::vector<entt::entity>> add_multiblock_with_segments<Cmp::HealingSpringMultiBlock, Cmp::HealingSpringSegment>( entt::registry &, sf::Vector2f, const Sprites::SpriteSheet &, size_t, float );
extern template std::pair<entt::entity, std::vector<entt::entity>> add_multiblock_with_segments<Cmp::CryptObjectiveMultiBlock, Cmp::CryptObjectiveSegment>( entt::registry &, sf::Vector2f, const Sprites::SpriteSheet &, size_t, float );
extern template std::pair<entt::entity, std::vector<entt::entity>> add_multiblock_with_segments<Cmp::RuinStairsLowerMultiBlock, Cmp::RuinStairsSegment>( entt::registry &, sf::Vector2f, const Sprites::SpriteSheet &, size_t, float );
extern template std::pair<entt::entity, std::vector<entt::entity>> add_multiblock_with_segments<Cmp::RuinStairsUpperMultiBlock, Cmp::RuinStairsSegment>( entt::registry &, sf::Vector2f, const Sprites::SpriteSheet &, size_t, float );
extern template std::pair<entt::entity, std::vector<entt::entity>> add_multiblock_with_segments<Cmp::RuinStairsBalustradeMultiBlock, Cmp::RuinStairsSegment>( entt::registry &, sf::Vector2f, const Sprites::SpriteSheet &, size_t, float );
extern template std::pair<entt::entity, std::vector<entt::entity>> add_multiblock_with_segments<Cmp::RuinStairsGateMultiBlock, Cmp::RuinGateSegment>( entt::registry &, sf::Vector2f, const Sprites::SpriteSheet &, size_t, float );
extern template std::pair<entt::entity, std::vector<entt::entity>> add_multiblock_with_segments<Cmp::RuinHexagramMultiBlock, Cmp::RuinHexagramSegment>( entt::registry &, sf::Vector2f, const Sprites::SpriteSheet &, size_t, float );
extern template std::pair<entt::entity, std::vector<entt::entity>> add_multiblock_with_segments<Cmp::AltarMultiBlock, Cmp::AltarSegment>( entt::registry &, sf::Vector2f, const Sprites::SpriteSheet &, size_t, float );
extern template std::pair<entt::entity, std::vector<entt::entity>> add_multiblock_with_segments<Cmp::GraveMultiBlock, Cmp::GraveSegment>( entt::registry &, sf::Vector2f, const Sprites::SpriteSheet &, size_t, float );
extern template std::pair<entt::entity, std::vector<entt::entity>> add_multiblock_with_segments<Cmp::CryptBuildingMultiBlock, Cmp::CryptBuildingSegment>( entt::registry &, sf::Vector2f, const Sprites::SpriteSheet &, size_t, float );
extern template std::pair<entt::entity, std::vector<entt::entity>> add_multiblock_with_segments<Cmp::HealingSpringBuildingMultiBlock, Cmp::HealingSpringBuildingSegment>( entt::registry &, sf::Vector2f, const Sprites::SpriteSheet &, size_t, float );
extern template std::pair<entt::entity, std::vector<entt::entity>> add_multiblock_with_segments<Cmp::RuinBuildingMultiBlock, Cmp::RuinBuildingSegment>( entt::registry &, sf::Vector2f, const Sprites::SpriteSheet &, size_t, float );

} // namespace Game::Factory::Multiblock
// clang-format on

#endif // SRC_FACTORY_MULTIBLOCKFACTORY_HPP__
