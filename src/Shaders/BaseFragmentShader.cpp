#include <Shaders/BaseFragmentShader.hpp>

namespace ProceduralMaze::Sprites
{

BaseFragmentShader::BaseFragmentShader( std::filesystem::path shader_path, sf::Vector2u texture_size )
    : m_render_texture( texture_size ),
      m_shader_path( shader_path )
{
}

void BaseFragmentShader::setup()
{
  pre_setup_texture();
  m_render_texture.display();
  setup_shader();
  post_setup_shader();
}

void BaseFragmentShader::setup_shader()
{
  if ( !std::filesystem::exists( m_shader_path ) )
  {
    SPDLOG_CRITICAL( "Shader file does not exist: {}", m_shader_path.string() );
    throw std::filesystem::filesystem_error( "Shader file does not exist", m_shader_path,
                                             std::make_error_code( std::errc::no_such_file_or_directory ) );
  }
  SPDLOG_DEBUG( "Loading shader from {}", m_shader_path.string() );
  if ( !m_shader.loadFromFile( m_shader_path.string(), sf::Shader::Type::Fragment ) )
  {
    SPDLOG_CRITICAL( "Failed to load shader {}", m_shader_path.string() );
    throw std::runtime_error( "Failed to load shader: " + m_shader_path.string() );
  }
  SPDLOG_INFO( "Shader {} loaded successfully", m_shader_path.string() );
}

// set the Sprite position
void BaseFragmentShader::set_position( const sf::Vector2f &position ) { m_sprite.setPosition( position ); }

void BaseFragmentShader::draw( sf::RenderTarget &target, sf::RenderStates states ) const
{
  states.shader = &m_shader;
  target.draw( m_sprite, states );
}

} // namespace ProceduralMaze::Sprites