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
  // Register float components
  registerComponent<Cmp::Persistent::ArmedOffDelay>( "ArmedOffDelay" );
  registerComponent<Cmp::Persistent::ArmedOnDelay>( "ArmedOnDelay" );
  registerComponent<Cmp::Persistent::FuseDelay>( "FuseDelay" );
  registerComponent<Cmp::Persistent::PlayerAnimFramerate>( "PlayerAnimFramerate" );
  registerComponent<Cmp::Persistent::PlayerDetectionScale>( "PlayerDetectionScale" );
  registerComponent<Cmp::Persistent::PlayerLerpSpeed>( "PlayerLerpSpeed" );
  registerComponent<Cmp::Persistent::PlayerDiagonalLerpSpeedModifier>( "PlayerDiagonalLerpSpeedModifier" );
  registerComponent<Cmp::Persistent::PlayerShortcutLerpSpeedModifier>( "PlayerShortcutLerpSpeedModifier" );
  registerComponent<Cmp::Persistent::PlayerSubmergedLerpSpeedModifier>( "PlayerSubmergedLerpSpeedModifier" );
  registerComponent<Cmp::Persistent::ObstaclePushBack>( "ObstaclePushBack" );
  registerComponent<Cmp::Persistent::NpcAnimFramerate>( "NpcAnimFramerate" );
  registerComponent<Cmp::Persistent::NpcActivateScale>( "NpcActivateScale" );
  registerComponent<Cmp::Persistent::NpcDamageDelay>( "NpcDamageDelay" );
  registerComponent<Cmp::Persistent::FloodSpeed>( "FloodSpeed" );
  registerComponent<Cmp::Persistent::MusicVolume>( "MusicVolume" );
  registerComponent<Cmp::Persistent::NpcScanScale>( "NpcScanScale" );
  registerComponent<Cmp::Persistent::NpcLerpSpeed>( "NpcLerpSpeed" );
  registerComponent<Cmp::Persistent::WormholeAnimFramerate>( "WormholeAnimFramerate" );
  registerComponent<Cmp::Persistent::NpcDeathAnimFramerate>( "NpcDeathAnimFramerate" );
  registerComponent<Cmp::Persistent::DiggingCooldownThreshold>( "DiggingCooldownThreshold" );
  registerComponent<Cmp::Persistent::DiggingDamagePerHit>( "DiggingDamagePerHit" );

  // Register int components
  registerComponent<Cmp::Persistent::BombDamage>( "BombDamage" );
  registerComponent<Cmp::Persistent::BombInventory>( "BombInventory" );
  registerComponent<Cmp::Persistent::BlastRadius>( "BlastRadius" );
  registerComponent<Cmp::Persistent::HealthBonus>( "HealthBonus" );
  registerComponent<Cmp::Persistent::BombBonus>( "BombBonus" );
  registerComponent<Cmp::Persistent::WaterBonus>( "WaterBonus" );
  registerComponent<Cmp::Persistent::NpcDamage>( "NpcDamage" );
  registerComponent<Cmp::Persistent::CorruptionDamage>( "CorruptionDamage" );

  // Register special types (sf::Vector2f)
  registerComponent<Cmp::Persistent::PlayerStartPosition>( "PlayerStartPosition" );
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