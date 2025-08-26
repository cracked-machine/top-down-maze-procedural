#ifndef __COMPONENTS_ARMED_HPP__
#define __COMPONENTS_ARMED_HPP__

#include <SFML/System/Clock.hpp>
#include <SFML/System/Time.hpp>
#include <entt/entity/registry.hpp>

namespace ProceduralMaze::Cmp {

class Armed {
public:
    Armed() { m_clock.restart(); }
    sf::Time getElapsedTime() const { return m_clock.getElapsedTime(); }
    sf::Clock m_clock;
private:
};

} // namespace ProceduralMaze::Cmp

#endif // __COMPONENTS_ARMED_HPP__