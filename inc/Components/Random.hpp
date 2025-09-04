#ifndef __COMPONENTS_RANDOM_HPP__
#define __COMPONENTS_RANDOM_HPP__

#include <Base.hpp>
#include <algorithm>
#include <cstddef>
#include <random>

namespace ProceduralMaze::Cmp {

class Random : public Cmp::Base
{
public:
  // set the rng range
  Random( int min, int max ) : m_intdist( min, max ) {}
  ~Random() { SPDLOG_DEBUG( "~Random()" ); }

  // get the next random number
  int gen() { return m_intdist( m_randgen ); }

  // set the seed for random generator
  static void seed( unsigned long s )
  {
    m_seed = s;
    m_randgen = std::mt19937( s );
  }

  // get the seed for random generator
  static unsigned long seed() { return m_seed; }

private:
  static unsigned long m_seed;
  static inline std::mt19937 m_randgen{ std::random_device{}() };
  std::uniform_int_distribution<> m_intdist;
};

} // namespace ProceduralMaze::Cmp

#endif // __COMPONENTS_RANDOM_HPP__