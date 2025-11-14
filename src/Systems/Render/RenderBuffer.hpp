#ifndef SRC_SYSTEMS_RENDER_RENDERBUFFER_HPP_
#define SRC_SYSTEMS_RENDER_RENDERBUFFER_HPP_

#include <SFML/Graphics/RenderTexture.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/VertexArray.hpp>

namespace ProceduralMaze::Sys {

class RenderBuffer : public sf::Drawable, public sf::Transformable
{
public:
  //! @brief Construct a new Render Buffer object - used by RenderSystem::safe_render_sprite_to_target()
  //! @param vertices Pass by value to get a mutable copy
  //! @param texture  Reference to the texture to use. Avoid expensive copy.
  RenderBuffer( sf::VertexArray vertices, const sf::Texture &texture )
      : m_vertices{ vertices },
        m_texture{ texture } {};

  //! @brief Set the alpha component on each vertex in the buffer
  //! @param alpha The alpha value to set (0-255)
  void set_pick_opacity( uint8_t alpha )
  {
    for ( std::size_t idx = 0; idx < m_vertices.getVertexCount(); ++idx )
      m_vertices[idx].color.a = alpha;
  }

  //! @brief Draw the sprite buffer to the target
  //! @param target The render target to draw to
  //! @param states The render states to apply
  void draw( sf::RenderTarget &target, sf::RenderStates states ) const override
  {
    // apply the transform
    states.transform *= getTransform();

    // apply the tileset texture
    states.texture = &m_texture;

    // draw the vertex array
    target.draw( m_vertices, states );
  }

private:
  //! @brief The vertex array containing the sprite's geometry
  //! @note Stored by value to allow mutable operations
  sf::VertexArray m_vertices;

  //! @brief Reference to the texture used by the sprite buffer
  //! @note Stored by reference to avoid expensive copies
  const sf::Texture &m_texture;
};

} // namespace ProceduralMaze::Sys

#endif // SRC_SYSTEMS_RENDER_RENDERBUFFER_HPP_