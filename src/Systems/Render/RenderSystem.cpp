#include <SFML/Graphics/Color.hpp>

#include <Systems/Render/RenderSystem.hpp>

namespace ProceduralMaze::Sys {

RenderSystem::RenderSystem( ProceduralMaze::SharedEnttRegistry reg, sf::RenderWindow &window,
                            Sprites::SpriteFactory &sprite_factory )
    : BaseSystem( reg, window, sprite_factory )
{
  SPDLOG_DEBUG( "RenderSystem constructor called" );
}

std::unordered_map<Sprites::SpriteMetaType, std::reference_wrapper<Sprites::MultiSprite>> RenderSystem::m_multisprite_map;

void RenderSystem::init_multisprites()
{
  for ( auto type : m_sprite_factory.get_all_sprite_types() )
  {
    // const_cast is needed because get_multisprite_by_type returns const&
    // but we need mutable access for rendering operations like pick()
    // Note: Using const_cast to remove const from a reference is safe here because the underlying objects in SpriteFactory are
    // actually mutable. The factory just returns them as const references for encapsulation, but you need mutable access for
    // rendering operations.
    m_multisprite_map.emplace( type,
                               std::ref( const_cast<Sprites::MultiSprite &>( m_sprite_factory.get_multisprite_by_type( type ) ) ) );
  }
}

void RenderSystem::render_text( std::string text, unsigned int size, sf::Vector2f position, Alignment align, float padding,
                                sf::Color fill_color, sf::Color outline_color )
{
  sf::Text title_text( m_font, text, size );
  title_text.setFillColor( fill_color );
  title_text.setOutlineColor( outline_color );
  if ( outline_color != sf::Color::Transparent ) { title_text.setOutlineThickness( 5.f ); }

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
  sf::RectangleShape title_bg;
  sf::FloatRect title_bounds = title_text.getLocalBounds();
  title_bg.setSize( { title_bounds.size.x + padding * 2.f, title_bounds.size.y + padding * 2.f } );
  title_bg.setFillColor( sf::Color::Black );
  title_bg.setPosition(
      { final_position.x + title_bounds.position.x - padding, final_position.y + title_bounds.position.y - padding } );

  m_window.draw( title_bg );
  m_window.draw( title_text );
}

void RenderSystem::safe_render_sprite_to_target( sf::RenderTarget &target, const std::string &sprite_type,
                                                 const sf::FloatRect &pos_cmp, int sprite_index, sf::Vector2f scale, uint8_t alpha,
                                                 sf::Vector2f origin, sf::Angle angle )
{
  if ( not is_visible_in_view( m_window.getView(), pos_cmp ) ) return;
  try
  {
    auto &sprite = m_multisprite_map.at( sprite_type );

    auto pick_result = sprite.get().pick( sprite_index, sprite_type );
    sprite.get().setPosition( pos_cmp.position );
    sprite.get().setScale( scale );
    sprite.get().set_pick_opacity( alpha );
    sprite.get().setOrigin( origin );
    sprite.get().setRotation( angle );
    if ( pick_result ) { target.draw( sprite.get() ); }
    else { render_fallback_square_to_target( target, pos_cmp, sf::Color::Cyan ); }
  }
  catch ( const std::out_of_range &e )
  {
    // SPDLOG_WARN( "Missing sprite '{}' in map, rendering fallback square", sprite_type );
    render_fallback_square_to_target( target, pos_cmp, sf::Color::Magenta );
  }
}

void RenderSystem::render_fallback_square_to_target( sf::RenderTarget &target, const sf::FloatRect &pos_cmp,
                                                     const sf::Color &color )
{
  sf::RectangleShape fallback_square( kGridSquareSizePixelsF );
  fallback_square.setPosition( pos_cmp.position );
  fallback_square.setFillColor( color );
  fallback_square.setOutlineColor( sf::Color::White );
  fallback_square.setOutlineThickness( 1.f );
  target.draw( fallback_square ); // Draw to specified target
}

// Keep the original for backwards compatibility
void RenderSystem::safe_render_sprite( const std::string &sprite_type, const sf::FloatRect &pos_cmp, int sprite_index,
                                       sf::Vector2f scale, uint8_t alpha, sf::Vector2f origin, sf::Angle angle )
{
  safe_render_sprite_to_target( m_window, sprite_type, pos_cmp, sprite_index, scale, alpha, origin, angle );
}

void RenderSystem::render_fallback_square( const sf::FloatRect &pos_cmp, const sf::Color &color )
{
  render_fallback_square_to_target( m_window, pos_cmp, color );
}

sf::View RenderSystem::s_game_view{};

} // namespace ProceduralMaze::Sys
