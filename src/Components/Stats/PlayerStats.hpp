#ifndef SRC_CMPS_STATS_PLAYERSTATS_HPP_
#define SRC_CMPS_STATS_PLAYERSTATS_HPP_

#include <Stats/BaseAction.hpp>
namespace ProceduralMaze::Cmp
{

class PlayerStats
{
public:
  //! @brief Construct a new Player Stats object
  //! @param health
  //! @param fear
  //! @param despair
  //! @param infamy
  //! @param disease
  PlayerStats( Stats::Health health, Stats::Fear fear, Stats::Despair despair, Stats::Infamy infamy, Stats::Disease disease = Stats::Disease::NONE )
      : m_health( std::clamp( health.value, 0, 100 ) ),
        m_fear( std::clamp( fear.value, 0, 100 ) ),
        m_despair( std::clamp( despair.value, 0, 100 ) ),
        m_infamy( std::clamp( infamy.value, 0, 100 ) ),
        m_disease( disease )
  {
  }
  ~PlayerStats() {}

  [[nodiscard]] int health() const { return m_health; }
  [[nodiscard]] int fear() const { return m_fear; }
  [[nodiscard]] int despair() const { return m_despair; }
  [[nodiscard]] int infamy() const { return m_infamy; }
  [[nodiscard]] Stats::Disease disease() const { return m_disease; }

  //! @brief Update the player stats with the BaseAction object
  //! @note  BaseAction: health, fear, despair, infamy, disease
  //! @param action
  void apply_modifiers( const BaseAction &action )
  {
    m_health = std::clamp( m_health + action.health(), 0, 100 );
    SPDLOG_INFO( "Player health = {}", m_health );
    m_fear = std::clamp( m_fear + action.fear(), 0, 100 );
    m_despair = std::clamp( m_despair + action.despair(), 0, 100 );
    m_infamy = std::clamp( m_infamy + action.infamy(), 0, 100 );
    if ( m_disease != Stats::Disease::NONE ) { m_disease = action.disease(); }
  }

private:
  int m_health{ 0 };
  int m_fear{ 0 };
  int m_despair{ 0 };
  int m_infamy{ 0 };
  Stats::Disease m_disease{ Stats::Disease::NONE };
};

} // namespace ProceduralMaze::Cmp

#endif // SRC_CMPS_STATS_PLAYERSTATS_HPP_