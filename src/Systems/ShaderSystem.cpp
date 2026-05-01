#include <Shaders/IShaderSprite.hpp>
#include <Systems/ShaderSystem.hpp>

namespace ProceduralMaze::Sys
{

ShaderSystem::ShaderSystem( entt::registry &reg, sf::RenderWindow &window, Sprites::SpriteFactory &sprite_factory, Audio::SoundBank &sound_bank )
    : BaseSystem( reg, window, sprite_factory, sound_bank )
{
}

std::vector<entt::entity> ShaderSystem::add_to_registry( ShaderSpriteOwner owner, Cmp::ZOrderValue zorder )
{
  std::vector<entt::entity> entt_list;

  auto entt = reg().create();
  reg().emplace<ShaderSpriteOwner>( entt, std::move( owner ) );
  reg().emplace<Cmp::ZOrderValue>( entt, zorder );
  entt_list.push_back( entt );
  SPDLOG_INFO( "Created ShaderSprite {}", static_cast<uint32_t>( entt ) );

  return entt_list;
}

void ShaderSystem::update()
{
  for ( auto [entt, owner] : reg().view<ShaderSpriteOwner>().each() )
  {
    owner.sprite->update( reg() );
  }
}

[[nodiscard]] Sprites::IShaderSprite *ShaderSystem::find( entt::registry &reg, const std::string &tag )
{
  for ( auto [entt, owner] : reg.view<ShaderSpriteOwner>().each() )
  {
    if ( owner.sprite->get_tag() == tag ) return owner.sprite.get();
  }
  return nullptr;
}

} // namespace ProceduralMaze::Sys