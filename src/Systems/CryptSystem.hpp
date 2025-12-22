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

  //! @brief Close all open rooms (except the one occupied by the player)
  void closeAllRooms();
  void openSelectedRooms( std::set<entt::entity> selected_rooms );
  void openAllRooms();

  bool openRandomStartPassages();
  bool openRandomMiddlePassages();
  void openAllMiddlePassages();
  void openFinalPassage();
  void closeAllPassages();

  void find_passage_target( Cmp::CryptPassageDoor &start_passage_door, const sf::FloatRect search_quadrant, std::set<entt::entity> exclude_entts,
                            OnePassagePerTargetRoom one_passage = OnePassagePerTargetRoom::YES,
                            AllowDuplicatePassages duplicates_policy = AllowDuplicatePassages::NO );
  bool createDogLegPassage( Cmp::CryptPassageDoor start, sf::FloatRect end_bounds,
                            AllowDuplicatePassages duplicates_policy = AllowDuplicatePassages::NO );
  bool createDrunkenWalkPassage( Cmp::CryptPassageDoor start, sf::FloatRect end_bounds, std::set<entt::entity> exclude_entts,
                                 AllowDuplicatePassages duplicates_policy = AllowDuplicatePassages::NO );
  bool place_passage_block( unsigned int passage_id, float x, float y, AllowDuplicatePassages duplicates_policy = AllowDuplicatePassages::NO );

private:
  //! @brief Get the single Cmp::CryptRoomStart component
  //! @return Cmp::CryptRoomStart&
  std::pair<entt::entity, Cmp::CryptRoomStart &> get_crypt_room_start()
  {
    auto start_room_view = getReg().view<Cmp::CryptRoomStart>();
    if ( start_room_view.front() == entt::null ) throw std::runtime_error( "CryptSystem::get_crypt_room_start - Unable to get Cmp::CryptRoomStart" );
    return { start_room_view.front(), getReg().get<Cmp::CryptRoomStart>( start_room_view.front() ) };
  }

  //! @brief Get the single Cmp::CryptRoomEnd component
  //! @return Cmp::CryptRoomEnd&
  std::pair<entt::entity, Cmp::CryptRoomEnd &> get_crypt_room_end()
  {
    auto end_room_view = getReg().view<Cmp::CryptRoomEnd>();
    if ( end_room_view.front() == entt::null ) throw std::runtime_error( "CryptSystem::get_crypt_room_end - Unable to get Cmp::CryptRoomEnd" );
    return { end_room_view.front(), getReg().get<Cmp::CryptRoomEnd>( end_room_view.front() ) };
  }

  entt::dispatcher &m_scenemanager_event_dispatcher;
  sf::Clock m_door_cooldown_timer;
  float m_door_cooldown_time{ 1.0f }; // 1 second cooldown
  sf::Vector2f m_player_last_known_graveyard_pos{ 0, 0 };
  unsigned int m_current_passage_id{ 0 };

  std::unordered_map<Cmp::CryptPassageDirection, Cmp::Direction> m_direction_dictionary = {
      { Cmp::CryptPassageDirection::NORTH, Cmp::Direction( { 0.f, -1.f } ) },
      { Cmp::CryptPassageDirection::EAST, Cmp::Direction( { 1.f, 0.f } ) },
      { Cmp::CryptPassageDirection::SOUTH, Cmp::Direction( { 0.f, 1.f } ) },
      { Cmp::CryptPassageDirection::WEST, Cmp::Direction( { -1.f, 0.f } ) } };

  std::vector<Cmp::Direction> m_direction_choices = { Cmp::Direction( { 0.f, -1.f } ), Cmp::Direction( { 1.f, 0.f } ), Cmp::Direction( { 0.f, 1.f } ),
                                                      Cmp::Direction( { -1.f, 0.f } ) };
};

} // namespace ProceduralMaze::Sys
#endif // SRC_SYSTEMS_CRYPTSYSTEM_HPP__

/// Issues:
// 1. CryptPassageBlock offset/alignment DONE
// 2. Passages going through open rooms?
// 3. Dog leg passages don't extend out from the left/right sides properly DONE
// 4. Try drunken walk
// 5. Remove obstacles for newly opened passages
// 6. restore obstacles for newly closed passages