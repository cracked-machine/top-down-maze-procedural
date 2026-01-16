#ifndef SRC_CMP_INVENTORY_SCRYINGBALL_HPP_
#define SRC_CMP_INVENTORY_SCRYINGBALL_HPP_

#include <Components/Random.hpp>
#include <vector>
namespace ProceduralMaze::Cmp
{

struct ScryingBall
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

} // namespace ProceduralMaze::Cmp

#endif // SRC_CMP_INVENTORY_SCRYINGBALL_HPP_