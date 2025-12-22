#ifndef SRC_SYSTEMS_CRYPTSYSTEM_HPP__
#define SRC_SYSTEMS_CRYPTSYSTEM_HPP__

#include <Components/CryptEntrance.hpp>
#include <Components/CryptPassageDoor.hpp>
#include <Components/CryptRoomClosed.hpp>
#include <Components/CryptRoomEnd.hpp>
#include <Components/CryptRoomOpen.hpp>
#include <Components/CryptRoomStart.hpp>
#include <Components/Direction.hpp>
#include <Events/CryptRoomEvent.hpp>
#include <Events/PlayerActionEvent.hpp>
#include <Sprites/TileMap.hpp>
#include <Systems/BaseSystem.hpp>
#include <Utils/Maths.hpp>
#include <Utils/Random.hpp>
#include <queue>
#include <set>
#include <stdexcept>

namespace ProceduralMaze::Sys
{

class CryptSystem : public ProceduralMaze::Sys::BaseSystem
{
public:
  using RoomDistanceQueue = std::priority_queue<std::pair<float, sf::Vector2f>, std::vector<std::pair<float, sf::Vector2f>>,
                                                Utils::Maths::DistanceVector2fComparator>;

  enum class OnePassagePerTargetRoom { YES, NO };
  enum class AllowDuplicatePassages { YES, NO };

  CryptSystem( entt::registry &reg, sf::RenderWindow &window, Sprites::SpriteFactory &sprite_factory, Audio::SoundBank &sound_bank,
               entt::dispatcher &scenemanager_event_dispatcher )
      : ProceduralMaze::Sys::BaseSystem( reg, window, sprite_factory, sound_bank ),
        m_scenemanager_event_dispatcher( scenemanager_event_dispatcher )
  {
    // The entt::dispatcher is independent of the registry, so it is safe to bind event handlers in the constructor
    std::ignore = get_systems_event_queue().sink<Events::PlayerActionEvent>().connect<&CryptSystem::on_player_action>( this );
    std::ignore = get_systems_event_queue().sink<Events::CryptRoomEvent>().connect<&CryptSystem::on_room_event>( this );
  }

  void on_player_action( Events::PlayerActionEvent &event )
  {
    // Handle player actions related to crypts here if needed
    // For now, this function is a placeholder
    if ( event.action == Events::PlayerActionEvent::GameActions::ACTIVATE ) unlock_crypt_door();
    if ( event.action == Events::PlayerActionEvent::GameActions::ACTIVATE ) check_objective_activation( event.action );
  }

  void on_room_event( Events::CryptRoomEvent &event );

  virtual void onPause() override {}
  virtual void onResume() override {}
  void unlock_crypt_door();
  void check_entrance_collision();
  void check_exit_collision();
  void check_objective_activation( Events::PlayerActionEvent::GameActions action );
  void spawn_exit( sf::Vector2u spawn_position );

  //! @brief Create west, north, east passages for the start room via find_passage_target()
  void connectPassagesBetweenStartAndOpenRooms();

  //! @brief Create west, north, east and south passages for the occupied room via find_passage_target()
  void connectPassagesBetweenOccupiedAndOpenRooms();

  //! @brief Create west, north, east and south passages for all open rooms via find_passage_target()
  void connectPassagesBetweenAllOpenRooms();

  //! @brief Create north passage for occupied room to the end room. Calls createDrunkenWalkPassage() directly.
  void connectPassageBetweenOccupiedAndEndRoom();

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

  //! @brief Change all Cmp::CryptRoomOpen to Cmp::CryptRoomClosed
  //! @note Excludes start/end rooms and the open room occupied by the player)
  void closeOpenRooms();

  void fillClosedRooms();

  //! @brief Change selected Cmp::CryptRoomClosed to Cmp::CryptRoomOpen.
  //! @param selected_rooms Set of entity ids to open
  //! @note Removes Cmp::Obstacle
  void openSelectedRooms( std::set<entt::entity> selected_rooms );

  void emptyOpenRooms();

  //! @brief Removes all Cmp::CryptPassageBlock entities
  void removeAllPassageBlocks();

  void emptyOpenPassages();
  void fillAllPassages();

  void createRoomBorders();

  void tidyPassageBlocks( bool exclude_closed_rooms = false );

private:
  //! @brief Get the single Cmp::CryptRoomStart component
  //! @return Pair of entt::entity and Cmp::CryptRoomStart&
  std::pair<entt::entity, Cmp::CryptRoomStart &> get_crypt_room_start()
  {
    auto start_room_view = getReg().view<Cmp::CryptRoomStart>();
    if ( start_room_view.front() == entt::null ) throw std::runtime_error( "CryptSystem::get_crypt_room_start - Unable to get Cmp::CryptRoomStart" );
    return { start_room_view.front(), getReg().get<Cmp::CryptRoomStart>( start_room_view.front() ) };
  }

  //! @brief Get the single Cmp::CryptRoomEnd component
  //! @return Pair of entt::entity and Cmp::CryptRoomEnd&
  std::pair<entt::entity, Cmp::CryptRoomEnd &> get_crypt_room_end()
  {
    auto end_room_view = getReg().view<Cmp::CryptRoomEnd>();
    if ( end_room_view.front() == entt::null ) throw std::runtime_error( "CryptSystem::get_crypt_room_end - Unable to get Cmp::CryptRoomEnd" );
    return { end_room_view.front(), getReg().get<Cmp::CryptRoomEnd>( end_room_view.front() ) };
  }

  //! @brief Dispatcher reference for scene management events
  entt::dispatcher &m_scenemanager_event_dispatcher;

  //! @brief Prevents rapid external entrance door unlocking
  sf::Clock m_door_cooldown_timer;
  float m_door_cooldown_time{ 1.0f }; // 1 second cooldown

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
};

} // namespace ProceduralMaze::Sys
#endif // SRC_SYSTEMS_CRYPTSYSTEM_HPP__

/// Issues:
// 1. CryptPassageBlock offset/alignment DONE
// 2. Passages going through open rooms? DONE
// 3. Dog leg passages don't extend out from the left/right sides properly DONE
// 4. Try drunken walk DONE
// 5. Remove obstacles for newly opened passages DONE
// 6. restore obstacles for newly closed passages DONE
// 7. fix z-fighting with obstacles
// 8. add sxf for room change
// 9. pick better blocks