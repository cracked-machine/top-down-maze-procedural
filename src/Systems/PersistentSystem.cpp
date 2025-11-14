#include <Components/Persistent/EffectsVolume.hpp>
#include <Components/Persistent/WeaponDegradePerHit.hpp>
#define JSON_NOEXCEPTION
#include <spdlog/spdlog.h>

#include <Components/Persistent/ArmedOffDelay.hpp>
#include <Components/Persistent/ArmedOnDelay.hpp>
#include <Components/Persistent/BlastRadius.hpp>
#include <Components/Persistent/BombBonus.hpp>
#include <Components/Persistent/BombDamage.hpp>
#include <Components/Persistent/BombInventory.hpp>
#include <Components/Persistent/CorruptionDamage.hpp>
#include <Components/Persistent/DiggingCooldownThreshold.hpp>
#include <Components/Persistent/DiggingDamagePerHit.hpp>
#include <Components/Persistent/FuseDelay.hpp>
#include <Components/Persistent/GameState.hpp>
#include <Components/Persistent/HealthBonus.hpp>
#include <Components/Persistent/MaxShrines.hpp>
#include <Components/Persistent/MusicVolume.hpp>
#include <Components/Persistent/NpcActivateScale.hpp>
#include <Components/Persistent/NpcDamage.hpp>
#include <Components/Persistent/NpcDeathAnimFramerate.hpp>
#include <Components/Persistent/NpcGhostAnimFramerate.hpp>
#include <Components/Persistent/NpcLerpSpeed.hpp>
#include <Components/Persistent/NpcPushBack.hpp>
#include <Components/Persistent/NpcScanScale.hpp>
#include <Components/Persistent/NpcSkeleAnimFramerate.hpp>
#include <Components/Persistent/PcDamageDelay.hpp>
#include <Components/Persistent/PlayerAnimFramerate.hpp>
#include <Components/Persistent/PlayerDetectionScale.hpp>
#include <Components/Persistent/PlayerDiagonalLerpSpeedModifier.hpp>
#include <Components/Persistent/PlayerLerpSpeed.hpp>
#include <Components/Persistent/PlayerShortcutLerpSpeedModifier.hpp>
#include <Components/Persistent/PlayerStartPosition.hpp>
#include <Components/Persistent/ShrineCostPerSprite.hpp>
#include <Components/Persistent/WormholeAnimFramerate.hpp>
#include <Systems/PersistentSystem.hpp>
#include <fstream>
#include <nlohmann/json.hpp>

