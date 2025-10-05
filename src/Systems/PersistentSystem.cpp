#include <Persistent/ArmedOffDelay.hpp>
#include <Persistent/ArmedOnDelay.hpp>
#include <Persistent/BlastRadius.hpp>
#include <Persistent/BombBonus.hpp>
#include <Persistent/BombDamage.hpp>
#include <Persistent/BombInventory.hpp>
#include <Persistent/CorruptionDamage.hpp>
#include <Persistent/FloodSpeed.hpp>
#include <Persistent/FuseDelay.hpp>
#include <Persistent/HealthBonus.hpp>
#include <Persistent/MusicVolume.hpp>
#include <Persistent/NPCActivateScale.hpp>
#include <Persistent/NPCScanScale.hpp>
#include <Persistent/NpcDamage.hpp>
#include <Persistent/NpcDamageDelay.hpp>
#include <Persistent/NpcLerpSpeed.hpp>
#include <Persistent/NpcPushBack.hpp>
#include <Persistent/ObstaclePushBack.hpp>
#include <Persistent/PCDetectionScale.hpp>
#include <Persistent/PlayerDiagonalLerpSpeedModifier.hpp>
#include <Persistent/PlayerLerpSpeed.hpp>
#include <Persistent/PlayerShortcutLerpSpeedModifier.hpp>
#include <Persistent/PlayerStartPosition.hpp>
#include <Persistent/PlayerSubmergedlLerpSpeedModifier.hpp>
#include <Persistent/WaterBonus.hpp>
#include <PersistentSystem.hpp>
#include <fstream>

#define JSON_NOEXCEPTION
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

#include <Persistent/GameState.hpp>

