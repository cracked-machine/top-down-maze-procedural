#include <Components/Stats/BuryAction.hpp>
#include <Components/Stats/CarryAction.hpp>
#include <Components/Stats/ConsumeAction.hpp>
#include <Components/Stats/DestroyAction.hpp>
#include <Components/Stats/ExhumeAction.hpp>
#include <Components/Stats/SacrificeAction.hpp>

#include <Inventory/InventoryItem.hpp>
#include <Random.hpp>
#include <Systems/ItemSystem.hpp>
#include <fstream>
#include <nlohmann/json.hpp>

namespace ProceduralMaze::Sys
{

ItemSystem *ItemSystem::s_instance = nullptr;

ItemSystem::ItemSystem( entt::registry &reg, sf::RenderWindow &window, Sprites::SpriteFactory &sprite_factory, Audio::SoundBank &sound_bank )
    : BaseSystem( reg, window, sprite_factory, sound_bank )
{
  s_instance = this;
  SPDLOG_DEBUG( "ItemSystem initialized" );
}

nlohmann::json ItemSystem::load_json_file( const std::filesystem::path &json_file )
{
  if ( not std::filesystem::exists( json_file ) )
  {
    SPDLOG_ERROR( "JSON file does not exist: {}", json_file.string() );
    throw std::runtime_error( "JSON file not found: " + json_file.string() );
  }

  std::ifstream fs( json_file );
  if ( not fs.is_open() )
  {
    SPDLOG_ERROR( "Unable to open JSON file: {}", json_file.string() );
    throw std::runtime_error( "Cannot open JSON file: " + json_file.string() );
  }

  nlohmann::json json;
  fs >> json;
  return json;
}

int ItemSystem::health( const nlohmann::json &item ) { return item.at( "health" ).get<int>(); }
int ItemSystem::fear( const nlohmann::json &item ) { return item.at( "fear" ).get<int>(); }
int ItemSystem::despair( const nlohmann::json &item ) { return item.at( "despair" ).get<int>(); }
int ItemSystem::infamy( const nlohmann::json &item ) { return item.at( "infamy" ).get<int>(); }

void ItemSystem::init_item_store()
{
  nlohmann::json json = load_json_file( m_item_json_file_path );
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
    m_item_store.emplace( item_key, std::move( carryitem ) );
    SPDLOG_INFO( "Loaded item: {} ({})", item_key, sprite_mtype );
  }
  SPDLOG_INFO( "Item store loaded with {} items", m_item_store.size() );
}

[[nodiscard]] Cmp::InventoryItem ItemSystem::get_item( const std::string &item_key ) const
{
  auto it = m_item_store.find( item_key );
  if ( it == m_item_store.end() )
  {
    SPDLOG_ERROR( "Item not found in store: {}", item_key );
    throw std::runtime_error( "Unknown item key: " + item_key );
  }
  return it->second; // returns a copy
}

[[nodiscard]] std::vector<std::string> ItemSystem::get_all_item_keys() const
{
  std::vector<std::string> keys;
  keys.reserve( m_item_store.size() );
  for ( const auto &[key, _] : m_item_store )
  {
    keys.push_back( key );
  }
  return keys;
}

[[nodiscard]] std::string ItemSystem::get_random_item_from_list( std::vector<std::string> list ) const
{
  if ( list.empty() ) { throw std::runtime_error( "provided list is empty" ); }
  Cmp::RandomInt picker( 0, static_cast<int>( list.size() ) - 1 );
  return list.at( picker.gen() );
}

} // namespace ProceduralMaze::Sys