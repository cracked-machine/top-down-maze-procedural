#ifndef SRC_COMPONENTS_STATS_BASEACTION_HPP__
#define SRC_COMPONENTS_STATS_BASEACTION_HPP__

namespace Game::Cmp
{
namespace Stats
{

enum class DiseaseType { NONE, RABIES, PLAGUE, LEPROSY };

// clang-format off
struct Health   { int value{ 0 }; };
struct Fear     { int value{ 0 }; };
struct Despair  { int value{ 0 }; };
struct Infamy   { int value{ 0 }; };
struct Toxicity { int value{ 0 }; };
struct Tick     { float value{ 0 }; };
struct Disease  { DiseaseType type{ DiseaseType::NONE }; float tick{0}; };
// clang-format on

} // namespace Stats

//! @brief Construct a new Base Action object
//! @param health
//! @param fear
//! @param despair
//! @param infamy
//! @param toxicity
//! @param disease
class BaseAction
{
public:
  BaseAction( Stats::Health health, Stats::Fear fear, Stats::Despair despair, Stats::Infamy infamy, Stats::Toxicity toxicity, Stats::Tick tick,
              Stats::Disease disease = {} )
      : m_health( health.value ),
        m_fear( fear.value ),
        m_despair( despair.value ),
        m_infamy( infamy.value ),
        m_toxicity( toxicity.value ),
        m_disease( disease ),
        m_tick( tick.value )
  {
  }
  ~BaseAction() {}

  [[nodiscard]] int health() const { return m_health; }
  [[nodiscard]] int fear() const { return m_fear; }
  [[nodiscard]] int despair() const { return m_despair; }
  [[nodiscard]] int infamy() const { return m_infamy; }
  [[nodiscard]] int toxicity() const { return m_toxicity; }
  [[nodiscard]] float interval() const { return m_tick; }
  [[nodiscard]] Stats::Disease disease() const { return m_disease; }

  BaseAction &operator+=( const BaseAction &rhs )
  {
    m_health += rhs.m_health;
    m_fear += rhs.m_fear;
    m_despair += rhs.m_despair;
    m_infamy += rhs.m_infamy;
    m_tick += rhs.m_tick;
    return *this;
  }

private:
  int m_health{ 0 };
  int m_fear{ 0 };
  int m_despair{ 0 };
  int m_infamy{ 0 };
  int m_toxicity{ 0 };
  Stats::Disease m_disease{};
  float m_tick{ 0 };
};

} // namespace Game::Cmp

#endif // SRC_COMPONENTS_STATS_BASEACTION_HPP__
