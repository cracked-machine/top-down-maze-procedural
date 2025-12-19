#ifndef SRC_SYSTEMS_CRYPTSYSTEM_HPP__
#define SRC_SYSTEMS_CRYPTSYSTEM_HPP__

#include <Components/CryptEntrance.hpp>
#include <Components/CryptPassageDoor.hpp>
#include <Components/CryptRoomClosed.hpp>
#include <Components/CryptRoomEnd.hpp>
#include <Components/CryptRoomOpen.hpp>
#include <Components/CryptRoomStart.hpp>
#include <Events/CryptRoomEvent.hpp>
#include <Events/PlayerActionEvent.hpp>
#include <Sprites/TileMap.hpp>
#include <Systems/BaseSystem.hpp>
#include <Utils/Maths.hpp>
#include <Utils/Random.hpp>
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
  void closeAllRooms();
  void openSelectedRooms( std::set<entt::entity> selected_rooms );

  void openRandomPassages();
  void closeAllPassages();
  bool place_passage_block( float x, float y, std::vector<entt::entity> &new_block_list );
  bool createDogLegPassage( std::pair<Cmp::CryptPassageDirection, sf::Vector2f> start, std::pair<Cmp::CryptPassageDirection, sf::Vector2f> end );
  bool createDrunkWalkPassage( sf::Vector2f start, sf::Vector2f end );

private:
  entt::dispatcher &m_scenemanager_event_dispatcher;
  sf::Clock m_door_cooldown_timer;
  float m_door_cooldown_time{ 1.0f }; // 1 second cooldown
  sf::Vector2f m_player_last_known_graveyard_pos{ 0, 0 };
};

} // namespace ProceduralMaze::Sys
#endif // SRC_SYSTEMS_CRYPTSYSTEM_HPP__