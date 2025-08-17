#ifndef __COMPONENTS_RANDOM_HPP__
#define __COMPONENTS_RANDOM_HPP__

#include <random>
#include <base.hpp>

namespace ProceduralMaze::Cmp {

class Random : public Cmp::Base {
public:
    Random(int min, int max) : m_intdist(min, max) { SPDLOG_DEBUG("Random()"); }
    ~Random() { SPDLOG_DEBUG("~Random()"); }
    int gen() { return m_intdist( m_randgen ); }

    static void seed(unsigned long s) 
    {
        m_seed = s;
        m_randgen = std::mt19937(s);
    }
    static unsigned long seed() { return m_seed; }
private:
    static unsigned long m_seed;
    static inline std::mt19937 m_randgen{std::random_device{}()};
    std::uniform_int_distribution<> m_intdist;
};


} // namespace ProceduralMaze::Cmp

#endif // __COMPONENTS_RANDOM_HPP__