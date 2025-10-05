#include <RenderSystem.hpp>
#include <SFML/Graphics/Color.hpp>

namespace ProceduralMaze::Sys {

RenderSystem::RenderSystem( ProceduralMaze::SharedEnttRegistry reg )
    : BaseSystem( reg )
{
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

std::unique_ptr<sf::RenderWindow> RenderSystem::m_window = nullptr;

} // namespace ProceduralMaze::Sys
