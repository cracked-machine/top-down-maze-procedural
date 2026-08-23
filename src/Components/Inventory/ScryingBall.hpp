#ifndef SRC_COMPONENTS_INVENTORY_SCRYINGBALL_HPP__
#define SRC_COMPONENTS_INVENTORY_SCRYINGBALL_HPP__

#include <Components/Random.hpp>
#include <vector>
namespace Game::Cmp
{

//! @brief A scrying ball item. When active, the Render system draws a colored "dogleg" guide line from
//! the holder to the nearest landmark matching `target` (an altar, crypt entrance, or exit).
struct SeeingStone
{
  //! @brief The landmark a scrying ball guides towards, also used as the color of its guide line.
  enum class Target
  {
    //! @brief No landmark - no guide line is drawn.
    NONE,
    //! @brief Guides towards crypt entrances, drawn in red.
    RED,
    //! @brief Guides towards altars, drawn in yellow.
    YELLOW,
    //! @brief Guides towards exits, drawn in green.
    GREEN
  };

  //! @brief Pick a random Target not present in `excludes`, so that multiple scrying balls in play
  //! don't guide towards the same kind of landmark.
  //! @param excludes Targets to exclude from the pick.
  //! @return Target A randomly chosen Target from the remaining pool, or Target::NONE if none remain.
  static Target random_pick( std::vector<Target> excludes )
  {
    std::vector<Target> pool{ Target::GREEN, Target::RED, Target::YELLOW };
    std::erase_if( pool, [&excludes]( Target t ) { return std::ranges::find( excludes.begin(), excludes.end(), t ) != excludes.end(); } );
    if ( pool.empty() ) return Target::NONE;
    Cmp::RandomInt rnd( 0, pool.size() - 1 );
    return pool.at( rnd.gen() );
  }

  //! @brief Whether the guide line should currently be rendered.
  bool active{ false };
  //! @brief The landmark this scrying ball guides towards.
  Target target{ Target::NONE };
};

} // namespace Game::Cmp

#endif // SRC_COMPONENTS_INVENTORY_SCRYINGBALL_HPP__
