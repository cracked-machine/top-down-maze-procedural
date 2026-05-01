#ifndef SRC_SYSTEMS_SHADERSYSTEM_HPP_
#define SRC_SYSTEMS_SHADERSYSTEM_HPP_

#include <Shaders/IShaderSprite.hpp>
#include <Systems/BaseSystem.hpp>
#include <ZOrderValue.hpp>
namespace ProceduralMaze::Sys
{

//! @brief  This wraps ParticleSpriteBase<IParticle> so it can be emplaced/retrieved with the Entt registry as a single type.
//!         ParticleSystem::find can retrieve ParticleSpriteBase<IParticle> via the specified `tag`
struct ShaderSpriteOwner
{
  std::unique_ptr<Sprites::IShaderSprite> sprite;

  explicit ShaderSpriteOwner( std::unique_ptr<Sprites::IShaderSprite> sprite )
      : sprite( std::move( sprite ) )
  {
  }
};

class ShaderSystem : public BaseSystem
{
public:
  ShaderSystem( entt::registry &reg, sf::RenderWindow &window, Sprites::SpriteFactory &sprite_factory, Audio::SoundBank &sound_bank );

  void add( std::unique_ptr<Sprites::IShaderSprite> shader, Cmp::ZOrderValue z_order )
  {
    add_to_registry( ShaderSpriteOwner( std::move( shader ) ), z_order );
  }

  //! @brief Calls IShaderSprite::update() function within all added ShaderSpriteBase<T>
  void update();

  //! @brief Find a ShaderSpriteOwner by tag and return a pointer to ShaderSpriteBase, or nullptr if not found
  [[nodiscard]] static Sprites::IShaderSprite *find( entt::registry &reg, const std::string &tag );

  //! @brief event handlers for pausing system clocks
  void on_pause() override {};
  //! @brief event handlers for resuming system clocks
  void on_resume() override {};

private:
  //! @brief Add a wrapped shader sprite and zorder component to the registry
  //! @param owner
  //! @param zorder
  //! @return std::vector<entt::entity>
  std::vector<entt::entity> add_to_registry( ShaderSpriteOwner owner, Cmp::ZOrderValue zorder );
};

} // namespace ProceduralMaze::Sys

#endif // SRC_SYSTEMS_SHADERSYSTEM_HPP_