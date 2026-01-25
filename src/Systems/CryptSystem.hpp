#ifndef SRC_SYSTEMS_CRYPTSYSTEM_HPP__
#define SRC_SYSTEMS_CRYPTSYSTEM_HPP__

#include <Components/Crypt/CryptRoomEnd.hpp>
#include <Components/Crypt/CryptRoomStart.hpp>

#include <Events/CryptRoomEvent.hpp>
#include <Events/PlayerActionEvent.hpp>

#include <Systems/BaseSystem.hpp>
#include <Utils/Maths.hpp>

#include <queue>
#include <set>

namespace ProceduralMaze::Sys
{

class CryptSystem : public ProceduralMaze::Sys::BaseSystem
{
public:
  using RoomDistanceQueue = std::priority_queue<std::pair<float, sf::Vector2f>, std::vector<std::pair<float, sf::Vector2f>>,
                                                Utils::Maths::DistanceVector2fComparator>;

  CryptSystem( entt::registry &reg, sf::RenderWindow &window, Sprites::SpriteFactory &sprite_factory, Audio::SoundBank &sound_bank,
               entt::dispatcher &scenemanager_event_dispatcher )
      : ProceduralMaze::Sys::BaseSystem( reg, window, sprite_factory, sound_bank ),
        m_scenemanager_event_dispatcher( scenemanager_event_dispatcher )
  {
    // The entt::dispatcher is independent of the registry, so it is safe to bind event handlers in the constructor
    std::ignore = get_systems_event_queue().sink<Events::PlayerActionEvent>().connect<&CryptSystem::on_player_action>( this );
    std::ignore = get_systems_event_queue().sink<Events::CryptRoomEvent>().connect<&CryptSystem::on_room_event>( this );
  }

  void update();

  //! @brief Handle player action events
  //! @param event
  void on_player_action( Events::PlayerActionEvent &event );

  //! @brief Handle crypt room events
  //! @param event
  void on_room_event( Events::CryptRoomEvent &event );

  //! @brief Check for collisions with the entrance
  void check_entrance_collision();

  //! @brief Check for collisions with the exit
  void check_exit_collision();

  //! @brief Spawn the exit room
  //! @param spawn_position
  void spawn_exit( sf::Vector2u spawn_position );

  //! @brief Unlock the crypt door
  void unlock_crypt_door();

  //! @brief Check for objective activation
  //! @param action
  void check_objective_activation( Events::PlayerActionEvent::GameActions action );

  //! @brief Check for lever activation
  //! @param action
  void check_lever_activation();

  void check_chest_activation( Events::PlayerActionEvent::GameActions action );

  //! @brief Restores border Cmp::Obstacles to Cmp::CryptRoomEnd, Cmp::CryptRoomStart and Cmp::CryptRoomOpen areas
  void createRoomBorders();

  virtual void onPause() override {}
  virtual void onResume() override {}

  //! @brief Shuffle the rooms and passages
  void shuffle_rooms_passages();

  void reset_maze()
  {
    m_maze_unlocked = false;
    m_enabled_levers = 0;
  }

private:
  //! @brief Unlock the objective passage
  void unlock_objective_passage();

  //! @brief Unlock the exit passage
  void unlock_exit_passage();

  //! @brief Change all Cmp::CryptRoomOpen to Cmp::CryptRoomClosed
  //! @note Excludes start/end rooms and the open room occupied by the player)
  void closeOpenRooms();

  //! @brief Restores missing Cmp::Obstacles components to Cmp::CryptRoomClosed areas
  void fillClosedRooms();

  //! @brief Change selected Cmp::CryptRoomClosed to Cmp::CryptRoomOpen.
  //! @param selected_rooms Set of entity ids to open
  //! @note Removes Cmp::Obstacle
  void openSelectedRooms( std::set<entt::entity> selected_rooms );

  //! @brief Open all rooms
  void openAllRooms();

  //! @brief Removes Cmp::Obstacles from Cmp::CryptRoomOpen areas
  void emptyOpenRooms();

  std::vector<entt::entity> getAvailableRoomPositions();
  //! @brief Adds Cmp::Lever components to Cmp::CryptRoomOpen areas
  void addLeverToOpenRooms();
  void addChestToOpenRooms();

  //! @brief Add lava pits to open rooms
  void addLavaPitOpenRooms();
  void doLavaPitAnimation();

  //! @brief Remove lava pits from open rooms
  void removeLavaPitOpenRooms();

  //! @brief Check player collision with lava pits
  void checkLavaPitCollision();

  //! @brief Activate/deactivate the lava pit if player is close
  void checkLavaPitActivationByProximity();

  //! @brief Check player collision with the spike traps
  void checkSpikeTrapCollision();

  //! @brief Activate/deactivate the spike trap if player is close
  //! @note This uses animation component `m_animation_active` member.
  void checkSpikeTrapActivationByProximity();

  //! @brief Removes Cmp::Lever components from Cmp::CryptRoomOpen areas
  void removeLeverOpenRooms();
  void removeChestOpenRooms();

  //! @brief Removes ALL levers.
  void removeAllLevers();
  void removeAllChests();

  //! @brief Spawn NPCs in each open room in the game area
  void spawnNpcInOpenRooms();

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

  //! @brief Number of enabled levers
  unsigned int m_enabled_levers{ 0 };

  //! @brief Indicates if the maze was unlocked this cycle
  bool m_maze_unlocked{ false };

  sf::Clock m_lava_effect_cooldown_timer;
  sf::Time m_lava_effect_cooldown_threshold{ sf::seconds( 1.f ) };
};

} // namespace ProceduralMaze::Sys
#endif // SRC_SYSTEMS_CRYPTSYSTEM_HPP__
