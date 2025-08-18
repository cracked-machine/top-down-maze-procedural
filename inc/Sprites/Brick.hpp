#ifndef __SPRITES_BRICK_HPP__
#define __SPRITES_BRICK_HPP__

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/RectangleShape.hpp>

namespace ProceduralMaze::Sprites {

// brick graphic
class Brick : public sf::RectangleShape {
public:

    Brick(sf::Vector2f pos, sf::Color fill_colour = BRICK_FILLCOLOUR, sf::Color line_colour = BRICK_LINECOLOUR) 
    : 
        sf::RectangleShape(sf::Vector2f(WIDTH, HEIGHT))
    {
        setPosition(pos);
        setFillColor(fill_colour);
        setOutlineColor(line_colour);
        setOutlineThickness(LINETHICKNESS);
    };

    // so we can query the attributes without having to 
    // store each brick geometry before they are drawn
    constexpr static const float WIDTH{10};
    constexpr static const float HEIGHT{10};
    constexpr static const sf::Vector2f SIZE{WIDTH, HEIGHT};

    constexpr static const float LINETHICKNESS{0};
    
    constexpr static const float FULLWIDTH{WIDTH + (LINETHICKNESS*2)};
    constexpr static const float FULLHEIGHT{HEIGHT + (LINETHICKNESS*2)};
    constexpr static const sf::Vector2f FULLSIZE{FULLWIDTH, FULLHEIGHT};
    
    constexpr static const float HALFWIDTH{WIDTH/2};
    constexpr static const float HALFHEIGHT{HEIGHT/2};
    
    constexpr static const sf::Color BRICK_FILLCOLOUR{110, 30, 18};
    constexpr static const sf::Color BRICK_LINECOLOUR{69, 28, 20};
    constexpr static const sf::Color BEDROCK_FILLCOLOUR{124,124,124};
    constexpr static const sf::Color BEDROCK_LINECOLOUR{86, 96, 96};
};

} // namespace ProceduralMaze::Sprites

#endif // __SPRITES_BRICK_HPP__