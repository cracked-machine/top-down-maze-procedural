#ifndef SRC_SYSTEMS_PROCGEN_PASSAGEALGO_HPP_
#define SRC_SYSTEMS_PROCGEN_PASSAGEALGO_HPP_

#include <Crypt/CryptPassageBlock.hpp>
#include <Crypt/CryptPassageDoor.hpp>
#include <Direction.hpp>

#include <Systems/ProcGen/PassageCache.hpp>
#include <optional>

namespace ProceduralMaze::Sys::ProcGen
{
enum class AllowDuplicatePassages { YES, NO };
enum class OnePassagePerTargetRoom { YES, NO };
enum class WalkingType { DRUNK, DOGLEG };
enum class CachedOnly { TRUE, FALSE };

using MidPointDistanceQueue = std::priority_queue<std::pair<float, entt::entity>, std::vector<std::pair<float, entt::entity>>,
                                                  std::greater<std::pair<float, entt::entity>>>;

class PassageAlogirthms
{
public:
  PassageAlogirthms() = default;

  //! @brief Create a Drunken Walk Passage between start and end points

  //! @brief Place a passage block at the specified position
  //! @param passage_id The ID of the passage to place
  //! @param x The x-coordinate of the position
  //! @param y The y-coordinate of the position
  //! @param duplicates_policy Whether to allow duplicate passages blocks
  //! @return Cmp::CryptPassageBlock
  std::optional<Cmp::CryptPassageBlock> place_passage_block( entt::registry &reg, float x, float y,

                                                             AllowDuplicatePassages duplicates_policy = AllowDuplicatePassages::NO );

  //! @param start The starting position and direction for the passage
  //! @param end_bounds The bounds of the end point for the passage
  //! @param exclude_entts Entities to exclude from the search, e.g. the occupied room
  //! @param duplicates_policy Whether to allow duplicate passages blocks
  //! @return true if the passage was created successfully
  //! @return false if the passage could not be created
  std::vector<Cmp::CryptPassageBlock> create_drunken_walk( entt::registry &reg, Cmp::CryptPassageDoor start, sf::FloatRect end_bounds,
                                                           sf::Vector2f map_size_pixel, std::set<entt::entity> exclude_entts,
                                                           AllowDuplicatePassages duplicates_policy = AllowDuplicatePassages::NO );

  //! @brief Create a Dog Leg Passage between start and end points
  //! @param start The starting position and direction for the passage
  //! @param end_bounds The bounds of the end point for the passage
  //! @param duplicates_policy Whether to allow duplicate passages blocks
  //! @return true if the passage was created successfully
  //! @return false if the passage could not be created
  std::vector<Cmp::CryptPassageBlock> create_dog_leg( entt::registry &reg, Cmp::CryptPassageDoor start, sf::FloatRect end_bounds,
                                                      AllowDuplicatePassages duplicates_policy = AllowDuplicatePassages::NO );

  void reset() { m_current_passage_id = 0; }
  unsigned int get_current_passage_id() { return m_current_passage_id; }
  void increment_passage_id() { m_current_passage_id++; }

  void cache_wall_components( entt::registry &reg );

private:
  //! @brief Current passage ID for new passages
  unsigned int m_current_passage_id{ 0 };

  //! @brief Precalculated list of wall obstacles (created during scene init)
  std::vector<sf::FloatRect> m_cached_wall_components;
};

} // namespace ProceduralMaze::Sys::ProcGen

#endif // SRC_SYSTEMS_PROCGEN_PASSAGEALGO_HPP_