namespace ProceduralMaze::Sys {

PersistentSystem::PersistentSystem( SharedEnttRegistry reg, sf::RenderWindow &window, Sprites::SpriteFactory &sprite_factory,
                                    Audio::SoundBank &sound_bank )
    : BaseSystem( reg, window, sprite_factory, sound_bank )
{
  SPDLOG_DEBUG( "PersistentSystem constructor called" );

  std::ignore = getEventDispatcher().sink<Events::SaveSettingsEvent>().connect<&Sys::PersistentSystem::on_save_settings_event>(
      this );
  SPDLOG_DEBUG( "PersistentSystem initialized" );
}

void PersistentSystem::initializeComponentRegistry()
{
  // Register a JSON key and its derived Cmp::Persistent::BasePersistent class for deserialization.
  // Note: You can set default values here but they will be overridden when loading from json file.
  // If you want these defaults to be used then you must override the deserialize() function in the
  // derived Cmp::Persistent::BasePersistent class. See PlayerStartPosition component as an example.
  // clang-format off
  registerComponent<Cmp::Persistent::ArmedOffDelay>( "ArmedOffDelay" );
  registerComponent<Cmp::Persistent::ArmedOnDelay>( "ArmedOnDelay" );
  registerComponent<Cmp::Persistent::BombDamage>( "BombDamage" );
  registerComponent<Cmp::Persistent::BombInventory>( "BombInventory" );
  registerComponent<Cmp::Persistent::BlastRadius>( "BlastRadius" );
  registerComponent<Cmp::Persistent::BombBonus>( "BombBonus" );
  registerComponent<Cmp::Persistent::CorruptionDamage>( "CorruptionDamage" );
  registerComponent<Cmp::Persistent::DiggingCooldownThreshold>( "DiggingCooldownThreshold" );
  registerComponent<Cmp::Persistent::DiggingDamagePerHit>( "DiggingDamagePerHit" );
  registerComponent<Cmp::Persistent::EffectsVolume>( "EffectsVolume" );
  registerComponent<Cmp::Persistent::FuseDelay>( "FuseDelay" );
  registerComponent<Cmp::Persistent::HealthBonus>( "HealthBonus" );
  registerComponent<Cmp::Persistent::MusicVolume>( "MusicVolume" );
  registerComponent<Cmp::Persistent::MaxShrines>( "MaxShrines" );
  registerComponent<Cmp::Persistent::NpcGhostAnimFramerate>( "NpcGhostAnimFramerate" );
  registerComponent<Cmp::Persistent::NpcSkeleAnimFramerate>( "NpcSkeleAnimFramerate" );
  registerComponent<Cmp::Persistent::NpcActivateScale>( "NpcActivateScale" );
  registerComponent<Cmp::Persistent::PcDamageDelay>( "PcDamageDelay" );
  registerComponent<Cmp::Persistent::NpcScanScale>( "NpcScanScale" );
  registerComponent<Cmp::Persistent::NpcLerpSpeed>( "NpcLerpSpeed" );
  registerComponent<Cmp::Persistent::NpcDamage>( "NpcDamage" );
  registerComponent<Cmp::Persistent::NpcPushBack>( "NpcPushBack" );
  registerComponent<Cmp::Persistent::NpcDeathAnimFramerate>( "NpcDeathAnimFramerate" );
  registerComponent<Cmp::Persistent::PlayerAnimFramerate>( "PlayerAnimFramerate" );
  registerComponent<Cmp::Persistent::PlayerDetectionScale>( "PlayerDetectionScale" );
  registerComponent<Cmp::Persistent::PlayerLerpSpeed>( "PlayerLerpSpeed" );
  registerComponent<Cmp::Persistent::PlayerDiagonalLerpSpeedModifier>( "PlayerDiagonalLerpSpeedModifier" );
  registerComponent<Cmp::Persistent::PlayerShortcutLerpSpeedModifier>( "PlayerShortcutLerpSpeedModifier" );
  registerComponent<Cmp::Persistent::ShrineCostPerSprite>( "ShrineCostPerSprite" );
  registerComponent<Cmp::Persistent::WormholeAnimFramerate>( "WormholeAnimFramerate" );
  registerComponent<Cmp::Persistent::WeaponDegradePerHit>( "WeaponDegradePerHit" );

  // clang-format on

  // The default value here is retained if the corresponding value in the json is 0,0
  auto default_player_start_pos = sf::Vector2f( ( Sys::BaseSystem::kGridSquareSizePixels.x * 5 ),
                                                ( Sys::BaseSystem::kDisplaySize.y / 2.f ) );

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
  serializeComponent.template operator()<Cmp::Persistent::CorruptionDamage>( "CorruptionDamage" );
  serializeComponent.template operator()<Cmp::Persistent::DiggingCooldownThreshold>( "DiggingCooldownThreshold" );
  serializeComponent.template operator()<Cmp::Persistent::DiggingDamagePerHit>( "DiggingDamagePerHit" );
  serializeComponent.template operator()<Cmp::Persistent::EffectsVolume>( "EffectsVolume" );
  serializeComponent.template operator()<Cmp::Persistent::FuseDelay>( "FuseDelay" );
  serializeComponent.template operator()<Cmp::Persistent::HealthBonus>( "HealthBonus" );
  serializeComponent.template operator()<Cmp::Persistent::MaxShrines>( "MaxShrines" );
  serializeComponent.template operator()<Cmp::Persistent::MusicVolume>( "MusicVolume" );
  serializeComponent.template operator()<Cmp::Persistent::NpcActivateScale>( "NpcActivateScale" );
  serializeComponent.template operator()<Cmp::Persistent::NpcScanScale>( "NpcScanScale" );
  serializeComponent.template operator()<Cmp::Persistent::NpcLerpSpeed>( "NpcLerpSpeed" );
  serializeComponent.template operator()<Cmp::Persistent::NpcDamage>( "NpcDamage" );
  serializeComponent.template operator()<Cmp::Persistent::NpcDeathAnimFramerate>( "NpcDeathAnimFramerate" );
  serializeComponent.template operator()<Cmp::Persistent::NpcPushBack>( "NpcPushBack" );
  serializeComponent.template operator()<Cmp::Persistent::NpcSkeleAnimFramerate>( "NpcSkeleAnimFramerate" );
  serializeComponent.template operator()<Cmp::Persistent::NpcGhostAnimFramerate>( "NpcGhostAnimFramerate" );
  serializeComponent.template operator()<Cmp::Persistent::PcDamageDelay>( "PcDamageDelay" );
  serializeComponent.template operator()<Cmp::Persistent::PlayerStartPosition>( "PlayerStartPosition" );
  serializeComponent.template operator()<Cmp::Persistent::PlayerDetectionScale>( "PlayerDetectionScale" );
  serializeComponent.template operator()<Cmp::Persistent::PlayerLerpSpeed>( "PlayerLerpSpeed" );
  serializeComponent.template operator()<Cmp::Persistent::PlayerAnimFramerate>( "PlayerAnimFramerate" );
  serializeComponent.template operator()<Cmp::Persistent::PlayerDiagonalLerpSpeedModifier>( "PlayerDiagonalLerpSpeedModifier" );
  serializeComponent.template operator()<Cmp::Persistent::PlayerShortcutLerpSpeedModifier>( "PlayerShortcutLerpSpeedModifier" );
  serializeComponent.template operator()<Cmp::Persistent::ShrineCostPerSprite>( "ShrineCostPerSprite" );
  serializeComponent.template operator()<Cmp::Persistent::WeaponDegradePerHit>( "WeaponDegradePerHit" );
  serializeComponent.template operator()<Cmp::Persistent::WormholeAnimFramerate>( "WormholeAnimFramerate" );

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