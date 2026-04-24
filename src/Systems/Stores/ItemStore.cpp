#include <Stats/BuryAction.hpp>
#include <Stats/CarryAction.hpp>
#include <Stats/ConsumeAction.hpp>
#include <Stats/DestroyAction.hpp>
#include <Stats/ExhumeAction.hpp>
#include <Stats/SacrificeAction.hpp>
#include <Systems/Stores/BaseStore.hpp>
#include <Systems/Stores/ItemStore.hpp>

#include <nlohmann/json.hpp>

namespace ProceduralMaze::Sys
{

ItemStore::ItemStore( entt::registry &reg, sf::RenderWindow &window, Sprites::SpriteFactory &sprite_factory, Audio::SoundBank &sound_bank )
    : StoreSingleton<ItemStore, Cmp::InventoryItem>( reg, window, sprite_factory, sound_bank )
{
  s_instance = this;
  m_json_file_path = "res/json/items.json";
  init_store();
  SPDLOG_DEBUG( "ItemStore initialized" );
}

void ItemStore::init_store()
{
  nlohmann::json json = load_json_file( m_json_file_path );
  for ( const auto &[item_key, item_value] : json.items() )
  {
    Sprites::SpriteMetaType sprite_mtype = item_value.at( "sprite" ).get<std::string>();
    Cmp::InventoryItem carryitem( sprite_mtype );
    for ( const auto &action_entry : item_value.at( "actions" ) )
    {
      for ( const auto &[action_key, action_value] : action_entry.items() )
      {
        if ( action_key == "bury_action" )
        {
          carryitem.action_fx_map.emplace( typeid( Cmp::BuryAction ), Cmp::BuryAction( { health( action_value ) }, { fear( action_value ) },
                                                                                       { despair( action_value ) }, { infamy( action_value ) } ) );
        }
        else if ( action_key == "carry_action" )
        {
          carryitem.action_fx_map.emplace( typeid( Cmp::CarryAction ), Cmp::CarryAction( { health( action_value ) }, { fear( action_value ) },
                                                                                         { despair( action_value ) }, { infamy( action_value ) } ) );
        }
        else if ( action_key == "consume_action" )
        {
          carryitem.action_fx_map.emplace(
              typeid( Cmp::ConsumeAction ),
              Cmp::ConsumeAction( { health( action_value ) }, { fear( action_value ) }, { despair( action_value ) }, { infamy( action_value ) } ) );
        }
        else if ( action_key == "destroy_action" )
        {
          carryitem.action_fx_map.emplace(
              typeid( Cmp::DestroyAction ),
              Cmp::DestroyAction( { health( action_value ) }, { fear( action_value ) }, { despair( action_value ) }, { infamy( action_value ) } ) );
        }
        else if ( action_key == "exhume_action" )
        {
          carryitem.action_fx_map.emplace(
              typeid( Cmp::ExhumeAction ),
              Cmp::ExhumeAction( { health( action_value ) }, { fear( action_value ) }, { despair( action_value ) }, { infamy( action_value ) } ) );
        }
        else if ( action_key == "sacrifice_action" )
        {
          carryitem.action_fx_map.emplace(
              typeid( Cmp::SacrificeAction ),
              Cmp::SacrificeAction( { health( action_value ) }, { fear( action_value ) }, { despair( action_value ) }, { infamy( action_value ) } ) );
        }
        else { SPDLOG_WARN( "Unknown action key: {}", action_key ); }
      }
    }
    m_store.emplace( item_key, std::move( carryitem ) );
    SPDLOG_INFO( "Loaded item: {} ({})", item_key, sprite_mtype );
  }
  SPDLOG_INFO( "Item store loaded with {} items", m_store.size() );
}

} // namespace ProceduralMaze::Sys