#ifndef __COMPONENTS_RANDOM_HPP__
#define __COMPONENTS_RANDOM_HPP__

#include <random>

namespace ProceduralMaze::Cmp {

// Non-template base class holds shared static state
// This allows all template instantiations to share a global seed (if set).
class RandomBase
{
protected:
  inline static unsigned long s_global_seed;
  inline static bool s_global_seed_set;
  inline static unsigned long s_instance_counter;

public:
  static void setGlobalSeed( unsigned long seed )
  {
    s_global_seed = seed;
    s_global_seed_set = true;
    s_instance_counter = 0;
  }

  static void clearGlobalSeed()
  {
    s_global_seed_set = false;
    s_instance_counter = 0;
  }

  static bool isGlobalSeedSet() { return s_global_seed_set; }
};

template <typename T> class Random : public RandomBase
{
public:
  // set the rng range
  Random( T min, T max )
      : m_dist( min, max )
  {
    // Use shared global seed if set
    if ( s_global_seed_set ) { m_randgen.seed( s_global_seed + s_instance_counter++ ); }
    // else: already initialized with random_device in member initializer
  }

  // get the next random number
  T gen()
  {
    if constexpr ( std::is_integral_v<T> ) { return m_dist( m_randgen ); }
    else { return m_dist( m_randgen ); }
  }

  // set the seed for this RNG instance
  void seed( unsigned long s ) { m_randgen = std::mt19937( s ); }

private:
  std::mt19937 m_randgen{ std::random_device{}() };

  // clang-format off
  typename std::conditional_t<std::is_integral_v<T>, 
    std::uniform_int_distribution<T>, 
    std::uniform_real_distribution<T>
  > m_dist;
  // clang-format on
};

using RandomInt = Random<int>;
using RandomFloat = Random<float>;

} // namespace ProceduralMaze::Cmp

#endif // __COMPONENTS_RANDOM_HPP__