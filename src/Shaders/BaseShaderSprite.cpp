#include <Shaders/BaseShaderSprite.hpp>
#include <utility>

namespace ProceduralMaze::Sprites
{

BaseShaderSprite::BaseShaderSprite( std::filesystem::path vertex_shader_path, std::filesystem::path frag_shader_path, sf::Vector2u texture_size )
    : m_render_texture( texture_size ),
      m_vert_shader_path( std::move( vertex_shader_path ) ),
      m_frag_shader_path( std::move( frag_shader_path ) )
{
}

void BaseShaderSprite::setup()
{
  pre_setup_texture();
  m_render_texture.display();
  load_shader_files();
  post_setup_shader();
}

void BaseShaderSprite::load_shader_files()
{
  if ( not std::filesystem::exists( m_vert_shader_path ) )
  {
    throw std::filesystem::filesystem_error( "Vertex Shader file does not exist", m_vert_shader_path,
                                             std::make_error_code( std::errc::no_such_file_or_directory ) );
  }
  if ( not std::filesystem::exists( m_frag_shader_path ) )
  {
    throw std::filesystem::filesystem_error( "Fragment Shader file does not exist", m_frag_shader_path,
                                             std::make_error_code( std::errc::no_such_file_or_directory ) );
  }

  if ( not m_shader.loadFromFile( m_vert_shader_path.string(), m_frag_shader_path.string() ) )
  {
    throw std::runtime_error( "Failed to load shaders: " + m_vert_shader_path.string() + " and " + m_frag_shader_path.string() );
  }
  SPDLOG_INFO( "Shaders {} loaded successfully", m_vert_shader_path.string() );
}

void BaseShaderSprite::set_position( const sf::Vector2f &position ) { m_sprite.setPosition( position ); }
void BaseShaderSprite::setPosition( const sf::Vector2f &position ) { set_position( position ); }
void BaseShaderSprite::set_texture_view( sf::View view_update ) { m_render_texture.setView( view_update ); }
void BaseShaderSprite::resize_texture( sf::Vector2u new_size )
{
  [[maybe_unused]] auto result = m_render_texture.resize( new_size );
  SPDLOG_INFO( "Resized render texture to {}x{}, result: {}", new_size.x, new_size.y, result ? "Success" : "Failed" );
  m_sprite.setTexture( m_render_texture.getTexture(), true );
  m_sprite.setTextureRect( sf::IntRect( { 0, 0 }, { static_cast<int>( new_size.x ), static_cast<int>( new_size.y ) } ) );
}

void BaseShaderSprite::set_center_at_position( sf::Vector2f pos )
{
  sf::Vector2f new_position = { pos.x - ( static_cast<float>( get_texture_size().x ) / 2.f ),
                                pos.y - ( static_cast<float>( get_texture_size().y ) / 2.f ) };
  set_position( new_position );
}

void BaseShaderSprite::draw( sf::RenderTarget &target, sf::RenderStates states ) const
{
  states.shader = &m_shader;
  target.draw( m_sprite, states );
}

//

} // namespace ProceduralMaze::Sprites