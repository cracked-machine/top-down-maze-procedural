#include <Shaders/BaseShader.hpp>

namespace ProceduralMaze::Sprites
{

BaseShader::BaseShader( std::filesystem::path vertex_shader_path, std::filesystem::path frag_shader_path, sf::Vector2u texture_size )
    : m_render_texture( texture_size ),
      m_vert_shader_path( vertex_shader_path ),
      m_frag_shader_path( frag_shader_path )
{
}

void BaseShader::setup()
{
  pre_setup_texture();
  m_render_texture.display();
  setup_shader();
  post_setup_shader();
}

void BaseShader::setup_shader()
{

  if ( !std::filesystem::exists( m_vert_shader_path ) )
  {
    SPDLOG_CRITICAL( "Vertex shader file does not exist: {}", m_vert_shader_path.string() );
    throw std::filesystem::filesystem_error( "Vertex Shader file does not exist", m_vert_shader_path,
                                             std::make_error_code( std::errc::no_such_file_or_directory ) );
  }
  if ( !std::filesystem::exists( m_frag_shader_path ) )
  {
    SPDLOG_CRITICAL( "Fragment shader file does not exist: {}", m_frag_shader_path.string() );
    throw std::filesystem::filesystem_error( "Fragment Shader file does not exist", m_frag_shader_path,
                                             std::make_error_code( std::errc::no_such_file_or_directory ) );
  }

  SPDLOG_DEBUG( "Loading shaders from {} and {}", m_vert_shader_path.string(), m_frag_shader_path.string() );
  if ( !m_shader.loadFromFile( m_vert_shader_path.string(), m_frag_shader_path.string() ) )
  {
    SPDLOG_CRITICAL( "Failed to load shaders {} and {}", m_vert_shader_path.string(), m_frag_shader_path.string() );
    throw std::runtime_error( "Failed to load shaders: " + m_vert_shader_path.string() + " and " + m_frag_shader_path.string() );
  }
  SPDLOG_INFO( "Shaders {} loaded successfully", m_vert_shader_path.string() );
}

// set the Sprite position
void BaseShader::set_position( const sf::Vector2f &position ) { m_sprite.setPosition( position ); }

void BaseShader::draw( sf::RenderTarget &target, sf::RenderStates states ) const
{
  states.shader = &m_shader;
  target.draw( m_sprite, states );
}

} // namespace ProceduralMaze::Sprites