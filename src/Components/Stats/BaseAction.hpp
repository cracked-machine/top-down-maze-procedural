#ifndef SRC_CMPS_STATS_BASEACTION_HPP_
#define SRC_CMPS_STATS_BASEACTION_HPP_

namespace ProceduralMaze::Cmp
{
namespace Stats
{

enum class DiseaseType { NONE, RABIES, PLAGUE, LEPROSY };

// clang-format off
struct Health  { int value{ 0 }; };
struct Fear    { int value{ 0 }; };
struct Despair { int value{ 0 }; };
struct Infamy  { int value{ 0 }; };
struct Tick  { float value{ 0 }; };
struct Disease { DiseaseType type{ DiseaseType::NONE }; float tick{0}; };
// clang-format on

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
  BaseAction( Stats::Health health, Stats::Fear fear, Stats::Despair despair, Stats::Infamy infamy, Stats::Tick tick, Stats::Disease disease = {} )
      : m_health( health.value ),
        m_fear( fear.value ),
        m_despair( despair.value ),
        m_infamy( infamy.value ),
        m_disease( disease ),
        m_tick( tick.value )
  {
  }
  ~BaseAction() {}

  [[nodiscard]] int health() const { return m_health; }
  [[nodiscard]] int fear() const { return m_fear; }
  [[nodiscard]] int despair() const { return m_despair; }
  [[nodiscard]] int infamy() const { return m_infamy; }
  [[nodiscard]] float interval() const { return m_tick; }
  [[nodiscard]] Stats::Disease disease() const { return m_disease; }

private:
  int m_health{ 0 };
  int m_fear{ 0 };
  int m_despair{ 0 };
  int m_infamy{ 0 };
  Stats::Disease m_disease{};
  float m_tick{ 0 };
};

} // namespace ProceduralMaze::Cmp

#endif // SRC_CMPS_STATS_BASEACTION_HPP_