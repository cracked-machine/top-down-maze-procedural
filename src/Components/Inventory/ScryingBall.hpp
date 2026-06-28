#ifndef SRC_COMPONENTS_INVENTORY_SCRYINGBALL_HPP__
#define SRC_COMPONENTS_INVENTORY_SCRYINGBALL_HPP__

#include <Components/Random.hpp>
#include <vector>
namespace Game::Cmp
{

struct SeeingStone
{
  enum class Target { NONE, RED, YELLOW, GREEN };
  static Target random_pick( std::vector<Target> excludes )
  {
    std::vector<Target> pool{ Target::GREEN, Target::RED, Target::YELLOW };
    std::erase_if( pool, [&excludes]( Target t ) { return std::ranges::find( excludes.begin(), excludes.end(), t ) != excludes.end(); } );
    if ( pool.empty() ) return Target::NONE;
    Cmp::RandomInt rnd( 0, pool.size() - 1 );
    return pool.at( rnd.gen() );
  }
  bool active{ false };
  Target target{ Target::NONE };
};

} // namespace Game::Cmp

#endif // SRC_COMPONENTS_INVENTORY_SCRYINGBALL_HPP__
