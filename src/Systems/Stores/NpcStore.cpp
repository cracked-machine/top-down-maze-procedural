#include <Stats/BuryAction.hpp>
#include <Stats/CarryAction.hpp>
#include <Stats/CollisionAction.hpp>
#include <Stats/ConsumeAction.hpp>
#include <Stats/DestroyAction.hpp>
#include <Stats/ExhumeAction.hpp>
#include <Stats/ProjectileAction.hpp>
#include <Stats/SacrificeAction.hpp>
#include <Systems/Stores/NpcStore.hpp>

#include <nlohmann/json.hpp>

namespace ProceduralMaze::Sys
{

NpcStore::NpcStore( entt::registry &reg, sf::RenderWindow &window, Sprites::SpriteFactory &sprite_factory, Audio::SoundBank &sound_bank )
    : StoreSingleton<NpcStore, Cmp::NPC>( reg, window, sprite_factory, sound_bank )
{
  s_instance = this;
  m_json_file_path = "res/json/npc.json";
  init_store();
  SPDLOG_DEBUG( "NpcStore initialized" );
}

void NpcStore::init_store()
{
  nlohmann::json json = load_json_file( m_json_file_path );
  for ( const auto &[item_key, item_value] : json.items() )
  {
    std::vector<Sprites::SpriteMetaType> mtype_list;
    for ( const auto &sprite_json : item_value.at( "sprites" ) )
    {
      mtype_list.push_back( sprite_json.get<std::string>() );
    }

    Cmp::NPC npc( mtype_list );
    for ( const auto &action_entry : item_value.at( "actions" ) )
    {
      for ( const auto &[action_key, action_value] : action_entry.items() )
      {
        if ( action_key == "bury_action" )
        {
          npc.actions.emplace( typeid( Cmp::BuryAction ),
                               Cmp::BuryAction( { health( action_value ) }, { fear( action_value ) }, { despair( action_value ) },
                                                { infamy( action_value ) }, { tick( action_value ) }, disease( action_value ) ) );
        }
        else if ( action_key == "carry_action" )
        {
          npc.actions.emplace( typeid( Cmp::CarryAction ),
                               Cmp::CarryAction( { health( action_value ) }, { fear( action_value ) }, { despair( action_value ) },
                                                 { infamy( action_value ) }, { tick( action_value ) }, disease( action_value ) ) );
        }
        else if ( action_key == "consume_action" )
        {
          npc.actions.emplace( typeid( Cmp::ConsumeAction ),
                               Cmp::ConsumeAction( { health( action_value ) }, { fear( action_value ) }, { despair( action_value ) },
                                                   { infamy( action_value ) }, { tick( action_value ) }, disease( action_value ) ) );
        }
        else if ( action_key == "destroy_action" )
        {
          npc.actions.emplace( typeid( Cmp::DestroyAction ),
                               Cmp::DestroyAction( { health( action_value ) }, { fear( action_value ) }, { despair( action_value ) },
                                                   { infamy( action_value ) }, { tick( action_value ) }, disease( action_value ) ) );
        }
        else if ( action_key == "exhume_action" )
        {
          npc.actions.emplace( typeid( Cmp::ExhumeAction ),
                               Cmp::ExhumeAction( { health( action_value ) }, { fear( action_value ) }, { despair( action_value ) },
                                                  { infamy( action_value ) }, { tick( action_value ) }, disease( action_value ) ) );
        }
        else if ( action_key == "sacrifice_action" )
        {
          npc.actions.emplace( typeid( Cmp::SacrificeAction ),
                               Cmp::SacrificeAction( { health( action_value ) }, { fear( action_value ) }, { despair( action_value ) },
                                                     { infamy( action_value ) }, { tick( action_value ) }, disease( action_value ) ) );
        }
        else if ( action_key == "projectile_action" )
        {
          npc.actions.emplace( typeid( Cmp::ProjectileAction ),
                               Cmp::ProjectileAction( { health( action_value ) }, { fear( action_value ) }, { despair( action_value ) },
                                                      { infamy( action_value ) }, { tick( action_value ) }, disease( action_value ) ) );
        }
        else if ( action_key == "collision_action" )
        {
          npc.actions.emplace( typeid( Cmp::CollisionAction ),
                               Cmp::CollisionAction( { health( action_value ) }, { fear( action_value ) }, { despair( action_value ) },
                                                     { infamy( action_value ) }, { tick( action_value ) }, disease( action_value ) ) );
        }
        else { SPDLOG_WARN( "Unknown action key: {}", action_key ); }
      }
    }
    m_store.emplace( item_key, std::move( npc ) );
    SPDLOG_INFO( "Loaded item: {}", item_key );
  }
  SPDLOG_INFO( "Item store loaded with {} items", m_store.size() );
}

} // namespace ProceduralMaze::Sys