#include <Components/Inventory/InventoryItem.hpp>
#include <Components/Random.hpp>
#include <Components/Stats/BuryAction.hpp>
#include <Components/Stats/CarryAction.hpp>
#include <Components/Stats/ConsumeAction.hpp>
#include <Components/Stats/DestroyAction.hpp>
#include <Components/Stats/ExhumeAction.hpp>
#include <Components/Stats/SacrificeAction.hpp>
#include <Stats/BaseAction.hpp>
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
float BaseStore::tick( const nlohmann::json &item ) { return item.at( "tick" ).get<float>(); }

Cmp::Stats::Disease BaseStore::disease( const nlohmann::json &item )
{
  const auto &disease = item.contains( "disease" ) ? item.at( "disease" ) : item;

  std::string type = disease.at( "type" ).get<std::string>();
  float tick = item.at( "tick" ).get<float>();

  if ( type == "none" ) return { .type = Cmp::Stats::DiseaseType::NONE, .tick = tick };
  if ( type == "leprosy" ) return { .type = Cmp::Stats::DiseaseType::LEPROSY, .tick = tick };
  if ( type == "plague" ) return { .type = Cmp::Stats::DiseaseType::PLAGUE, .tick = tick };
  if ( type == "rabies" ) return { .type = Cmp::Stats::DiseaseType::RABIES, .tick = tick };
  return { .type = Cmp::Stats::DiseaseType::NONE, .tick = 0.f };
}

} // namespace ProceduralMaze::Sys