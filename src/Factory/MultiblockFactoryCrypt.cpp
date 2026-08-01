#include <Factory/MultiblockFactoryImpl.hpp>

namespace Game::Factory::Multiblock
{

namespace detail
{

// clang-format off
template void create_multiblock<Cmp::CryptBuildingMultiBlock>( entt::registry &, entt::entity, const Cmp::UUID &, Cmp::Position, const Sprites::SpriteSheet &, size_t );
template void create_multiblock<Cmp::CryptObjectiveMultiBlock>( entt::registry &, entt::entity, const Cmp::UUID &, Cmp::Position, const Sprites::SpriteSheet &, size_t );

template void update_segments<Cmp::CryptBuildingMultiBlock, Cmp::CryptBuildingSegment>( entt::registry &, const Sprites::SpriteSheet &, entt::entity, Cmp::CryptBuildingMultiBlock );
template void update_segments<Cmp::CryptObjectiveMultiBlock, Cmp::CryptObjectiveSegment>( entt::registry &, const Sprites::SpriteSheet &, entt::entity, Cmp::CryptObjectiveMultiBlock );

template std::vector<entt::entity> create_multiblock_segments<Cmp::CryptBuildingMultiBlock, Cmp::CryptBuildingSegment>( entt::registry &, entt::entity, const Cmp::UUID &, Cmp::Position, const Sprites::SpriteSheet & );
template std::vector<entt::entity> create_multiblock_segments<Cmp::CryptObjectiveMultiBlock, Cmp::CryptObjectiveSegment>( entt::registry &, entt::entity, const Cmp::UUID &, Cmp::Position, const Sprites::SpriteSheet & );
// clang-format on

} // namespace detail

// clang-format off
template std::pair<entt::entity, std::vector<entt::entity>> add_multiblock_with_segments<Cmp::CryptBuildingMultiBlock, Cmp::CryptBuildingSegment>( entt::registry &, sf::Vector2f, const Sprites::SpriteSheet &, size_t, float );
template std::pair<entt::entity, std::vector<entt::entity>> add_multiblock_with_segments<Cmp::CryptObjectiveMultiBlock, Cmp::CryptObjectiveSegment>( entt::registry &, sf::Vector2f, const Sprites::SpriteSheet &, size_t, float );
// clang-format on

} // namespace Game::Factory::Multiblock
