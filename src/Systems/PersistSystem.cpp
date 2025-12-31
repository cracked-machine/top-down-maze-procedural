#include <Components/Persistent/CryptNpcSpawnCount.hpp>
#include <Components/Persistent/NpcShockwaveFreq.hpp>
#include <Components/Persistent/NpcShockwaveMaxRadius.hpp>
#include <Components/Persistent/NpcShockwaveResolution.hpp>
#include <Components/Persistent/NpcShockwaveSpeed.hpp>
#include <Systems/PersistSystem.hpp>

#define JSON_NOEXCEPTION
#include <fstream>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

#include <Components/Persistent/ArmedOffDelay.hpp>
#include <Components/Persistent/ArmedOnDelay.hpp>
#include <Components/Persistent/BlastRadius.hpp>
#include <Components/Persistent/BombBonus.hpp>
#include <Components/Persistent/BombDamage.hpp>
#include <Components/Persistent/BombInventory.hpp>
#include <Components/Persistent/CorruptionDamage.hpp>
#include <Components/Persistent/CorruptionSeed.hpp>
#include <Components/Persistent/DiggingCooldownThreshold.hpp>
#include <Components/Persistent/DiggingDamagePerHit.hpp>
#include <Components/Persistent/DisplayResolution.hpp>
#include <Components/Persistent/EffectsVolume.hpp>
#include <Components/Persistent/ExitKeyRequirement.hpp>
#include <Components/Persistent/FuseDelay.hpp>
#include <Components/Persistent/GameState.hpp>
#include <Components/Persistent/GraveNumMultiplier.hpp>
#include <Components/Persistent/HealthBonus.hpp>
#include <Components/Persistent/MaxNumAltars.hpp>
#include <Components/Persistent/MaxNumCrypts.hpp>
#include <Components/Persistent/MusicVolume.hpp>
#include <Components/Persistent/NpcActivateScale.hpp>
#include <Components/Persistent/NpcDamage.hpp>
#include <Components/Persistent/NpcDeathAnimFramerate.hpp>
#include <Components/Persistent/NpcGhostAnimFramerate.hpp>
#include <Components/Persistent/NpcLerpSpeed.hpp>
#include <Components/Persistent/NpcPushBack.hpp>
#include <Components/Persistent/NpcScanScale.hpp>
#include <Components/Persistent/NpcShockwaveSpeed.hpp>
#include <Components/Persistent/NpcSkeleAnimFramerate.hpp>
#include <Components/Persistent/PcDamageDelay.hpp>
#include <Components/Persistent/PlayerAnimFramerate.hpp>
#include <Components/Persistent/PlayerDetectionScale.hpp>
#include <Components/Persistent/PlayerDiagonalLerpSpeedModifier.hpp>
#include <Components/Persistent/PlayerFootstepAddDelay.hpp>
#include <Components/Persistent/PlayerFootstepFadeDelay.hpp>
#include <Components/Persistent/PlayerLerpSpeed.hpp>
#include <Components/Persistent/PlayerShortcutLerpSpeedModifier.hpp>
#include <Components/Persistent/PlayerStartPosition.hpp>
#include <Components/Persistent/SinkholeSeed.hpp>
#include <Components/Persistent/WeaponDegradePerHit.hpp>
#include <Components/Persistent/WormholeAnimFramerate.hpp>
#include <Components/Persistent/WormholeSeed.hpp>
#include <Systems/BaseSystem.hpp>
#include <Systems/PersistSystemImpl.hpp>
#include <Utils/Utils.hpp>

