#ifndef SRC_SYSTEMS_PASSAGESYSTEM_HPP
#define SRC_SYSTEMS_PASSAGESYSTEM_HPP

#include <Components/CryptPassageDoor.hpp>
#include <Components/CryptRoomStart.hpp>
#include <Components/Direction.hpp>
#include <Systems/BaseSystem.hpp>
#include <Systems/Events/PassageEvent.hpp>

#include <set>

namespace ProceduralMaze::Sys
{

class PassageSystem : public ProceduralMaze::Sys::BaseSystem
{

public:
  PassageSystem( entt::registry &reg, sf::RenderWindow &window, Sprites::SpriteFactory &sprite_factory, Audio::SoundBank &sound_bank )
      : ProceduralMaze::Sys::BaseSystem( reg, window, sprite_factory, sound_bank )

  {
    std::ignore = get_systems_event_queue().sink<Events::PassageEvent>().connect<&PassageSystem::on_passage_event>( this );
  }

  void on_passage_event( Events::PassageEvent &event );

  void remove_passages();
  void open_passages();

  //! @brief Create west, north, east passages for the start room via find_passage_target()
  //! @param start_room_entt The entity ID of the start room
  void connectPassagesBetweenStartAndOpenRooms( entt::entity start_room_entt );

  //! @brief Create west, north, east and south passages for the occupied room via find_passage_target()
  void connectPassagesBetweenOccupiedAndOpenRooms();

  //! @brief Create west, north, east and south passages for all open rooms via find_passage_target()
  void connectPassagesBetweenAllOpenRooms();

  //! @brief Create north passage for occupied room to the end room. Calls createDrunkenWalkPassage() directly.
  //! @param end_room_entt The entity ID of the end room
  void connectPassageBetweenOccupiedAndEndRoom( entt::entity end_room_entt );

  virtual void onPause() override {}
  virtual void onResume() override {}

private:
  enum class OnePassagePerTargetRoom { YES, NO };
  enum class AllowDuplicatePassages { YES, NO };

  //! @brief Searches quadrant for nearest open room (Cmp::CryptRoomOpen) and creates a passage to it
  //! @param start_passage_door The starting position and direction for the passage
  //! @param search_quadrant The area to search for target room
  //! @param exclude_entts Entities to exclude from the search, e.g. the occupied room
  //! @param one_passage Whether to create only one passage per target room, default: YES
  //! @param duplicates_policy Whether to allow duplicate passages blocks, default: NO
  void find_passage_target( Cmp::CryptPassageDoor &start_passage_door, const sf::FloatRect search_quadrant, std::set<entt::entity> exclude_entts,
                            OnePassagePerTargetRoom one_passage = OnePassagePerTargetRoom::YES,
                            AllowDuplicatePassages duplicates_policy = AllowDuplicatePassages::NO );

  //! @brief Create a Dog Leg Passage between start and end points
  //! @param start The starting position and direction for the passage
  //! @param end_bounds The bounds of the end point for the passage
  //! @param duplicates_policy Whether to allow duplicate passages blocks
  //! @return true if the passage was created successfully
  //! @return false if the passage could not be created
  bool createDogLegPassage( Cmp::CryptPassageDoor start, sf::FloatRect end_bounds,
                            AllowDuplicatePassages duplicates_policy = AllowDuplicatePassages::NO );

  //! @brief Create a Drunken Walk Passage between start and end points
  //! @param start The starting position and direction for the passage
  //! @param end_bounds The bounds of the end point for the passage
  //! @param exclude_entts Entities to exclude from the search, e.g. the occupied room
  //! @param duplicates_policy Whether to allow duplicate passages blocks
  //! @return true if the passage was created successfully
  //! @return false if the passage could not be created
  bool createDrunkenWalkPassage( Cmp::CryptPassageDoor start, sf::FloatRect end_bounds, std::set<entt::entity> exclude_entts,
                                 AllowDuplicatePassages duplicates_policy = AllowDuplicatePassages::NO );

  //! @brief Place a passage block at the specified position
  //! @param passage_id The ID of the passage to place
  //! @param x The x-coordinate of the position
  //! @param y The y-coordinate of the position
  //! @param duplicates_policy Whether to allow duplicate passages blocks
  //! @return true if the passage block was placed successfully
  //! @return false if the passage block could not be placed
  bool place_passage_block( unsigned int passage_id, float x, float y, AllowDuplicatePassages duplicates_policy = AllowDuplicatePassages::NO );

  //! @brief Removes all Cmp::CryptPassageBlock entities
  void removeAllPassageBlocks();

  //! @brief Removes Cmp::Obstacles from Cmp::CryptPassageBlock areas
  void emptyOpenPassages();

  //! @brief Restores missing Cmp::Obstacles components to Cmp::CryptPassageBlock areas
  void fillAllPassages();

  //! @brief Removes any Cmp::CryptPassageBlock components added inside rooms
  //! @param exclude_closed_rooms
  void tidyPassageBlocks( bool exclude_closed_rooms = false );

  //! @brief Current passage ID for new passages
  unsigned int m_current_passage_id{ 0 };

  //! @brief direction vector quick lookup for passage creation
  std::unordered_map<Cmp::CryptPassageDirection, Cmp::Direction> m_direction_dictionary = {
      { Cmp::CryptPassageDirection::NORTH, Cmp::Direction( { 0.f, -1.f } ) },
      { Cmp::CryptPassageDirection::EAST, Cmp::Direction( { 1.f, 0.f } ) },
      { Cmp::CryptPassageDirection::SOUTH, Cmp::Direction( { 0.f, 1.f } ) },
      { Cmp::CryptPassageDirection::WEST, Cmp::Direction( { -1.f, 0.f } ) } };

  //! @brief Direction random pool for drunken walk passage creation
  std::vector<Cmp::Direction> m_direction_choices = { Cmp::Direction( { 0.f, -1.f } ), Cmp::Direction( { 1.f, 0.f } ), Cmp::Direction( { 0.f, 1.f } ),
                                                      Cmp::Direction( { -1.f, 0.f } ) };

  //! @brief Drunken walk roulette picker for direction
  //! @note undefined odds are used to select a random direction
  Cmp::RandomInt m_direction_picker{ 0, 99 };
  //! @brief Drunken walk roulette odds for moving towards target: 60%
  const float m_roulette_target_bias_odds = 0.6f;
  //! @brief Drunken walk roulette odds for continuing in the same direction
  const float m_roulette_same_direction_odds = 0.1f;

  //! @brief Prevent infinite walks
  const int kMaxStepsPerWalk{ 100 };
  //! @brief Prevent drunken walk from getting stuck
  const int kMaxAttemptsPerStep{ 200 };
  //! @brief Minimum passage-to-passage distance
  const float kMinBlockDistanceBetweenPassages{ Constants::kGridSquareSizePixelsF.x * 2.0f };
  //! @brief Force drunken walk to initially stay in orthogonal direction e.g. north, east, west, south from starting point
  const int kMinInitialOrthogonalSteps{ 3 };
  //! @brief Minimum passage-to-room distance: scale factor of 16x16 block
  const float kMinPassageRoomsDistanceScaleFactor{ 2.f };
  //! @brief Number of steps to delay 'kMinPassageRoomsDistanceScaleFactor' use
  const int kMinPassageRoomsDistanceDelay{ 3 };
};

} // namespace ProceduralMaze::Sys

#endif // SRC_SYSTEMS_PASSAGESYSTEM_HPP