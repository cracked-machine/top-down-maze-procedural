#ifndef SRC_SYSTEMS_PASSAGESYSTEM_HPP
#define SRC_SYSTEMS_PASSAGESYSTEM_HPP

#include <Components/Crypt/CryptPassageDoor.hpp>
#include <Components/Crypt/CryptRoomStart.hpp>
#include <Components/Direction.hpp>
#include <Components/Random.hpp>
#include <Crypt/CryptPassageBlock.hpp>
#include <Crypt/CryptRoomClosed.hpp>
#include <Crypt/CryptRoomOpen.hpp>
#include <SceneControl/SceneData.hpp>
#include <SpatialHashGrid.hpp>
#include <Systems/BaseSystem.hpp>
#include <Systems/Events/PassageEvent.hpp>
#include <Systems/ProcGen/PassageAlgorithms.hpp>
#include <Systems/ProcGen/PassageCache.hpp>
#include <Utils/Maths.hpp>
#include <Wall.hpp>
#include <cstddef>

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

  void init_scene_data( const Scene::SceneMapSharedPtr &crypt_scene_data ) { m_crypt_scene_data = crypt_scene_data; }

  //! @brief init the weak pointer for the spatial grid
  //! @param pathfinding_navmesh
  void init_nav_mesh( const PathFinding::SpatialHashGridSharedPtr &pathfinding_navmesh )
  {
    m_pathfinding_navmesh = pathfinding_navmesh;
    m_passage_algos.cache_wall_components( reg() );
  }

  void on_passage_event( Events::PassageEvent &event );

  void update( sf::Time dt );

  void remove_passages();
  void open_passages();

  //! @brief Create west, north, east passages for the start room via find_passage_target()
  //! @param start_room_entt The entity ID of the start room
  void connect_start_and_open_rooms_passages( entt::entity start_room_entt );

  //! @brief Create west, north, east and south passages for the occupied room via find_passage_target()
  void connect_occupied_and_open_room_passages();

  //! @brief Create west, north, east and south passages for all open rooms via find_passage_target()
  void cache_all_room_connections();

  void connect_end_room_to_nearest_closed_room();

  //! @brief Create north passage for occupied room to the end room. Calls createDrunkenWalkPassage() directly.
  //! @param end_room_entt The entity ID of the end room
  void connect_occupied_and_end_room_passages( entt::registry &reg, entt::entity end_room_entt, sf::Vector2f map_size_pixel );

  void add_spike_traps();

  virtual void on_pause() override {}
  virtual void on_resume() override {}

private:
  //! @brief Removes all Cmp::CryptPassageBlock entities
  void remove_all_passage_blocks();

  //! @brief Removes Cmp::Obstacles from Cmp::CryptPassageBlock areas
  void empty_open_passages();

  //! @brief Restores missing Cmp::Obstacles components to Cmp::CryptPassageBlock areas
  void fill_all_passages();

  //! @brief Removes any Cmp::CryptPassageBlock components added inside rooms
  //! @param exclude_closed_rooms
  void tidy_passage_blocks( bool exclude_closed_rooms = false );

  void create_uncached_passages();
  void create_cached_passages();

  template <typename ROOMTYPE>
  ProcGen::MidPointDistanceQueue find_room_distances( Cmp::CryptPassageDoor &start_passage_door, const sf::FloatRect search_quadrant,
                                                      std::set<entt::entity> exclude_entts );

  template <typename ROOMTYPE>
  std::vector<Cmp::CryptPassageBlock>
  find_passages( Cmp::CryptPassageDoor &start_passage_door, ProcGen::MidPointDistanceQueue &dist_pqueue, ProcGen::WalkingType walktype,
                 sf::Vector2f map_size_pixel, ProcGen::OnePassagePerTargetRoom passage_limit, ProcGen::AllowDuplicatePassages duplicates_policy );

  //! @brief Used for NPC pathfinding
  PathFinding::SpatialHashGridWeakPtr m_pathfinding_navmesh;

  //! @brief Holds data about the Crypt scene levelgen and static multiblock objects
  Scene::SceneMapWeakPtr m_crypt_scene_data;

  //! @brief index for keeping track of the current `m_cached_passage_list` position.
  size_t m_region_idx{ 0 };

  //! @brief flag for enabling final dynamic scene update via the update function
  bool m_connect_all_rooms{ false };

  ProcGen::PassageCachedRegions<40> m_cached_passage_list;

  //! @brief The list of passage blocks for all paths during dynamic scene updates
  std::vector<Cmp::CryptPassageBlock> m_uncached_passage_list;

  ProcGen::PassageAlogirthms m_passage_algos;
};

extern template std::vector<Cmp::CryptPassageBlock>
PassageSystem::find_passages<Cmp::CryptRoomOpen>( Cmp::CryptPassageDoor &, ProcGen::MidPointDistanceQueue &, ProcGen::WalkingType, sf::Vector2f,
                                                  ProcGen::OnePassagePerTargetRoom, ProcGen::AllowDuplicatePassages );

extern template std::vector<Cmp::CryptPassageBlock>
PassageSystem::find_passages<Cmp::CryptRoomClosed>( Cmp::CryptPassageDoor &, ProcGen::MidPointDistanceQueue &, ProcGen::WalkingType, sf::Vector2f,
                                                    ProcGen::OnePassagePerTargetRoom, ProcGen::AllowDuplicatePassages );

extern template ProcGen::MidPointDistanceQueue PassageSystem::find_room_distances<Cmp::CryptRoomOpen>( Cmp::CryptPassageDoor &, const sf::FloatRect,
                                                                                                       std::set<entt::entity> );
extern template ProcGen::MidPointDistanceQueue PassageSystem::find_room_distances<Cmp::CryptRoomClosed>( Cmp::CryptPassageDoor &, const sf::FloatRect,
                                                                                                         std::set<entt::entity> );

} // namespace ProceduralMaze::Sys

#endif // SRC_SYSTEMS_PASSAGESYSTEM_HPP