#include <RenderSystem.hpp>
#include <SFML/Graphics/Color.hpp>

namespace ProceduralMaze::Sys {

RenderSystem::RenderSystem( ProceduralMaze::SharedEnttRegistry reg )
    : BaseSystem( reg )
{
}

std::unordered_map<Sprites::SpriteMetaType, std::optional<Sprites::MultiSprite>> RenderSystem::m_multisprite_map;

void RenderSystem::init_multisprites()
{
  using namespace Sprites;
  auto &factory = get_persistent_component<std::shared_ptr<SpriteFactory>>();
  for ( auto type : factory->get_all_sprite_types() )
  {
    m_multisprite_map[type] = factory->get_multisprite_by_type( type );
    if ( !m_multisprite_map[type] )
    {
      SPDLOG_CRITICAL( "Unable to get {} from SpriteFactory", type );
      std::terminate();
    }
  }
}

void RenderSystem::render_text( std::string text, unsigned int size, sf::Vector2f position, Alignment align,
                                float padding, sf::Color fill_color, sf::Color outline_color )
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

  getWindow().draw( title_bg );
  getWindow().draw( title_text );
}

void RenderSystem::safe_render_sprite_to_target( sf::RenderTarget &target, const std::string &sprite_type,
                                                 const sf::FloatRect &pos_cmp, int sprite_index, sf::Vector2f scale,
                                                 uint8_t alpha, sf::Vector2f origin, sf::Angle angle )
{
  if ( not is_visible_in_view( getWindow().getView(), pos_cmp ) ) return;
  try
  {

    auto &sprite = m_multisprite_map.at( sprite_type );
    if ( sprite.has_value() )
    {
      auto pick_result = sprite->pick( sprite_index, sprite_type );
      sprite->setPosition( pos_cmp.position );
      sprite->setScale( scale );
      sprite->set_pick_opacity( alpha );
      sprite->setOrigin( origin );
      sprite->setRotation( angle );
      if ( pick_result )
      {
        target.draw( *sprite ); // Draw to specified target instead of getWindow()
      }
      // SPDLOG_INFO( "Failed to pick sprite '{}' with index {}", sprite_type, sprite_index );
      else { render_fallback_square_to_target( target, pos_cmp, sf::Color::Cyan ); }
    }
    else
    {
      // SPDLOG_WARN( "Sprite '{}' exists in map but has no value", sprite_type );
      render_fallback_square_to_target( target, pos_cmp, sf::Color::Yellow );
    }
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
  safe_render_sprite_to_target( getWindow(), sprite_type, pos_cmp, sprite_index, scale, alpha, origin, angle );
}

void RenderSystem::render_fallback_square( const sf::FloatRect &pos_cmp, const sf::Color &color )
{
  render_fallback_square_to_target( getWindow(), pos_cmp, color );
}

std::unique_ptr<sf::RenderWindow> RenderSystem::m_window = nullptr;

sf::View RenderSystem::s_game_view{};

} // namespace ProceduralMaze::Sys
