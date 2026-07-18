#include <Factory/MultiblockFactoryImpl.hpp>

namespace Game::Factory
{

namespace detail
{

// clang-format off
template void create_multiblock<Cmp::AltarMultiBlock>( entt::registry &, entt::entity, const Cmp::UUID &, Cmp::Position, const Sprites::SpriteSheet &, size_t );
template void create_multiblock<Cmp::GraveMultiBlock>( entt::registry &, entt::entity, const Cmp::UUID &, Cmp::Position, const Sprites::SpriteSheet &, size_t );
template void create_multiblock<Cmp::GraveExitMultiBlock>( entt::registry &, entt::entity, const Cmp::UUID &, Cmp::Position, const Sprites::SpriteSheet &, size_t );

template void update_segments<Cmp::AltarMultiBlock, Cmp::AltarSegment>( entt::registry &, const Sprites::SpriteSheet &, entt::entity, Cmp::AltarMultiBlock );
template void update_segments<Cmp::GraveMultiBlock, Cmp::GraveSegment>( entt::registry &, const Sprites::SpriteSheet &, entt::entity, Cmp::GraveMultiBlock );
template void update_segments<Cmp::GraveExitMultiBlock, Cmp::GraveExitSegment>( entt::registry &, const Sprites::SpriteSheet &, entt::entity, Cmp::GraveExitMultiBlock );

template std::vector<entt::entity> create_multiblock_segments<Cmp::AltarMultiBlock, Cmp::AltarSegment>( entt::registry &, entt::entity, const Cmp::UUID &, Cmp::Position, const Sprites::SpriteSheet & );
template std::vector<entt::entity> create_multiblock_segments<Cmp::GraveMultiBlock, Cmp::GraveSegment>( entt::registry &, entt::entity, const Cmp::UUID &, Cmp::Position, const Sprites::SpriteSheet & );
template std::vector<entt::entity> create_multiblock_segments<Cmp::GraveExitMultiBlock, Cmp::GraveExitSegment>( entt::registry &, entt::entity, const Cmp::UUID &, Cmp::Position, const Sprites::SpriteSheet & );
// clang-format on

} // namespace detail

// clang-format off
template std::pair<entt::entity, std::vector<entt::entity>> add_multiblock_with_segments<Cmp::AltarMultiBlock, Cmp::AltarSegment>( entt::registry &, sf::Vector2f, const Sprites::SpriteSheet &, size_t, float );
template std::pair<entt::entity, std::vector<entt::entity>> add_multiblock_with_segments<Cmp::GraveMultiBlock, Cmp::GraveSegment>( entt::registry &, sf::Vector2f, const Sprites::SpriteSheet &, size_t, float );
template std::pair<entt::entity, std::vector<entt::entity>> add_multiblock_with_segments<Cmp::GraveExitMultiBlock, Cmp::GraveExitSegment>( entt::registry &, sf::Vector2f, const Sprites::SpriteSheet &, size_t, float );
// clang-format on

} // namespace Game::Factory
