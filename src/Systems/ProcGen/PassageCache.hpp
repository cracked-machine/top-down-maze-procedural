#ifndef SRC_SYSTEMS_PROCGEN_PASSAGECACHE_HPP__
#define SRC_SYSTEMS_PROCGEN_PASSAGECACHE_HPP__

#include <Components/Crypt/PassageBlock.hpp>
#include <cstddef>

namespace Game::Sys::ProcGen
{

//! @brief Max number of regions to split the game area for optimization with final dynamic scene update
const int kMaxCachedRegions{ 40 };

//! @brief Fixed-size collection of MAXREGIONS passage block regions, used to split the cached
//!        passage block list into spatial buckets for the final dynamic scene update.
//! @tparam MAXREGIONS Maximum number of regions the game area is split into.
template <int MAXREGIONS>
class PassageCachedRegions
{

public:
  //! @brief Construct a new Passage Cached Regions object
  PassageCachedRegions() = default;

  //! @brief Optimization for final dynamic level update by spliting cached passage block lists into regions
  struct BlockRegion
  {
    //! @brief World bounds of this region.
    sf::FloatRect region;

    //! @brief Passage blocks whose position falls within this region.
    std::vector<Cmp::Crypt::PassageBlock> blocklist;
  };

  //! @brief Append a region to the list.
  //! @param br
  void add( BlockRegion br ) { m_list.push_back( br ); }

  //! @brief Get an iterator to the first region.
  auto begin() { return m_list.begin(); }

  //! @brief Get an iterator past the last region.
  auto end() { return m_list.end(); }

  //! @brief Get the number of regions.
  auto size() { return m_list.size(); }

  //! @brief Get the region at the specified index.
  //! @param idx
  const auto &get( size_t idx ) { return m_list.at( idx ); }

  //! @brief Clear all regions.
  void clear() { m_list.clear(); }

private:
  //! @brief  Precalculated list of passage blocks for all paths during final dynamic scene updates.
  //!         Created during scene init.
  std::array<BlockRegion, MAXREGIONS> m_list;
};

} // namespace Game::Sys::ProcGen

#endif // SRC_SYSTEMS_PROCGEN_PASSAGECACHE_HPP__