namespace ProceduralMaze::Sys
{

PersistSystem::PersistSystem( entt::registry &reg, sf::RenderWindow &window, Sprites::SpriteFactory &sprite_factory, Audio::SoundBank &sound_bank )
    : BaseSystem( reg, window, sprite_factory, sound_bank )
{
  // The entt::dispatcher is independent of the registry, so it is safe to bind event handlers in
  // the constructor
  std::ignore = get_systems_event_queue().sink<Events::SaveSettingsEvent>().connect<&Sys::PersistSystem::on_save_settings_event>( this );
  SPDLOG_DEBUG( "PersistSystem initialized" );
}

void PersistSystem::initializeComponentRegistry()
{
  // Register a JSON key and its derived Cmp::Persist::BasePersistent class for deserialization (load).
  // Note: You can set default values here but they will be overridden when loading from json file.
  // If you want these defaults to be used then you must override the deserialize() function in the
  // derived Cmp::Persist::BasePersistent class. See PlayerStartPosition component as an example.
  // clang-format off
  registerComponent<Cmp::Persist::ArmedOffDelay>( "ArmedOffDelay" );
  registerComponent<Cmp::Persist::ArmedOnDelay>( "ArmedOnDelay" );
  registerComponent<Cmp::Persist::BlastRadius>( "BlastRadius" );
  registerComponent<Cmp::Persist::BombBonus>( "BombBonus" );
  registerComponent<Cmp::Persist::BombDamage>( "BombDamage" );
  registerComponent<Cmp::Persist::BombInventory>( "BombInventory" );
  registerComponent<Cmp::Persist::CorruptionDamage>( "CorruptionDamage" );
  registerComponent<Cmp::Persist::CryptNpcSpawnCount>( "CryptNpcSpawnCount" );
  registerComponent<Cmp::Persist::DiggingCooldownThreshold>( "DiggingCooldownThreshold" );
  registerComponent<Cmp::Persist::DiggingDamagePerHit>( "DiggingDamagePerHit" );
  registerComponent<Cmp::Persist::DisplayResolution>( "DisplayResolution", Constants::kFallbackDisplaySize );
  registerComponent<Cmp::Persist::EffectsVolume>( "EffectsVolume" );
  registerComponent<Cmp::Persist::ExitKeyRequirement>( "ExitKeyRequirement" );
  registerComponent<Cmp::Persist::FuseDelay>( "FuseDelay" );
  registerComponent<Cmp::Persist::GraveNumMultiplier>( "GraveNumMultiplier" );
  registerComponent<Cmp::Persist::HealthBonus>( "HealthBonus" );
  registerComponent<Cmp::Persist::MaxNumAltars>( "MaxNumAltars" );
  registerComponent<Cmp::Persist::MaxNumCrypts>( "MaxNumCrypts" );
  registerComponent<Cmp::Persist::MusicVolume>( "MusicVolume" );
  registerComponent<Cmp::Persist::NpcActivateScale>( "NpcActivateScale" );
  registerComponent<Cmp::Persist::NpcDamage>( "NpcDamage" );
  registerComponent<Cmp::Persist::NpcDeathAnimFramerate>( "NpcDeathAnimFramerate" );
  registerComponent<Cmp::Persist::NpcGhostAnimFramerate>( "NpcGhostAnimFramerate" );
  registerComponent<Cmp::Persist::NpcLerpSpeed>( "NpcLerpSpeed" );
  registerComponent<Cmp::Persist::NpcPushBack>( "NpcPushBack" );
  registerComponent<Cmp::Persist::NpcScanScale>( "NpcScanScale" );
  registerComponent<Cmp::Persist::NpcSkeleAnimFramerate>( "NpcSkeleAnimFramerate" );
  registerComponent<Cmp::Persist::NpcShockwaveResolution>( "NpcShockwaveResolution" );
  registerComponent<Cmp::Persist::NpcShockwaveSpeed>( "NpcShockwaveSpeed" );
  registerComponent<Cmp::Persist::NpcShockwaveFreq>( "NpcShockwaveFreq" );
  registerComponent<Cmp::Persist::NpcShockwaveMaxRadius>( "NpcShockwaveMaxRadius" );
  registerComponent<Cmp::Persist::PcDamageDelay>( "PcDamageDelay" );
  registerComponent<Cmp::Persist::PlayerAnimFramerate>( "PlayerAnimFramerate" );
  registerComponent<Cmp::Persist::PlayerDetectionScale>( "PlayerDetectionScale" );
  registerComponent<Cmp::Persist::PlayerDiagonalLerpSpeedModifier>( "PlayerDiagonalLerpSpeedModifier" );
  registerComponent<Cmp::Persist::PlayerFootstepAddDelay>( "PlayerFootstepAddDelay" );
  registerComponent<Cmp::Persist::PlayerFootstepFadeDelay>( "PlayerFootstepFadeDelay" );
  registerComponent<Cmp::Persist::PlayerLerpSpeed>( "PlayerLerpSpeed" );
  registerComponent<Cmp::Persist::PlayerShortcutLerpSpeedModifier>( "PlayerShortcutLerpSpeedModifier" );
  registerComponent<Cmp::Persist::WeaponDegradePerHit>( "WeaponDegradePerHit" );
  registerComponent<Cmp::Persist::WormholeAnimFramerate>( "WormholeAnimFramerate" );
  // clang-format on

  Sys::PersistSystem::add_persist_cmp<Cmp::Persist::WormholeSeed>( getReg(), 0 );
  Sys::PersistSystem::add_persist_cmp<Cmp::Persist::SinkholeSeed>( getReg(), 0 );
  Sys::PersistSystem::add_persist_cmp<Cmp::Persist::CorruptionSeed>( getReg(), 0 );
}

void PersistSystem::load_state()
{
  SPDLOG_DEBUG( "Loading persistent state..." );
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

void PersistSystem::save_state()
{
  SPDLOG_DEBUG( "Saving persistent state..." );
  nlohmann::json jsonData;

  // Helper lambda to serialize a component if it exists
  auto serializeComponent = [&]<typename ComponentType>( const std::string &key )
  {
    try
    {
      auto &component = Sys::PersistSystem::get_persist_cmp<ComponentType>( getReg() );
      jsonData[key] = component.serialize();
    } catch ( const std::exception &e )
    {
      SPDLOG_WARN( "Failed to serialize component {}: {}", key, e.what() );
    }
  };

  // Serialize (save) all registered components
  // clang-format off
  serializeComponent.template operator()<Cmp::Persist::ArmedOffDelay>( "ArmedOffDelay" );
  serializeComponent.template operator()<Cmp::Persist::ArmedOnDelay>( "ArmedOnDelay" );
  serializeComponent.template operator()<Cmp::Persist::BlastRadius>( "BlastRadius" );
  serializeComponent.template operator()<Cmp::Persist::BombBonus>( "BombBonus" );
  serializeComponent.template operator()<Cmp::Persist::BombDamage>( "BombDamage" );
  serializeComponent.template operator()<Cmp::Persist::BombInventory>( "BombInventory" );
  serializeComponent.template operator()<Cmp::Persist::CorruptionDamage>( "CorruptionDamage" );
  serializeComponent.template operator()<Cmp::Persist::CryptNpcSpawnCount>( "CryptNpcSpawnCount" );
  serializeComponent.template operator()<Cmp::Persist::DiggingCooldownThreshold>( "DiggingCooldownThreshold" );
  serializeComponent.template operator()<Cmp::Persist::DiggingDamagePerHit>( "DiggingDamagePerHit" );
  serializeComponent.template operator()<Cmp::Persist::DisplayResolution>( "DisplayResolution" );
  serializeComponent.template operator()<Cmp::Persist::EffectsVolume>( "EffectsVolume" );
  serializeComponent.template operator()<Cmp::Persist::ExitKeyRequirement>( "ExitKeyRequirement" );
  serializeComponent.template operator()<Cmp::Persist::FuseDelay>( "FuseDelay" );
  serializeComponent.template operator()<Cmp::Persist::GraveNumMultiplier>( "GraveNumMultiplier" );
  serializeComponent.template operator()<Cmp::Persist::HealthBonus>( "HealthBonus" );
  serializeComponent.template operator()<Cmp::Persist::MaxNumAltars>( "MaxNumAltars" );
  serializeComponent.template operator()<Cmp::Persist::MaxNumCrypts>( "MaxNumCrypts" );
  serializeComponent.template operator()<Cmp::Persist::MusicVolume>( "MusicVolume" );
  serializeComponent.template operator()<Cmp::Persist::NpcActivateScale>( "NpcActivateScale" );
  serializeComponent.template operator()<Cmp::Persist::NpcDamage>( "NpcDamage" );
  serializeComponent.template operator()<Cmp::Persist::NpcDeathAnimFramerate>( "NpcDeathAnimFramerate" );
  serializeComponent.template operator()<Cmp::Persist::NpcGhostAnimFramerate>( "NpcGhostAnimFramerate" );
  serializeComponent.template operator()<Cmp::Persist::NpcLerpSpeed>( "NpcLerpSpeed" );
  serializeComponent.template operator()<Cmp::Persist::NpcPushBack>( "NpcPushBack" );
  serializeComponent.template operator()<Cmp::Persist::NpcScanScale>( "NpcScanScale" );
  serializeComponent.template operator()<Cmp::Persist::NpcSkeleAnimFramerate>( "NpcSkeleAnimFramerate" );
  serializeComponent.template operator()<Cmp::Persist::NpcShockwaveSpeed>( "NpcShockwaveSpeed" );
  serializeComponent.template operator()<Cmp::Persist::NpcShockwaveResolution>( "NpcShockwaveResolution" );
  serializeComponent.template operator()<Cmp::Persist::NpcShockwaveFreq>( "NpcShockwaveFreq" );
  serializeComponent.template operator()<Cmp::Persist::NpcShockwaveMaxRadius>( "NpcShockwaveMaxRadius" );
  serializeComponent.template operator()<Cmp::Persist::PcDamageDelay>( "PcDamageDelay" );
  serializeComponent.template operator()<Cmp::Persist::PlayerAnimFramerate>( "PlayerAnimFramerate" );
  serializeComponent.template operator()<Cmp::Persist::PlayerDetectionScale>( "PlayerDetectionScale" );
  serializeComponent.template operator()<Cmp::Persist::PlayerDiagonalLerpSpeedModifier>( "PlayerDiagonalLerpSpeedModifier" );
  serializeComponent.template operator()<Cmp::Persist::PlayerFootstepAddDelay>( "PlayerFootstepAddDelay" );
  serializeComponent.template operator()<Cmp::Persist::PlayerFootstepFadeDelay>( "PlayerFootstepFadeDelay" );
  serializeComponent.template operator()<Cmp::Persist::PlayerLerpSpeed>( "PlayerLerpSpeed" );
  serializeComponent.template operator()<Cmp::Persist::PlayerShortcutLerpSpeedModifier>( "PlayerShortcutLerpSpeedModifier" );
  serializeComponent.template operator()<Cmp::Persist::PlayerStartPosition>( "PlayerStartPosition" );
  serializeComponent.template operator()<Cmp::Persist::WeaponDegradePerHit>( "WeaponDegradePerHit" );
  serializeComponent.template operator()<Cmp::Persist::WormholeAnimFramerate>( "WormholeAnimFramerate" );

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