namespace ProceduralMaze::Sys {

PersistentSystem::PersistentSystem( SharedEnttRegistry reg )
    : BaseSystem( reg )
{
  add_persistent_component<Cmp::Persistent::GameState>();
}

// call this before changing to menu game state
// 1. read json file
// 2. deserialize json to persistent components
// 3. load persistent components into registry context
void PersistentSystem::loadState()
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
    if ( key == "ArmedOffDelay" )
    {
      float delay = value["value"].template get<float>();
      SPDLOG_INFO( "Loaded ArmedOffDelay with delay: {}", delay );
      add_persistent_component<Cmp::Persistent::ArmedOffDelay>( delay );
    }
    if ( key == "ArmedOnDelay" )
    {
      float delay = value["value"].template get<float>();
      SPDLOG_INFO( "Loaded ArmedOnDelay with delay: {}", delay );
      add_persistent_component<Cmp::Persistent::ArmedOnDelay>( delay );
    }
    if ( key == "FuseDelay" )
    {
      float delay = value["value"].template get<float>();
      SPDLOG_INFO( "Loaded FuseDelay with delay: {}", delay );
      add_persistent_component<Cmp::Persistent::FuseDelay>( delay );
    }
    if ( key == "BombDamage" )
    {
      int damage = value["value"].template get<int>();
      SPDLOG_INFO( "Loaded BombDamage with damage: {}", damage );
      add_persistent_component<Cmp::Persistent::BombDamage>( damage );
    }
    if ( key == "PlayerStartPosition" )
    {
      float x = value["value"]["x"].template get<float>();
      float y = value["value"]["y"].template get<float>();
      if ( x == 0.0f && y == 0.0f )
      {
        SPDLOG_INFO( "Loaded PlayerStartPosition with default position." );
        add_persistent_component<Cmp::Persistent::PlayerStartPosition>();
      }
      else
      {
        SPDLOG_INFO( "Loaded PlayerStartPosition with position: ({}, {})", x, y );
        add_persistent_component<Cmp::Persistent::PlayerStartPosition>( sf::Vector2f( x, y ) );
      }
    }
    if ( key == "BombInventory" )
    {
      int inventory = value["value"].template get<int>();
      SPDLOG_INFO( "Loaded BombInventory with inventory: {}", inventory );
      add_persistent_component<Cmp::Persistent::BombInventory>( inventory );
    }
    if ( key == "BlastRadius" )
    {
      int radius = value["value"].template get<int>();
      SPDLOG_INFO( "Loaded BlastRadius with radius: {}", radius );
      add_persistent_component<Cmp::Persistent::BlastRadius>( radius );
    }
    if ( key == "PCDetectionScale" )
    {
      float scale = value["value"].template get<float>();
      SPDLOG_INFO( "Loaded PCDetectionScale with scale: {}", scale );
      add_persistent_component<Cmp::Persistent::PCDetectionScale>( scale );
    }
    if ( key == "PlayerLerpSpeed" )
    {
      float speed = value["value"].template get<float>();
      SPDLOG_INFO( "Loaded PlayerLerpSpeed with speed: {}", speed );
      add_persistent_component<Cmp::Persistent::PlayerLerpSpeed>( speed );
    }
    if ( key == "PlayerDiagonalLerpSpeedModifier" )
    {
      float modifier = value["value"].template get<float>();
      SPDLOG_INFO( "Loaded PlayerDiagonalLerpSpeedModifier with modifier: {}", modifier );
      add_persistent_component<Cmp::Persistent::PlayerDiagonalLerpSpeedModifier>( modifier );
    }
    if ( key == "PlayerShortcutLerpSpeedModifier" )
    {
      float modifier = value["value"].template get<float>();
      SPDLOG_INFO( "Loaded PlayerShortcutLerpSpeedModifier with modifier: {}", modifier );
      add_persistent_component<Cmp::Persistent::PlayerShortcutLerpSpeedModifier>( modifier );
    }
    if ( key == "PlayerSubmergedLerpSpeedModifier" )
    {
      float modifier = value["value"].template get<float>();
      SPDLOG_INFO( "Loaded PlayerSubmergedLerpSpeedModifier with modifier: {}", modifier );
      add_persistent_component<Cmp::Persistent::PlayerSubmergedLerpSpeedModifier>( modifier );
    }
    if ( key == "HealthBonus" )
    {
      int bonus = value["value"].template get<int>();
      SPDLOG_INFO( "Loaded HealthBonus with bonus: {}", bonus );
      add_persistent_component<Cmp::Persistent::HealthBonus>( bonus );
    }
    if ( key == "BombBonus" )
    {
      int bonus = value["value"].template get<int>();
      SPDLOG_INFO( "Loaded BombBonus with bonus: {}", bonus );
      add_persistent_component<Cmp::Persistent::BombBonus>( bonus );
    }
    if ( key == "WaterBonus" )
    {
      int bonus = value["value"].template get<int>();
      SPDLOG_INFO( "Loaded WaterBonus with bonus: {}", bonus );
      add_persistent_component<Cmp::Persistent::WaterBonus>( bonus );
    }
    if ( key == "NpcDamage" )
    {
      int damage = value["value"].template get<int>();
      SPDLOG_INFO( "Loaded NpcDamage with damage: {}", damage );
      add_persistent_component<Cmp::Persistent::NpcDamage>( damage );
    }
    if ( key == "ObstaclePushBack" )
    {
      float pushback = value["value"].template get<float>();
      SPDLOG_INFO( "Loaded ObstaclePushBack with pushback: {}", pushback );
      add_persistent_component<Cmp::Persistent::ObstaclePushBack>( pushback );
    }
    if ( key == "NpcPushBack" )
    {

      float pushback = value["value"].template get<float>();
      if ( pushback == 0.0f )
      {
        SPDLOG_INFO( "Loaded NpcPushBack with default pushback." );
        add_persistent_component<Cmp::Persistent::NpcPushBack>();
      }
      else
      {
        SPDLOG_INFO( "Loaded NpcPushBack with pushback: {}", pushback );
        add_persistent_component<Cmp::Persistent::NpcPushBack>( pushback );
      }

      SPDLOG_INFO( "Loaded NpcPushBack with pushback: {}", pushback );
      add_persistent_component<Cmp::Persistent::NpcPushBack>( pushback );
    }
    if ( key == "NPCActivateScale" )
    {
      float scale = value["value"].template get<float>();
      SPDLOG_INFO( "Loaded NPCActivateScale with scale: {}", scale );
      add_persistent_component<Cmp::Persistent::NPCActivateScale>( scale );
    }
    if ( key == "NpcDamageDelay" )
    {
      float delay = value["value"].template get<float>();
      SPDLOG_INFO( "Loaded NpcDamageDelay with delay: {}", delay );
      add_persistent_component<Cmp::Persistent::NpcDamageDelay>( delay );
    }
    if ( key == "CorruptionDamage" )
    {
      int damage = value["value"].template get<int>();
      SPDLOG_INFO( "Loaded CorruptionDamage with damage: {}", damage );
      add_persistent_component<Cmp::Persistent::CorruptionDamage>( damage );
    }
    if ( key == "FloodSpeed" )
    {
      float speed = value["value"].template get<float>();
      SPDLOG_INFO( "Loaded FloodSpeed with speed: {}", speed );
      add_persistent_component<Cmp::Persistent::FloodSpeed>( speed );
    }
    if ( key == "MusicVolume" )
    {
      float volume = value["value"].template get<float>();
      SPDLOG_INFO( "Loaded MusicVolume with volume: {}", volume );
      add_persistent_component<Cmp::Persistent::MusicVolume>( volume );
    }
    if ( key == "NPCScanScale" )
    {
      float scale = value["value"].template get<float>();
      SPDLOG_INFO( "Loaded NPCScanScale with scale: {}", scale );
      add_persistent_component<Cmp::Persistent::NPCScanScale>( scale );
    }
    if ( key == "NPCLerpSpeed" )
    {
      float speed = value["value"].template get<float>();
      SPDLOG_INFO( "Loaded NPCLerpSpeed with speed: {}", speed );
      add_persistent_component<Cmp::Persistent::NpcLerpSpeed>( speed );
    }

    // Add more components as needed
  }
}

// call this as last action when exiting settings menu
// 1. serialise persistent components to json
// 2. write json to file
void PersistentSystem::saveState() { SPDLOG_INFO( "Saving persistent state..." ); }

} // namespace ProceduralMaze::Sys