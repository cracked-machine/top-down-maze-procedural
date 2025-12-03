#include <SFML/Graphics/Color.hpp>
#include <Systems/Render/RenderBuffer.hpp>
#include <Systems/Render/RenderSystem.hpp>

namespace ProceduralMaze::Sys
{

RenderSystem::RenderSystem( entt::registry &reg, sf::RenderWindow &window, Sprites::SpriteFactory &sprite_factory, Audio::SoundBank &sound_bank )
    : BaseSystem( reg, window, sprite_factory, sound_bank )
{
  SPDLOG_DEBUG( "RenderSystem constructor called" );
}

void RenderSystem::render_text( std::string text, unsigned int size, sf::Vector2f position, Alignment align, float letter_spacing,
                                sf::Color fill_color, sf::Color outline_color )
{
  sf::Text title_text( m_font, text, size );
  title_text.setFillColor( fill_color );
  title_text.setOutlineColor( outline_color );
  if ( outline_color != sf::Color::Transparent ) { title_text.setOutlineThickness( 5.f ); }
  title_text.setLetterSpacing( letter_spacing );

  // if requested center aligned then we ignore the user provided x position
  sf::Vector2f final_position = position;
  if ( align == Alignment::CENTER )
  {
    sf::FloatRect text_bounds = title_text.getLocalBounds();
    final_position.x = kDisplaySize.x * 0.5f;
    final_position.x -= text_bounds.size.x * 0.5f;
  }

  title_text.setPosition( final_position );

  // Create a black background rectangle for the title text
  float cell_padding = 20;
  sf::RectangleShape title_bg;
  sf::FloatRect title_bounds = title_text.getLocalBounds();
  title_bg.setSize( { title_bounds.size.x + cell_padding * 2.f, title_bounds.size.y + cell_padding * 2.f } );
  title_bg.setFillColor( sf::Color::Black );
  title_bg.setPosition( { final_position.x + title_bounds.position.x - cell_padding, final_position.y + title_bounds.position.y - cell_padding } );

  m_window.draw( title_bg );
  m_window.draw( title_text );
}

void RenderSystem::safe_render_sprite_to_target( sf::RenderTarget &target, const std::string &sprite_type, const sf::FloatRect &pos_cmp,
                                                 std::size_t sprite_index, sf::Vector2f scale, uint8_t alpha, sf::Vector2f origin, sf::Angle angle )
{

  try
  {
    auto &sprite = m_sprite_factory.get_multisprite_by_type( sprite_type );

    if ( sprite_index < sprite.get_sprite_count() )
    {
      //! @brief Load the const& MultiSprite into a mutable RenderBuffer.
      //! @note The vertex array (item from 'm_va_list') is a copy, this means we can modify the geometry without affecting the
      //! global MultiSprite geometry. For performance reasons the global MultiSprite texture must be copied by reference, but
      //! in order to protect the global MultiSprite texture from accidental side effects it must also be a const reference,
      //! i.e. read-only.
      const auto &readonly_texture = sprite.get_texture();
      Sys::RenderBuffer sprite_buffer( sprite.m_va_list[sprite_index], readonly_texture );

      // Adjust position to compensate for origin offset when origin != (0,0)
      sf::Vector2f adjusted_position = pos_cmp.position;
      if ( origin != sf::Vector2f( 0.f, 0.f ) ) { adjusted_position += origin; }
      sprite_buffer.setPosition( adjusted_position );

      sprite_buffer.setScale( scale );
      sprite_buffer.set_pick_opacity( alpha );
      sprite_buffer.setOrigin( origin );
      sprite_buffer.setRotation( angle );
      target.draw( sprite_buffer );
    }
    else { render_fallback_square_to_target( target, pos_cmp, sf::Color::Cyan ); }
  }
  catch ( const std::out_of_range &e )
  {
    // SPDLOG_WARN( "Missing sprite '{}' in map, rendering fallback square", sprite_type );
    render_fallback_square_to_target( target, pos_cmp, sf::Color::Magenta );
  }
}

void RenderSystem::render_fallback_square_to_target( sf::RenderTarget &target, const sf::FloatRect &pos_cmp, const sf::Color &color )
{
  sf::RectangleShape fallback_square( kGridSquareSizePixelsF );
  fallback_square.setPosition( pos_cmp.position );
  fallback_square.setFillColor( color );
  fallback_square.setOutlineColor( sf::Color::White );
  fallback_square.setOutlineThickness( 1.f );
  target.draw( fallback_square ); // Draw to specified target
}

// Keep the original for backwards compatibility
void RenderSystem::safe_render_sprite( const std::string &sprite_type, const sf::FloatRect &pos_cmp, std::size_t sprite_index, sf::Vector2f scale,
                                       uint8_t alpha, sf::Vector2f origin, sf::Angle angle )
{
  safe_render_sprite_to_target( m_window, sprite_type, pos_cmp, sprite_index, scale, alpha, origin, angle );
}

void RenderSystem::render_fallback_square( const sf::FloatRect &pos_cmp, const sf::Color &color )
{
  render_fallback_square_to_target( m_window, pos_cmp, color );
}

sf::View RenderSystem::s_game_view{};

} // namespace ProceduralMaze::Sys
