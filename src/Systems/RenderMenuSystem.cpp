#include <BombSystem.hpp>
#include <Persistent/ArmedOnDelay.hpp>
#include <Persistent/BombBonus.hpp>
#include <Persistent/BombDamage.hpp>
#include <Persistent/FuseDelay.hpp>
#include <Persistent/HealthBonus.hpp>
#include <Persistent/NpcDamage.hpp>
#include <Persistent/NpcPushBack.hpp>
#include <Persistent/ObstaclePushBack.hpp>
#include <Persistent/WaterBonus.hpp>
#include <Systems/RenderMenuSystem.hpp>
#include <imgui.h>

namespace ProceduralMaze::Sys {

void RenderMenuSystem::render_menu()
{
  // main render begin
  getWindow().clear();
  {
    sf::Text title_text( m_font, "Procedural Maze Game", 96 );
    title_text.setFillColor( sf::Color::White );
    title_text.setPosition( { DISPLAY_SIZE.x / 4.f, 100.f } );
    getWindow().draw( title_text );

    sf::Text start_text( m_font, "Press <Enter> key to start", 48 );
    start_text.setFillColor( sf::Color::White );
    start_text.setPosition( { DISPLAY_SIZE.x / 4.f, 300.f } );
    getWindow().draw( start_text );

    sf::Text quit_text( m_font, "Press <Q> key to quit", 48 );
    quit_text.setFillColor( sf::Color::White );
    quit_text.setPosition( { DISPLAY_SIZE.x / 4.f, 350.f } );
    getWindow().draw( quit_text );

    sf::Text settings_text( m_font, "Press <S> key for settings", 48 );
    settings_text.setFillColor( sf::Color::White );
    settings_text.setPosition( { DISPLAY_SIZE.x / 4.f, 400.f } );
    getWindow().draw( settings_text );
  }

  getWindow().display();
  // main render end
}

void RenderMenuSystem::render_settings( sf::Time deltaTime )
{
  getWindow().clear();

  sf::Text title_text( m_font, "Settings", 64 );
  title_text.setFillColor( sf::Color::White );
  title_text.setPosition( { 100.f, 50.f } );
  getWindow().draw( title_text );

  sf::Text start_text( m_font, "Press <Esc> key to go back", 24 );
  start_text.setFillColor( sf::Color::White );
  start_text.setPosition( { 400.f, 95.f } );
  getWindow().draw( start_text );

  // ImGUI should be rendered before window.display() or SFML wipes the display buffer prematurely
  render_settings_widgets( deltaTime );

  getWindow().display();
}

void RenderMenuSystem::render_settings_widgets( sf::Time deltaTime )
{
  // need to make sure we call Update() and Render() every frame
  ImGui::SFML::Update( getWindow(), deltaTime );

  ImGui::Begin( "Settings", nullptr, kImGuiWindowOptions );
  // See PlayerSystem::Settings for details of which Components these are set

  auto &bomb_inventory = m_reg->ctx().get<Cmp::Persistent::BombInventory>();
  if ( ImGui::InputInt( "Bomb Inventory", &bomb_inventory() ) ) { bomb_inventory() = std::clamp( bomb_inventory(), -1, 100 ); }

  auto &blast_radius = m_reg->ctx().get<Cmp::Persistent::BlastRadius>();
  ImGui::SliderInt( "Blast Radius", &blast_radius(), 1, 3 );

  auto &max_speed = m_reg->ctx().get<Cmp::Persistent::PlayerMaxSpeed>();
  if ( ImGui::InputFloat( "Max Speed", &max_speed(), 1.0f, 10.0f, "%.1f pixels/second" ) ) { max_speed() = std::clamp( max_speed(), 50.f, 100.f ); }

  auto &friction_coefficient = m_reg->ctx().get<Cmp::Persistent::FrictionCoefficient>();
  ImGui::SliderFloat( "Friction Coefficient", &friction_coefficient(), 0.01f, 1.f, "%.2f" );

  auto &friction_falloff = m_reg->ctx().get<Cmp::Persistent::FrictionFalloff>();
  ImGui::SliderFloat( "Friction Falloff", &friction_falloff(), 0.01f, 1.f, "%.2f" );

  auto &land_acceleration = m_reg->ctx().get<Cmp::Persistent::LandAcceleration>();
  ImGui::SliderFloat( "Above Water Acceleration Rate", &land_acceleration(), 100.f, 1000.f, "%.1f pixels/second²" );
  auto &land_deceleration = m_reg->ctx().get<Cmp::Persistent::LandDeceleration>();
  ImGui::SliderFloat( "Above Water Deceleration Rate", &land_deceleration(), 100.f, 1000.f, "%.1f pixels/second²" );
  auto &water_acceleration = m_reg->ctx().get<Cmp::Persistent::WaterAcceleration>();
  ImGui::SliderFloat( "Under Water Acceleration Rate", &water_acceleration(), 50.f, 500.f, "%.1f pixels/second²" );
  auto &water_deceleration = m_reg->ctx().get<Cmp::Persistent::WaterDeceleration>();
  ImGui::SliderFloat( "Under Water Deceleration Rate", &water_deceleration(), 50.f, 500.f, "%.1f pixels/second²" );
  ImGui::Separator();
  auto &flood_speed = m_reg->ctx().get<Cmp::Persistent::FloodSpeed>();
  ImGui::SliderFloat( "Flood Velocity", &flood_speed(), 1.f, 10.f, "%.1f pixels/second" );
  ImGui::Separator();

  auto &npc_damage = m_reg->ctx().get<Cmp::Persistent::NpcDamage>();
  ImGui::SliderInt( "NPC Damage", &npc_damage(), 1, 50 );

  auto &bomb_damage = m_reg->ctx().get<Cmp::Persistent::BombDamage>();
  ImGui::SliderInt( "Bomb Damage", &bomb_damage(), 1, 50 );

  auto &fuse_delay = m_reg->ctx().get<Cmp::Persistent::FuseDelay>();
  ImGui::SliderFloat( "Fuse Delay", &fuse_delay(), 1.f, 10.f, "%.1f seconds" );

  auto &armed_on_delay = m_reg->ctx().get<Cmp::Persistent::ArmedOnDelay>();
  if ( ImGui::InputFloat( "Armed Detonation Delay Increment", &armed_on_delay(), 0.001f, 0.001f ) )
  {
    armed_on_delay() = std::clamp( armed_on_delay(), 0.001f, 0.5f );
  }
  auto &armed_off_delay = m_reg->ctx().get<Cmp::Persistent::ArmedOffDelay>();
  if ( ImGui::InputFloat( "Armed Off Delay", &armed_off_delay(), 0.001f, 0.001f ) )
  {
    armed_off_delay() = std::clamp( armed_off_delay(), 0.001f, 0.5f );
  }

  ImGui::Separator();

  auto &health_bonus = m_reg->ctx().get<Cmp::Persistent::HealthBonus>();
  ImGui::SliderInt( "Health Bonus", &health_bonus(), 1, 50 );
  auto &bomb_bonus = m_reg->ctx().get<Cmp::Persistent::BombBonus>();
  ImGui::SliderInt( "Bomb Bonus", &bomb_bonus(), 1, 20 );
  auto &water_bonus = m_reg->ctx().get<Cmp::Persistent::WaterBonus>();
  ImGui::SliderFloat( "Water Bonus", &water_bonus(), 10.f, 500.f, "%.1f units" );

  auto &obstacle_push_back = m_reg->ctx().get<Cmp::Persistent::ObstaclePushBack>();
  if ( ImGui::InputFloat( "Obstacle Push Back Factor", &obstacle_push_back(), 0.1f, 0.1f, "%.2f" ) )
  {
    obstacle_push_back() = std::clamp( obstacle_push_back(), 1.0f, 5.0f );
  }
  auto &npc_push_back = m_reg->ctx().get<Cmp::Persistent::NpcPushBack>();
  if ( ImGui::InputFloat( "NPC Push Back Distance", &npc_push_back(), 0.1f, 0.1f, "%.2f pixels" ) )
  {
    npc_push_back() = std::clamp( npc_push_back(), 1.0f, 50.0f );
  }

  ImGui::End();
  ImGui::SFML::Render( getWindow() );
}

void RenderMenuSystem::render_paused()
{
  // main render begin
  getWindow().clear();
  {
    sf::Text title_text( m_font, "Paused", 96 );
    title_text.setFillColor( sf::Color::White );
    title_text.setPosition( { DISPLAY_SIZE.x / 4.f, 100.f } );
    getWindow().draw( title_text );

    sf::Text start_text( m_font, "Press P to continue", 48 );
    start_text.setFillColor( sf::Color::White );
    start_text.setPosition( { DISPLAY_SIZE.x / 4.f, 200.f } );
    getWindow().draw( start_text );
  }

  getWindow().display();
  // main render end
}

void RenderMenuSystem::render_defeat_screen()
{
  // main render begin
  getWindow().clear();
  {
    sf::Text title_text( m_font, "You died!", 96 );
    title_text.setFillColor( sf::Color::White );
    title_text.setPosition( { DISPLAY_SIZE.x / 4.f, 100.f } );
    getWindow().draw( title_text );

    sf::Text start_text( m_font, "Press <R> key to continue", 48 );
    start_text.setFillColor( sf::Color::White );
    start_text.setPosition( { DISPLAY_SIZE.x / 4.f, 200.f } );
    getWindow().draw( start_text );
  }

  getWindow().display();
  // main render end
}

void RenderMenuSystem::render_victory_screen()
{
  // main render begin
  getWindow().clear();
  {
    sf::Text title_text( m_font, "You won!", 96 );
    title_text.setFillColor( sf::Color::White );
    title_text.setPosition( { DISPLAY_SIZE.x / 4.f, 100.f } );
    getWindow().draw( title_text );

    sf::Text start_text( m_font, "Press <R> key to continue", 48 );
    start_text.setFillColor( sf::Color::White );
    start_text.setPosition( { DISPLAY_SIZE.x / 4.f, 200.f } );
    getWindow().draw( start_text );
  }

  getWindow().display();
  // main render end
}

} // namespace ProceduralMaze::Sys