#ifndef SRC_SYSTEMS_SHADERSYSTEM_HPP__
#define SRC_SYSTEMS_SHADERSYSTEM_HPP__

#include <Components/ZOrderValue.hpp>
#include <Shaders/IShaderSprite.hpp>
#include <Systems/BaseSystem.hpp>
namespace Game::Sys
{

//! @brief  This wraps IShaderSprite so it can be emplaced/retrieved with the Entt registry as a single type.
//!         ShaderSystem::find can retrieve the wrapped IShaderSprite via the specified `tag`
struct ShaderSpriteOwner
{
  //! @brief The owned shader sprite implementation.
  std::unique_ptr<Sprites::IShaderSprite> sprite;

  //! @brief Construct a new Shader Sprite Owner object
  //! @param sprite
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

  //! @brief Calls IShaderSprite::update() function within all added ShaderSpriteBase<T>
  void update();

  //! @brief Find a ShaderSpriteOwner by tag and return a pointer to ShaderSpriteBase, or nullptr if not found
  //! @param reg
  //! @param tag
  //! @return Sprites::IShaderSprite*
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
