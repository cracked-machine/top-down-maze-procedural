#ifndef SRC_SYSTEMS_SHADERSYSTEM_HPP__
#define SRC_SYSTEMS_SHADERSYSTEM_HPP__

#include <Components/ZOrderValue.hpp>
#include <Shaders/IShaderSprite.hpp>
#include <Systems/BaseSystem.hpp>
namespace Game::Sys
{

//! @brief  This wraps SpriteBase<IParticle> so it can be emplaced/retrieved with the Entt registry as a single type.
//!         ParticleSystem::find can retrieve SpriteBase<IParticle> via the specified `tag`
struct ShaderSpriteOwner
{
  std::unique_ptr<Sprites::IShaderSprite> sprite;

  explicit ShaderSpriteOwner( std::unique_ptr<Sprites::IShaderSprite> sprite )
      : sprite( std::move( sprite ) )
  {
  }
};

//! @brief Core system for adding and updating IShaderSprite objects
class ShaderSystem : public BaseSystem
{
public:
  //! @brief Construct a new Shader System object
  //! @param reg
  //! @param window
  //! @param sprite_factory
  //! @param sound_bank
  ShaderSystem( entt::registry &reg, sf::RenderWindow &window, Sprites::SpriteFactory &sprite_factory, Audio::SoundBank &sound_bank );

  //! @brief Wrap and add a shader sprite to the registry.
  //! @param shader
  //! @param z_order
  void add( std::unique_ptr<Sprites::IShaderSprite> shader, Cmp::ZOrderValue z_order )
  {
    add_to_registry( ShaderSpriteOwner( std::move( shader ) ), z_order );
  }

  //! @brief Wrap and add a post-process shader sprite to the registry, deliberately without a
  //! Cmp::ZOrderValue so RenderGameSystem::refresh_z_order_queue skips it: post-process shaders (e.g.
  //! FearDistortionShader) are composited as an explicit final pass over the whole frame rather than
  //! drawn inline as a world/screen entity. Still findable via ShaderSystem::find and still updated
  //! wherever the caller invokes IShaderSprite::update() on it directly.
  //! @param shader
  void add_post_process( std::unique_ptr<Sprites::IShaderSprite> shader )
  {
    auto entt = reg().create();
    reg().emplace<ShaderSpriteOwner>( entt, ShaderSpriteOwner( std::move( shader ) ) );
    SPDLOG_INFO( "Created post-process ShaderSprite {}", static_cast<uint32_t>( entt ) );
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

} // namespace Game::Sys

#endif // SRC_SYSTEMS_SHADERSYSTEM_HPP__
