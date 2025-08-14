#ifndef __COMPONENTS_RANDOM_HPP__
#define __COMPONENTS_RANDOM_HPP__

#include <random>
#include <base.hpp>

namespace ProceduralMaze::Components {

class Random : public Components::Base {
public:
    Random(int min, int max) : m_intdist(min, max) { SPDLOG_DEBUG("Random()"); }
    ~Random() { SPDLOG_DEBUG("~Random()"); }
    int gen() { return m_intdist( m_randgen ); }

private:
    static inline std::mt19937 m_randgen{std::random_device{}()};
    std::uniform_int_distribution<> m_intdist;
};

} // namespace ProceduralMaze::Components

#endif // __COMPONENTS_RANDOM_HPP__