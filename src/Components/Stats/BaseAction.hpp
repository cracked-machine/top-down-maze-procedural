#ifndef SRC_CMPS_STATS_BASEACTION_HPP_
#define SRC_CMPS_STATS_BASEACTION_HPP_

namespace ProceduralMaze::Cmp
{
namespace Stats
{
// clang-format off
struct Health  { int value{ 0 }; };
struct Fear    { int value{ 0 }; };
struct Despair { int value{ 0 }; };
struct Infamy  { int value{ 0 }; };
// clang-format on

enum class Disease { NONE, RABIES, PLAGUE, LEPROSY };
enum class Tick { ONCE, SLOW, FAST };

} // namespace Stats

//! @brief Construct a new Base Action object
//! @param health
//! @param fear
//! @param despair
//! @param infamy
//! @param disease
class BaseAction
{
public:
  BaseAction( Stats::Health health, Stats::Fear fear, Stats::Despair despair, Stats::Infamy infamy, Stats::Disease disease = Stats::Disease::NONE,
              Stats::Tick tick = Stats::Tick::ONCE )
      : m_health( health.value ),
        m_fear( fear.value ),
        m_despair( despair.value ),
        m_infamy( infamy.value ),
        m_disease( disease ),
        m_tick( tick )
  {
  }
  ~BaseAction() {}

  [[nodiscard]] int health() const { return m_health; }
  [[nodiscard]] int fear() const { return m_fear; }
  [[nodiscard]] int despair() const { return m_despair; }
  [[nodiscard]] int infamy() const { return m_infamy; }
  [[nodiscard]] Stats::Disease disease() const { return m_disease; }
  [[nodiscard]] Stats::Tick tick() const { return m_tick; }

private:
  int m_health{ 0 };
  int m_fear{ 0 };
  int m_despair{ 0 };
  int m_infamy{ 0 };
  Stats::Disease m_disease{ Stats::Disease::NONE };
  Stats::Tick m_tick{ Stats::Tick::ONCE };
};

} // namespace ProceduralMaze::Cmp

#endif // SRC_CMPS_STATS_BASEACTION_HPP_