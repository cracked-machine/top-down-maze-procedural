#ifndef __COMPONENTS_RANDOM_COORD_HPP__
#define __COMPONENTS_RANDOM_COORD_HPP__

#include <SFML/System/Vector2.hpp>
#include <Components/Random.hpp>

namespace ProceduralMaze::Cmp {

class RandomCoord {
public:
    RandomCoord(const sf::Vector2f& coords) 
    : 
        m_x(0, coords.x), 
        m_y(0, coords.y) 
    { }
    ~RandomCoord() {}
    sf::Vector2f gen() { 
        return { 
            static_cast<float>(m_x.gen()), 
            static_cast<float>(m_y.gen()) 
        }; 
    }
private:
    Random m_x;
    Random m_y;
};

} // namespace ProceduralMaze::Cmp

#endif // __COMPONENTS_RANDOM_COORD_HPP__