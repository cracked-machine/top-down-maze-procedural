#ifndef SRC_SHADERS_ISHADERSPRITE_HPP__
#define SRC_SHADERS_ISHADERSPRITE_HPP__

namespace Game::Sprites
{

class IShaderSprite;

class IShaderSprite : public sf::Drawable, public sf::Transformable
{
public:
  virtual ~IShaderSprite() = default;
  virtual void pre_setup_texture() = 0;
  virtual void post_setup_shader() = 0;
  virtual void update( entt::registry &reg ) = 0;
  virtual void set_tag( const std::string &tag ) = 0;
  [[nodiscard]] virtual std::string get_tag() const = 0;
  virtual void resize_texture( sf::Vector2u new_size ) = 0;
  virtual void active( bool active ) = 0;
  virtual bool active() const = 0;

  //! @brief Access the render texture this sprite's shader will sample as its `texture` uniform.
  //! Used by RenderGameSystem to redirect a frame's drawing into a post-process shader (e.g.
  //! FearDistortionShader) instead of the flat clear color the other overlay shaders use.
  virtual sf::RenderTexture &get_render_texture() = 0;

  //! @brief Whether this shader samples everything drawn so far (rather than blending its own
  //! self-contained texture onto the scene, like the other overlay shaders do). A GPU can't read
  //! and write the same render target in one draw, so RenderGameSystem's z-order loop handles a
  //! post-process shader differently when it reaches its ZOrderValue-driven position in the queue:
  //! it finalizes whatever's been captured into get_render_texture() so far and composites the
  //! distorted result onto the window, instead of just blending this sprite in place like a normal
  //! overlay. See FearDistortionShader for the concrete example.
  [[nodiscard]] virtual bool is_post_process() const { return false; }

protected:
  virtual sf::Shader &get_shader() = 0;
};

} // namespace Game::Sprites

#endif // SRC_SHADERS_ISHADERSPRITE_HPP__
