#ifndef SRC_SYSTEMS_PROCGEN_PASSAGECACHE_HPP_
#define SRC_SYSTEMS_PROCGEN_PASSAGECACHE_HPP_

#include <Crypt/CryptPassageBlock.hpp>
#include <cstddef>
namespace ProceduralMaze::Sys::ProcGen
{

// Requirements:
// 1. push back new BlockRegion
// 3. get BlockRegion
//

//! @brief Max number of regions to split the game area for optimization with final dynamic scene update
const int kMaxCachedRegions{ 40 };

template <int MAXREGIONS>
class PassageCachedRegions
{

public:
  PassageCachedRegions() = default;

  //! @brief Optimization for final dynamic level update by spliting cached passage block lists into regions
  struct BlockRegion
  {
    //! @brief
    sf::FloatRect region;
    //! @brief
    std::vector<Cmp::CryptPassageBlock> blocklist;
  };

  void add( BlockRegion br ) { m_list.push_back( br ); }
  auto begin() { return m_list.begin(); }
  auto end() { return m_list.end(); }
  auto size() { return m_list.size(); }
  const auto &get( size_t idx ) { return m_list.at( idx ); }
  void clear() { m_list.clear(); }

private:
  //! @brief  Precalculated list of passage blocks for all paths during final dynamic scene updates.
  //!         Created during scene init.
  std::array<BlockRegion, MAXREGIONS> m_list;
};

} // namespace ProceduralMaze::Sys::ProcGen

#endif // SRC_SYSTEMS_PROCGEN_PASSAGECACHE_HPP_