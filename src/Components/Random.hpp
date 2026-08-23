#ifndef SRC_COMPONENTS_RANDOM_HPP__
#define SRC_COMPONENTS_RANDOM_HPP__

#include <random>

namespace Game::Cmp
{

//! @brief Non-template base class holds shared static state.
//! @note This allows all template instantiations to share a global seed (if set).
class RandomBase
{
protected:
  //! @brief The shared global seed applied to new Random instances, if set.
  inline static unsigned long s_global_seed;

  //! @brief Whether a global seed has been set via setGlobalSeed().
  inline static bool s_global_seed_set;

  //! @brief Counter added to the global seed for each new instance, so instances don't share identical sequences.
  inline static unsigned long s_instance_counter;

public:
  //! @brief Polymorphic destructor for derived classes
  virtual ~RandomBase() = default;

  //! @brief Set a global seed shared by all Random instantiations, resetting the instance counter.
  //! @param seed The seed value to share globally.
  static void setGlobalSeed( unsigned long seed )
  {
    s_global_seed = seed;
    s_global_seed_set = true;
    s_instance_counter = 0;
  }

  //! @brief Clear the global seed, so new Random instances fall back to random_device seeding.
  static void clearGlobalSeed()
  {
    s_global_seed_set = false;
    s_instance_counter = 0;
  }

  //! @brief Returns whether a global seed is currently set.
  static bool isGlobalSeedSet() { return s_global_seed_set; }
};

//! @brief Component wrapping a seedable uniform random number generator over the range [min, max].
//! @tparam T The numeric type produced (integral or floating point).
template <typename T>
class Random : public RandomBase
{
public:
  //! @brief Construct a generator producing values in the given range.
  //! @note Uses the shared global seed (if set via RandomBase::setGlobalSeed), otherwise seeds from random_device.
  //! @param min Inclusive lower bound of the generated range.
  //! @param max Upper bound of the generated range (inclusive for integral T, exclusive for floating point T).
  Random( T min, T max )
      : m_dist( min, max )
  {
    if constexpr ( not std::is_integral_v<T> )
    {
      if ( min >= max ) throw std::invalid_argument( "Random<float>: min must be strictly less than max" );
    }
    // Use shared global seed if set
    if ( s_global_seed_set ) { m_randgen.seed( s_global_seed + s_instance_counter++ ); }
    // else: already initialized with random_device in member initializer
  }

  //! @brief Returns raw bits - Satisfies UniformRandomBitGenerator — enables use with std::shuffle etc.
  using result_type = typename std::mt19937::result_type;

  //! @brief Generate raw engine output. Satisfies UniformRandomBitGenerator.
  //! @return result_type Raw bits from the underlying engine.
  result_type operator()() { return m_randgen(); }

  //! @brief Minimum value the underlying engine can produce. Satisfies UniformRandomBitGenerator.
  //! @return result_type The engine's minimum output.
  static constexpr result_type min() { return std::mt19937::min(); }

  //! @brief Maximum value the underlying engine can produce. Satisfies UniformRandomBitGenerator.
  //! @return result_type The engine's maximum output.
  static constexpr result_type max() { return std::mt19937::max(); }

  //! @brief Returns a value in [min, max] via the uniform distribution
  T gen() { return m_dist( m_randgen ); }

  //! @brief Set the seed for this RNG instance.
  //! @param s The seed value.
  void seed( unsigned long s ) { m_randgen = std::mt19937( s ); }

private:
  //! @brief The underlying Mersenne Twister engine, default-seeded from random_device.
  std::mt19937 m_randgen{ std::random_device{}() };

  // clang-format off
  //! @brief The uniform distribution over [min, max] - integer or real depending on T.
  typename std::conditional_t<std::is_integral_v<T>,
    std::uniform_int_distribution<T>,
    std::uniform_real_distribution<T>
  > m_dist;
  // clang-format on
};

//! @brief Random number generator component specialized for int.
using RandomInt = Random<int>;

//! @brief Random number generator component specialized for float.
using RandomFloat = Random<float>;

} // namespace Game::Cmp

#endif // SRC_COMPONENTS_RANDOM_HPP__
