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

} // namespace Stats

class BaseAction
{
public:
  BaseAction( Stats::Health health, Stats::Fear fear, Stats::Despair despair, Stats::Infamy infamy, Stats::Disease disease = Stats::Disease::NONE )
      : m_health( health.value ),
        m_fear( fear.value ),
        m_despair( despair.value ),
        m_infamy( infamy.value ),
        m_disease( disease )
  {
  }
  ~BaseAction() {}

  [[nodiscard]] int health() const { return m_health; }
  [[nodiscard]] int fear() const { return m_fear; }
  [[nodiscard]] int despair() const { return m_despair; }
  [[nodiscard]] int infamy() const { return m_infamy; }
  [[nodiscard]] Stats::Disease disease() const { return m_disease; }

private:
  int m_health{ 0 };
  int m_fear{ 0 };
  int m_despair{ 0 };
  int m_infamy{ 0 };
  Stats::Disease m_disease{ Stats::Disease::NONE };
};

} // namespace ProceduralMaze::Cmp

#endif // SRC_CMPS_STATS_BASEACTION_HPP_