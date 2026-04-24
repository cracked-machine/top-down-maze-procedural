#include <Components/Inventory/InventoryItem.hpp>
#include <Components/Random.hpp>
#include <Components/Stats/BuryAction.hpp>
#include <Components/Stats/CarryAction.hpp>
#include <Components/Stats/ConsumeAction.hpp>
#include <Components/Stats/DestroyAction.hpp>
#include <Components/Stats/ExhumeAction.hpp>
#include <Components/Stats/SacrificeAction.hpp>
#include <Systems/Stores/BaseStore.hpp>

#include <fstream>
#include <nlohmann/json.hpp>

namespace ProceduralMaze::Sys
{

BaseStore::BaseStore( entt::registry &reg, sf::RenderWindow &window, Sprites::SpriteFactory &sprite_factory, Audio::SoundBank &sound_bank )
    : BaseSystem( reg, window, sprite_factory, sound_bank )
{
  SPDLOG_DEBUG( "ItemSystem initialized" );
}

nlohmann::json BaseStore::load_json_file( const std::filesystem::path &json_file )
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

int BaseStore::health( const nlohmann::json &item ) { return item.at( "health" ).get<int>(); }
int BaseStore::fear( const nlohmann::json &item ) { return item.at( "fear" ).get<int>(); }
int BaseStore::despair( const nlohmann::json &item ) { return item.at( "despair" ).get<int>(); }
int BaseStore::infamy( const nlohmann::json &item ) { return item.at( "infamy" ).get<int>(); }

} // namespace ProceduralMaze::Sys