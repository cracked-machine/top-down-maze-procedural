#include <Factory/MultiblockFactoryImpl.hpp>

namespace Game::Factory::Multiblock
{

namespace detail
{

// clang-format off
template void create_multiblock<Cmp::RuinBuildingMultiBlock>( entt::registry &, entt::entity, const Cmp::UUID &, Cmp::Position, const Sprites::SpriteSheet &, size_t );
template void create_multiblock<Cmp::RuinHexagramMultiBlock>( entt::registry &, entt::entity, const Cmp::UUID &, Cmp::Position, const Sprites::SpriteSheet &, size_t );

template void update_segments<Cmp::RuinBuildingMultiBlock, Cmp::RuinBuildingSegment>( entt::registry &, const Sprites::SpriteSheet &, entt::entity, Cmp::RuinBuildingMultiBlock );
template void update_segments<Cmp::RuinHexagramMultiBlock, Cmp::RuinHexagramSegment>( entt::registry &, const Sprites::SpriteSheet &, entt::entity, Cmp::RuinHexagramMultiBlock );

template std::vector<entt::entity> create_multiblock_segments<Cmp::RuinBuildingMultiBlock, Cmp::RuinBuildingSegment>( entt::registry &, entt::entity, const Cmp::UUID &, Cmp::Position, const Sprites::SpriteSheet & );
template std::vector<entt::entity> create_multiblock_segments<Cmp::RuinHexagramMultiBlock, Cmp::RuinHexagramSegment>( entt::registry &, entt::entity, const Cmp::UUID &, Cmp::Position, const Sprites::SpriteSheet & );
// clang-format on

} // namespace detail

// clang-format off
template std::pair<entt::entity, std::vector<entt::entity>> add_multiblock_with_segments<Cmp::RuinBuildingMultiBlock, Cmp::RuinBuildingSegment>( entt::registry &, sf::Vector2f, const Sprites::SpriteSheet &, size_t, float );
template std::pair<entt::entity, std::vector<entt::entity>> add_multiblock_with_segments<Cmp::RuinHexagramMultiBlock, Cmp::RuinHexagramSegment>( entt::registry &, sf::Vector2f, const Sprites::SpriteSheet &, size_t, float );
// clang-format on

} // namespace Game::Factory::Multiblock
