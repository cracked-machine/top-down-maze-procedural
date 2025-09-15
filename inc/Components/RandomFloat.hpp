#ifndef __COMPONENTS_RANDOM_FLOAT_HPP__
#define __COMPONENTS_RANDOM_FLOAT_HPP__

#include <Base.hpp>
#include <random>

namespace ProceduralMaze::Cmp {

class RandomFloat
{
public:
  // set the rng range
  RandomFloat( float min, float max )
      : m_floatdist( min, max )
  {
  }
  ~RandomFloat() { SPDLOG_DEBUG( "~RandomFloat()" ); }

  // get the next random number
  float gen() { return m_floatdist( m_randgen ); }

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
  std::uniform_real_distribution<> m_floatdist;
};

} // namespace ProceduralMaze::Cmp

#endif // __COMPONENTS_RANDOM_FLOAT_HPP__