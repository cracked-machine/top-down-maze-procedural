#include <Factory/MultiblockFactoryImpl.hpp>

namespace Game::Factory::Multiblock
{

namespace detail
{

// clang-format off
template void create_multiblock<Cmp::RuinStairsLowerMultiBlock>( entt::registry &, entt::entity, const Cmp::UUID &, Cmp::Position, const Sprites::SpriteSheet &, size_t );
template void create_multiblock<Cmp::RuinStairsUpperMultiBlock>( entt::registry &, entt::entity, const Cmp::UUID &, Cmp::Position, const Sprites::SpriteSheet &, size_t );
template void create_multiblock<Cmp::RuinStairsBalustradeMultiBlock>( entt::registry &, entt::entity, const Cmp::UUID &, Cmp::Position, const Sprites::SpriteSheet &, size_t );
template void create_multiblock<Cmp::RuinStairsGateMultiBlock>( entt::registry &, entt::entity, const Cmp::UUID &, Cmp::Position, const Sprites::SpriteSheet &, size_t );

template void update_segments<Cmp::RuinStairsLowerMultiBlock, Cmp::RuinStairsSegment>( entt::registry &, const Sprites::SpriteSheet &, entt::entity, Cmp::RuinStairsLowerMultiBlock );
template void update_segments<Cmp::RuinStairsUpperMultiBlock, Cmp::RuinStairsSegment>( entt::registry &, const Sprites::SpriteSheet &, entt::entity, Cmp::RuinStairsUpperMultiBlock );
template void update_segments<Cmp::RuinStairsBalustradeMultiBlock, Cmp::RuinStairsSegment>( entt::registry &, const Sprites::SpriteSheet &, entt::entity, Cmp::RuinStairsBalustradeMultiBlock );
template void update_segments<Cmp::RuinStairsGateMultiBlock, Cmp::RuinGateSegment>( entt::registry &, const Sprites::SpriteSheet &, entt::entity, Cmp::RuinStairsGateMultiBlock );

template std::vector<entt::entity> create_multiblock_segments<Cmp::RuinStairsLowerMultiBlock, Cmp::RuinStairsSegment>( entt::registry &, entt::entity, const Cmp::UUID &, Cmp::Position, const Sprites::SpriteSheet & );
template std::vector<entt::entity> create_multiblock_segments<Cmp::RuinStairsUpperMultiBlock, Cmp::RuinStairsSegment>( entt::registry &, entt::entity, const Cmp::UUID &, Cmp::Position, const Sprites::SpriteSheet & );
template std::vector<entt::entity> create_multiblock_segments<Cmp::RuinStairsBalustradeMultiBlock, Cmp::RuinStairsSegment>( entt::registry &, entt::entity, const Cmp::UUID &, Cmp::Position, const Sprites::SpriteSheet & );
template std::vector<entt::entity> create_multiblock_segments<Cmp::RuinStairsGateMultiBlock, Cmp::RuinGateSegment>( entt::registry &, entt::entity, const Cmp::UUID &, Cmp::Position, const Sprites::SpriteSheet & );
// clang-format on

} // namespace detail

// clang-format off
template std::pair<entt::entity, std::vector<entt::entity>> add_multiblock_with_segments<Cmp::RuinStairsLowerMultiBlock, Cmp::RuinStairsSegment>( entt::registry &, sf::Vector2f, const Sprites::SpriteSheet &, size_t, float );
template std::pair<entt::entity, std::vector<entt::entity>> add_multiblock_with_segments<Cmp::RuinStairsUpperMultiBlock, Cmp::RuinStairsSegment>( entt::registry &, sf::Vector2f, const Sprites::SpriteSheet &, size_t, float );
template std::pair<entt::entity, std::vector<entt::entity>> add_multiblock_with_segments<Cmp::RuinStairsBalustradeMultiBlock, Cmp::RuinStairsSegment>( entt::registry &, sf::Vector2f, const Sprites::SpriteSheet &, size_t, float );
template std::pair<entt::entity, std::vector<entt::entity>> add_multiblock_with_segments<Cmp::RuinStairsGateMultiBlock, Cmp::RuinGateSegment>( entt::registry &, sf::Vector2f, const Sprites::SpriteSheet &, size_t, float );
// clang-format on

} // namespace Game::Factory::Multiblock
