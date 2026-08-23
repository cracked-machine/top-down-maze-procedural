#ifndef SRC_COMPONENTS_STATS_BASEACTION_HPP__
#define SRC_COMPONENTS_STATS_BASEACTION_HPP__

namespace Game::Cmp
{
namespace Stats
{

//! @brief Type of disease that an action can inflict on the player.
enum class DiseaseType
{
  //! @brief No disease is applied.
  NONE,
  //! @brief Rabies infection.
  RABIES,
  //! @brief Plague infection.
  PLAGUE,
  //! @brief Leprosy infection.
  LEPROSY
};

//! @brief Strongly-typed health delta, used to disambiguate BaseAction/derived-action constructor arguments.
struct Health
{
  //! @brief The change applied to the health stat.
  int value{ 0 };
};

//! @brief Strongly-typed fear delta, used to disambiguate BaseAction/derived-action constructor arguments.
struct Fear
{
  //! @brief The change applied to the fear stat.
  int value{ 0 };
};

//! @brief Strongly-typed despair delta, used to disambiguate BaseAction/derived-action constructor arguments.
struct Despair
{
  //! @brief The change applied to the despair stat.
  int value{ 0 };
};

//! @brief Strongly-typed infamy delta, used to disambiguate BaseAction/derived-action constructor arguments.
struct Infamy
{
  //! @brief The change applied to the infamy stat.
  int value{ 0 };
};

//! @brief Strongly-typed toxicity delta, used to disambiguate BaseAction/derived-action constructor arguments.
struct Toxicity
{
  //! @brief The change applied to the toxicity stat.
  int value{ 0 };
};

//! @brief Strongly-typed tick interval, used to disambiguate BaseAction/derived-action constructor arguments.
struct Tick
{
  //! @brief How often (seconds) the action re-applies, or 0 for a one-shot.
  float value{ 0 };
};

//! @brief Strongly-typed disease affliction, used to disambiguate BaseAction/derived-action constructor arguments.
struct Disease
{
  //! @brief The disease inflicted.
  DiseaseType type{ DiseaseType::NONE };
  //! @brief The disease's own re-apply interval, in seconds.
  float tick{ 0 };
};

} // namespace Stats

//! @brief Base class for a player-stat modifier applied when a particular kind of gameplay action
//! occurs (e.g. burying, carrying, colliding with, consuming, destroying, or being hit by something).
//! @details Holds the health/fear/despair/infamy/toxicity deltas, an optional disease affliction, and
//! the tick interval at which the modifier re-applies. Derived classes (BuryAction, CarryAction,
//! CollisionAction, ConsumeAction, DestroyAction, ProjectileAction, ProximityAction, SacrificeAction,
//! SpawnAction) add no behaviour of their own - they exist purely so that Cmp::WorldItem::actions and
//! NPC action maps can key modifiers by the std::type_index of the triggering action kind.
class BaseAction
{
public:
  //! @brief Construct a new Base Action object.
  //! @param health Change applied to the health stat.
  //! @param fear Change applied to the fear stat.
  //! @param despair Change applied to the despair stat.
  //! @param infamy Change applied to the infamy stat.
  //! @param toxicity Change applied to the toxicity stat.
  //! @param tick How often (seconds) the action re-applies, or 0 for a one-shot.
  //! @param disease Disease affliction applied alongside the stat changes, if any.
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
  //! @brief Destroy the Base Action object.
  ~BaseAction() {}

  //! @brief Get the health delta.
  //! @return int The change applied to the health stat.
  [[nodiscard]] int health() const { return m_health; }
  //! @brief Get the fear delta.
  //! @return int The change applied to the fear stat.
  [[nodiscard]] int fear() const { return m_fear; }
  //! @brief Get the despair delta.
  //! @return int The change applied to the despair stat.
  [[nodiscard]] int despair() const { return m_despair; }
  //! @brief Get the infamy delta.
  //! @return int The change applied to the infamy stat.
  [[nodiscard]] int infamy() const { return m_infamy; }
  //! @brief Get the toxicity delta.
  //! @return int The change applied to the toxicity stat.
  [[nodiscard]] int toxicity() const { return m_toxicity; }
  //! @brief Get the re-apply interval.
  //! @return float How often (seconds) the action re-applies, or 0 for a one-shot.
  [[nodiscard]] float interval() const { return m_tick; }
  //! @brief Get the disease affliction associated with this action.
  //! @return Stats::Disease The disease type and its own tick interval.
  [[nodiscard]] Stats::Disease disease() const { return m_disease; }

  //! @brief Accumulate another action's stat deltas into this one.
  //! @note Does not accumulate toxicity or disease.
  //! @param rhs The action whose deltas are added to this one.
  //! @return BaseAction& Reference to this action, after accumulation.
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
  //! @brief The change applied to the health stat.
  int m_health{ 0 };
  //! @brief The change applied to the fear stat.
  int m_fear{ 0 };
  //! @brief The change applied to the despair stat.
  int m_despair{ 0 };
  //! @brief The change applied to the infamy stat.
  int m_infamy{ 0 };
  //! @brief The change applied to the toxicity stat.
  int m_toxicity{ 0 };
  //! @brief The disease affliction applied alongside the stat changes, if any.
  Stats::Disease m_disease{};
  //! @brief How often (seconds) the action re-applies, or 0 for a one-shot.
  float m_tick{ 0 };
};

} // namespace Game::Cmp

#endif // SRC_COMPONENTS_STATS_BASEACTION_HPP__
