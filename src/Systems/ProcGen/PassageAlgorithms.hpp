#ifndef SRC_SYSTEMS_PROCGEN_PASSAGEALGORITHMS_HPP__
#define SRC_SYSTEMS_PROCGEN_PASSAGEALGORITHMS_HPP__

#include <Components/Crypt/PassageBlock.hpp>
#include <Components/Crypt/PassageDoor.hpp>
#include <Components/Direction.hpp>

#include <Systems/ProcGen/PassageCache.hpp>
#include <optional>

namespace Game::Sys::ProcGen
{
enum class AllowDuplicatePassages { YES, NO };
enum class OnePassagePerTargetRoom { YES, NO };
enum class WalkingType { DRUNK, DOGLEG };
enum class CachedOnly { TRUE, FALSE };

using MidPointDistanceQueue = std::priority_queue<std::pair<float, entt::entity>, std::vector<std::pair<float, entt::entity>>,
                                                  std::greater<std::pair<float, entt::entity>>>;

//! @brief Algorithms for carving passage blocks between crypt rooms: placing individual passage
//!        blocks and walking/plotting full passages between a starting door and a target room.
class PassageAlogirthms
{
public:
  //! @brief Construct a new Passage Alogirthms object
  PassageAlogirthms() = default;

  //! @brief Create a Drunken Walk Passage between start and end points

  //! @brief Place a passage block at the specified position
  //! @param passage_id The ID of the passage to place
  //! @param x The x-coordinate of the position
  //! @param y The y-coordinate of the position
  //! @param duplicates_policy Whether to allow duplicate passages blocks
  //! @return Cmp::Crypt::PassageBlock
  std::optional<Cmp::Crypt::PassageBlock> place_passage_block( entt::registry &reg, float x, float y,
                                                             AllowDuplicatePassages duplicates_policy = AllowDuplicatePassages::NO,
                                                             bool skip_wall_check = false );

  //! @brief Create a random, wandering "drunken walk" passage of blocks from the start door towards
  //!        the end bounds, biased to move towards the target while avoiding walls, rooms and other passages.
  //! @param start The starting position and direction for the passage
  //! @param end_bounds The bounds of the end point for the passage
  //! @param exclude_entts Entities to exclude from the search, e.g. the occupied room
  //! @param duplicates_policy Whether to allow duplicate passages blocks
  //! @return true if the passage was created successfully
  //! @return false if the passage could not be created
  std::vector<Cmp::Crypt::PassageBlock> create_drunken_walk( entt::registry &reg, Cmp::Crypt::PassageDoor start, sf::FloatRect end_bounds,
                                                           sf::Vector2f map_size_pixel, const std::set<entt::entity> &exclude_entts,
                                                           AllowDuplicatePassages duplicates_policy = AllowDuplicatePassages::NO );

  //! @brief Create a Dog Leg Passage between start and end points
  //! @param start The starting position and direction for the passage
  //! @param end_bounds The bounds of the end point for the passage
  //! @param duplicates_policy Whether to allow duplicate passages blocks
  //! @return true if the passage was created successfully
  //! @return false if the passage could not be created
  std::vector<Cmp::Crypt::PassageBlock> create_dog_leg( entt::registry &reg, Cmp::Crypt::PassageDoor start, sf::FloatRect end_bounds,
                                                      AllowDuplicatePassages duplicates_policy = AllowDuplicatePassages::NO );

  //! @brief Reset the current passage ID counter back to zero.
  void reset() { m_current_passage_id = 0; }

  //! @brief Get the current passage ID.
  //! @return unsigned int
  [[nodiscard]] unsigned int get_current_passage_id() const { return m_current_passage_id; }

  //! @brief Increment the current passage ID, starting a new logical passage.
  void increment_passage_id() { m_current_passage_id++; }

  //! @brief Precalculate and cache all wall obstacle bounds so passage carving can check for
  //!        wall collisions without repeatedly querying the registry.
  //! @param reg
  void cache_wall_components( entt::registry &reg );

private:
  //! @brief Current passage ID for new passages
  unsigned int m_current_passage_id{ 0 };

  //! @brief Precalculated list of wall obstacles (created during scene init)
  std::vector<sf::FloatRect> m_cached_wall_components;
};

} // namespace Game::Sys::ProcGen

#endif // SRC_SYSTEMS_PROCGEN_PASSAGEALGORITHMS_HPP__
