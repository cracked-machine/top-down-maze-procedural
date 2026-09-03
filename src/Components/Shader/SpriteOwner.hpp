#ifndef SRC_COMPONENTS_SHADER_SPRITEOWNER_HPP__
#define SRC_COMPONENTS_SHADER_SPRITEOWNER_HPP__

#include <Shaders/IShaderSprite.hpp>

#include <memory>
#include <utility>

namespace Game::Cmp::Shader
{

//! @brief  This wraps IShaderSprite so it can be emplaced/retrieved with the Entt registry as a single type.
//!         ShaderSystem::find can retrieve the wrapped IShaderSprite via the specified `tag`
struct SpriteOwner
{
  //! @brief The owned shader sprite implementation.
  std::unique_ptr<Sprites::IShaderSprite> sprite;

  //! @brief Construct a new Shader Sprite Owner object
  //! @param sprite
  explicit SpriteOwner( std::unique_ptr<Sprites::IShaderSprite> sprite )
      : sprite( std::move( sprite ) )
  {
  }
};

} // namespace Game::Cmp::Shader

#endif // SRC_COMPONENTS_SHADER_SPRITEOWNER_HPP__
