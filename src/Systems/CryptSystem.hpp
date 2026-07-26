#ifndef SRC_SYSTEMS_CRYPTSYSTEM_HPP__
#define SRC_SYSTEMS_CRYPTSYSTEM_HPP__

#include <Events/CryptRoomEvent.hpp>
#include <Events/PauseClocksEvent.hpp>
#include <Events/PlayerActionEvent.hpp>
#include <Utils/Crypt.hpp>

#include <Events/ResumeClocksEvent.hpp>
#include <Factory/ObstacleFactory.hpp>
#include <PathFinding/SmartPointers.hpp>
#include <Systems/BaseSystem.hpp>

#include <set>

// clang-format off
namespace Game::Cmp { class CryptRoomEnd; class CryptRoomStart; class Position; }
// clang-format on

namespace Game::Sys
{

class CryptSystem : public Game::Sys::BaseSystem
{
public:
  enum class RoomWallType { INTERIOR = 0, BORDER = 1 };
  CryptSystem( entt::registry &reg, sf::RenderWindow &window, Sprites::SpriteFactory &sprite_factory, Audio::SoundBank &sound_bank,
               entt::dispatcher &scenemanager_event_dispatcher )
      : Game::Sys::BaseSystem( reg, window, sprite_factory, sound_bank ),
        m_scenemanager_event_dispatcher( scenemanager_event_dispatcher )
  {
    // The entt::dispatcher is independent of the registry, so it is safe to bind event handlers in the constructor
    std::ignore = get_systems_event_queue().sink<Events::PlayerActionEvent>().connect<&CryptSystem::on_player_action>( this );
    std::ignore = get_systems_event_queue().sink<Events::CryptRoomEvent>().connect<&CryptSystem::on_room_event>( this );
    std::ignore = get_systems_event_queue().sink<Events::PauseClocksEvent>().connect<&Sys::CryptSystem::on_pause>( this );
    std::ignore = get_systems_event_queue().sink<Events::ResumeClocksEvent>().connect<&Sys::CryptSystem::on_resume>( this );
  }

  //! @brief init the weak pointer for the spatial grid
  //! @param spatial_grid_ptr
  void init( const PathFinding::SpatialHashGridSharedPtr &npc_navmesh, const PathFinding::SpatialHashGridSharedPtr &player_navmesh )
  {
    m_npc_navmesh = npc_navmesh;
    m_player_navmesh = player_navmesh;
  }

  //! @brief Initial maze setup. Called from Scene::on_enter()
  void setup();

  //! @brief Frame update. Called from Scene::do_update()
  void update( sf::Time dt );

  //! @brief Handle player action events
  //! @param event
  void on_player_action( Events::PlayerActionEvent &event );

  //! @brief Handle crypt room events
  //! @param event
  void on_room_event( Events::CryptRoomEvent &event );

  void update_exit_zorder();

  //! @brief Check for collisions with the entrance
  void check_entrance_collision();

  //! @brief Check for collisions with the exit
  void check_exit_collision();

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
  void create_room_borders( const Factory::UUIDEntityMap &uuid_map );

  void on_pause() override {}
  void on_resume() override {}

  //! @brief Shuffle the rooms and passages
  void shuffle_rooms_passages();

  //! @brief Generate the initial Crypt interior walls (fills in Cmp::CryptRoomsClosed)
  //! @note Except for start/end rooms, all other rooms should start as Cmp::CryptRoomsClosed
  void gen_crypt_initial_interior();

  //! @brief Pre-cache the passage connections between the initial closed rooms to mitigate FPS issues during play.
  void cache_all_room_connections();

  //! @brief Create the initial rooms that are closed by default.
  //! @param reg
  //! @param map_grid_size
  void create_initial_closed_rooms( sf::Vector2u map_grid_size );

  //! @brief Generate the main objective for the Crypt
  //! @param map_grid_size
  void create_end_room( sf::Vector2u map_grid_size );

  void reset_maze()
  {
    m_maze_unlocked = false;
    m_enabled_levers = 0;
  }

private:
  void decorate_interior_wall( entt::entity main_entt, Cmp::Position &main_pos_cmp, RoomWallType room_wall_type );

  //! @brief Unlock the objective passage
  void unlock_objective_passage();

  //! @brief Unlock the exit passage
  void unlock_exit_passage();

  //! @brief Change all Cmp::CryptRoomOpen to Cmp::CryptRoomClosed
  //! @note Excludes start/end rooms and the open room occupied by the player)
  void close_open_rooms( const Cmp::Position &player_pos_cmp );

  //! @brief Restores missing Cmp::Obstacles components to Cmp::CryptRoomClosed areas
  void fill_closed_rooms( const Factory::UUIDEntityMap &uuid_map );

  //! @brief Change selected Cmp::CryptRoomClosed to Cmp::CryptRoomOpen.
  //! @param selected_rooms Set of entity ids to open
  //! @note Removes Cmp::Obstacle
  void open_selected_rooms( const std::set<entt::entity> &selected_rooms );

  //! @brief Open all rooms
  void open_all_rooms();

  //! @brief Removes Cmp::Obstacles from Cmp::CryptRoomOpen areas
  void empty_open_rooms( const Factory::UUIDEntityMap &uuid_map );

  std::vector<entt::entity> get_available_room_positions();
  //! @brief Adds Cmp::Lever components to Cmp::CryptRoomOpen areas
  void add_lever_to_open_rooms();
  void add_chest_to_open_rooms( const Cmp::Position &player_pos_cmp );

  //! @brief Add lava pits to open rooms
  void add_lava_pit_open_rooms( const Cmp::Position &player_pos_cmp );
  void do_lava_pit_animation();

  //! @brief Remove lava pits from open rooms
  void remove_lava_pit_open_rooms( const Cmp::Position &player_pos_cmp );

  //! @brief Check player collision with lava pits
  void check_lava_pit_collision();

  //! @brief Activate/deactivate the lava pit if player is close
  void check_lava_pit_activation_by_proximity();

  //! @brief Check player collision with the spike traps
  void check_spike_trap_collision();

  //! @brief Activate/deactivate the spike trap if player is close
  //! @note This uses animation component `m_animation_active` member.
  void check_spike_trap_activation_by_proximity();

  //! @brief Removes Cmp::Lever components from Cmp::CryptRoomOpen areas
  void remove_lever_open_rooms( const Cmp::Position &player_pos_cmp );
  void remove_chest_open_rooms( const Cmp::Position &player_pos_cmp );

  //! @brief Removes ALL levers.
  void remove_all_levers();
  void remove_all_chests();

  //! @brief Spawn NPCs in each open room in the game area
  void spawn_npc_in_open_rooms();

  //! @brief Get the single Cmp::CryptRoomStart component
  //! @return Pair of entt::entity and Cmp::CryptRoomStart&
  std::pair<entt::entity, Cmp::CryptRoomStart &> get_crypt_room_start();

  //! @brief Get the single Cmp::CryptRoomEnd component
  //! @return Pair of entt::entity and Cmp::CryptRoomEnd&
  std::pair<entt::entity, Cmp::CryptRoomEnd &> get_crypt_room_end();

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

  sf::Clock m_priest_fear_clock;

  PathFinding::SpatialHashGridWeakPtr m_npc_navmesh;
  PathFinding::SpatialHashGridWeakPtr m_player_navmesh;
};

} // namespace Game::Sys
#endif // SRC_SYSTEMS_CRYPTSYSTEM_HPP__
