#include <Persistent/ArmedOffDelay.hpp>
#include <Persistent/ArmedOnDelay.hpp>
#include <Persistent/BlastRadius.hpp>
#include <Persistent/BombBonus.hpp>
#include <Persistent/BombDamage.hpp>
#include <Persistent/BombInventory.hpp>
#include <Persistent/CorruptionDamage.hpp>
#include <Persistent/DiggingCooldownThreshold.hpp>
#include <Persistent/DiggingDamagePerHit .hpp>
#include <Persistent/FloodSpeed.hpp>
#include <Persistent/FuseDelay.hpp>
#include <Persistent/HealthBonus.hpp>
#include <Persistent/MaxShrines.hpp>
#include <Persistent/MusicVolume.hpp>
#include <Persistent/NpcActivateScale.hpp>
#include <Persistent/NpcAnimFramerate.hpp>
#include <Persistent/NpcDamage.hpp>
#include <Persistent/NpcDamageDelay.hpp>
#include <Persistent/NpcDeathAnimFramerate.hpp>
#include <Persistent/NpcLerpSpeed.hpp>
#include <Persistent/NpcPushBack.hpp>
#include <Persistent/NpcScanScale.hpp>
#include <Persistent/ObstaclePushBack.hpp>
#include <Persistent/PlayerAnimFramerate.hpp>
#include <Persistent/PlayerDetectionScale.hpp>
#include <Persistent/PlayerDiagonalLerpSpeedModifier.hpp>
#include <Persistent/PlayerLerpSpeed.hpp>
#include <Persistent/PlayerShortcutLerpSpeedModifier.hpp>
#include <Persistent/PlayerStartPosition.hpp>
#include <Persistent/PlayerSubmergedlLerpSpeedModifier.hpp>
#include <Persistent/ShrineCost.hpp>
#include <Persistent/WaterBonus.hpp>
#include <Persistent/WormholeAnimFramerate.hpp>
#include <PersistentSystem.hpp>
#include <fstream>
#include <functional>
#include <unordered_map>

#define JSON_NOEXCEPTION
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

#include <Persistent/GameState.hpp>

