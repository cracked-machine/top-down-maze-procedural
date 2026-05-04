#ifndef SRC_SHADERS_ISHADERSPRITE_HPP_
#define SRC_SHADERS_ISHADERSPRITE_HPP_

namespace ProceduralMaze::Sprites
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

protected:
  virtual sf::Shader &get_shader() = 0;
};

} // namespace ProceduralMaze::Sprites

#endif // SRC_SHADERS_ISHADERSPRITE_HPP_