namespace ProceduralMaze::Sys {

PersistentSystem::PersistentSystem( SharedEnttRegistry reg )
    : BaseSystem( reg )
{
  SPDLOG_INFO( "PersistentSystem constructor called" ); // Add this debug line
  // not added from json file, always present
  add_persistent_component<Cmp::Persistent::GameState>();

  std::ignore = getEventDispatcher()
                    .sink<Events::SaveSettingsEvent>()
                    .connect<&Sys::PersistentSystem::on_save_settings_event>( this );

  initializeComponentRegistry();
}

void PersistentSystem::initializeComponentRegistry()
{
  // Register persistent component with their default values.
  // These should be overidden by modifying the json file directly or via in-game settings
  registerComponent<Cmp::Persistent::ArmedOffDelay>( "ArmedOffDelay", 0.075f );
  registerComponent<Cmp::Persistent::ArmedOnDelay>( "ArmedOnDelay", 0.025f );
  registerComponent<Cmp::Persistent::FuseDelay>( "FuseDelay", 3.f );
  registerComponent<Cmp::Persistent::PlayerAnimFramerate>( "PlayerAnimFramerate", 0.1f );
  registerComponent<Cmp::Persistent::PlayerDetectionScale>( "PlayerDetectionScale", 7.0f );
  registerComponent<Cmp::Persistent::PlayerLerpSpeed>( "PlayerLerpSpeed", 4.f );
  registerComponent<Cmp::Persistent::PlayerDiagonalLerpSpeedModifier>( "PlayerDiagonalLerpSpeedModifier", 0.707f );
  registerComponent<Cmp::Persistent::PlayerShortcutLerpSpeedModifier>( "PlayerShortcutLerpSpeedModifier", 0.3f );
  registerComponent<Cmp::Persistent::PlayerSubmergedLerpSpeedModifier>( "PlayerSubmergedLerpSpeedModifier", 0.3f );
  registerComponent<Cmp::Persistent::ObstaclePushBack>( "ObstaclePushBack", 1.1f );
  registerComponent<Cmp::Persistent::NpcAnimFramerate>( "NpcAnimFramerate", 0.1f );
  registerComponent<Cmp::Persistent::NpcActivateScale>( "NpcActivateScale", 5.f );
  registerComponent<Cmp::Persistent::NpcDamageDelay>( "NpcDamageDelay", 0.5f );
  registerComponent<Cmp::Persistent::FloodSpeed>( "FloodSpeed", 1.f );
  registerComponent<Cmp::Persistent::MusicVolume>( "MusicVolume", 100.0f );
  registerComponent<Cmp::Persistent::NpcScanScale>( "NpcScanScale", 2.5f );
  registerComponent<Cmp::Persistent::NpcLerpSpeed>( "NpcLerpSpeed", 1.f );
  registerComponent<Cmp::Persistent::WormholeAnimFramerate>( "WormholeAnimFramerate", 0.5f );
  registerComponent<Cmp::Persistent::NpcDeathAnimFramerate>( "NpcDeathAnimFramerate", 0.1f );
  registerComponent<Cmp::Persistent::DiggingCooldownThreshold>( "DiggingCooldownThreshold", 1.0f );
  registerComponent<Cmp::Persistent::DiggingDamagePerHit>( "DiggingDamagePerHit", 1.0f );
  registerComponent<Cmp::Persistent::WaterBonus>( "WaterBonus", 100.0f );
  registerComponent<Cmp::Persistent::BombDamage>( "BombDamage", 10 );
  registerComponent<Cmp::Persistent::BombInventory>( "BombInventory", 10 );
  registerComponent<Cmp::Persistent::BlastRadius>( "BlastRadius", 1 );
  registerComponent<Cmp::Persistent::HealthBonus>( "HealthBonus", 10 );
  registerComponent<Cmp::Persistent::BombBonus>( "BombBonus", 5 );
  registerComponent<Cmp::Persistent::NpcDamage>( "NpcDamage", 10 );
  registerComponent<Cmp::Persistent::CorruptionDamage>( "CorruptionDamage", 1 );
  registerComponent<Cmp::Persistent::NpcPushBack>( "NpcPushBack", 16.f );
  registerComponent<Cmp::Persistent::MaxShrines>( "MaxShrines", 3u );
  registerComponent<Cmp::Persistent::ShrineCost>( "ShrineCost", 2u );

  // Register special types (sf::Vector2f)
  auto default_player_start_pos = sf::Vector2f( Sys::BaseSystem::kGridSquareSizePixels.x * 5,
                                                static_cast<float>( Sys::BaseSystem::kDisplaySize.y ) / 2 );
  registerComponent<Cmp::Persistent::PlayerStartPosition>( "PlayerStartPosition", default_player_start_pos );
}

void PersistentSystem::load_state()
{
  SPDLOG_INFO( "Loading persistent state..." );
  nlohmann::json jsonData;
  std::ifstream inputFile( "res/json/persistent_components.json" );
  if ( inputFile.is_open() )
  {
    inputFile >> jsonData;
    inputFile.close();
  }

  for ( const auto &[key, value] : jsonData.items() )
  {
    if ( m_component_loaders.contains( key ) ) { m_component_loaders.at( key )( value ); }
    else { SPDLOG_WARN( "Unknown component: {}", key ); }
  }
}

void PersistentSystem::save_state()
{
  SPDLOG_INFO( "Saving persistent state..." );
  nlohmann::json jsonData;

  // Helper lambda to serialize a component if it exists
  auto serializeComponent = [&]<typename ComponentType>( const std::string &key ) {
    try
    {
      auto &component = get_persistent_component<ComponentType>();
      jsonData[key] = component.serialize();
    }
    catch ( const std::exception &e )
    {
      SPDLOG_WARN( "Failed to serialize component {}: {}", key, e.what() );
    }
  };

  // Serialize all registered components
  // clang-format off
  serializeComponent.template operator()<Cmp::Persistent::ArmedOffDelay>( "ArmedOffDelay" );
  serializeComponent.template operator()<Cmp::Persistent::ArmedOnDelay>( "ArmedOnDelay" );
  serializeComponent.template operator()<Cmp::Persistent::BombDamage>( "BombDamage" );
  serializeComponent.template operator()<Cmp::Persistent::BombInventory>( "BombInventory" );
  serializeComponent.template operator()<Cmp::Persistent::BlastRadius>( "BlastRadius" );
  serializeComponent.template operator()<Cmp::Persistent::BombBonus>( "BombBonus" );
  serializeComponent.template operator()<Cmp::Persistent::FloodSpeed>( "FloodSpeed" );
  serializeComponent.template operator()<Cmp::Persistent::FuseDelay>( "FuseDelay" );
  serializeComponent.template operator()<Cmp::Persistent::HealthBonus>( "HealthBonus" );
  serializeComponent.template operator()<Cmp::Persistent::MusicVolume>( "MusicVolume" );
  serializeComponent.template operator()<Cmp::Persistent::NpcActivateScale>( "NpcActivateScale" );
  serializeComponent.template operator()<Cmp::Persistent::NpcDamageDelay>( "NpcDamageDelay" );
  serializeComponent.template operator()<Cmp::Persistent::NpcScanScale>( "NpcScanScale" );
  serializeComponent.template operator()<Cmp::Persistent::NpcLerpSpeed>( "NpcLerpSpeed" );
  serializeComponent.template operator()<Cmp::Persistent::NpcDamage>( "NpcDamage" );
  serializeComponent.template operator()<Cmp::Persistent::NpcPushBack>( "NpcPushBack" );
  serializeComponent.template operator()<Cmp::Persistent::ObstaclePushBack>( "ObstaclePushBack" );
  serializeComponent.template operator()<Cmp::Persistent::PlayerStartPosition>( "PlayerStartPosition" );
  serializeComponent.template operator()<Cmp::Persistent::PlayerDetectionScale>( "PlayerDetectionScale" );
  serializeComponent.template operator()<Cmp::Persistent::PlayerLerpSpeed>( "PlayerLerpSpeed" );
  serializeComponent.template operator()<Cmp::Persistent::PlayerDiagonalLerpSpeedModifier>( "PlayerDiagonalLerpSpeedModifier" );
  serializeComponent.template operator()<Cmp::Persistent::PlayerShortcutLerpSpeedModifier>( "PlayerShortcutLerpSpeedModifier" );
  serializeComponent.template operator()<Cmp::Persistent::PlayerSubmergedLerpSpeedModifier>( "PlayerSubmergedLerpSpeedModifier" );
  serializeComponent.template operator()<Cmp::Persistent::WaterBonus>( "WaterBonus" );
  serializeComponent.template operator()<Cmp::Persistent::DiggingCooldownThreshold>( "DiggingCooldownThreshold" );
  serializeComponent.template operator()<Cmp::Persistent::DiggingDamagePerHit>( "DiggingDamagePerHit" );
  serializeComponent.template operator()<Cmp::Persistent::ShrineCost>( "ShrineCost" );
  serializeComponent.template operator()<Cmp::Persistent::MaxShrines>( "MaxShrines" );
  // clang-format on

  std::ofstream outputFile( "res/json/persistent_components.json" );
  if ( outputFile.is_open() )
  {
    outputFile << jsonData.dump( 4 );
    outputFile.close();
    SPDLOG_INFO( "Persistent state saved successfully" );
  }
  else { SPDLOG_ERROR( "Failed to open file for saving persistent state" ); }
}

} // namespace ProceduralMaze::Sys