#include <BombSystem.hpp>
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

void RenderMenuSystem::render_settings( PlayerSystem &psys, FloodSystem &fsys, BombSystem &bsys, sf::Time deltaTime )
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
  render_settings_widgets( psys, fsys, bsys, deltaTime );

  getWindow().display();
}

void RenderMenuSystem::render_settings_widgets( PlayerSystem &psys, FloodSystem &fsys, BombSystem &bsys, sf::Time deltaTime )
{
  // need to make sure we call Update() and Render() every frame
  ImGui::SFML::Update( getWindow(), deltaTime );

  ImGui::Begin( "Settings", nullptr, kImGuiWindowOptions );
  // See PlayerSystem::Settings for details of which Components these are set
  if ( ImGui::InputInt( "Bomb Inventory", &psys.m_settings.bomb_inventory ) )
  {
    psys.m_settings.bomb_inventory = std::clamp( psys.m_settings.bomb_inventory, -1, 100 );
  }
  ImGui::SliderInt( "Blast Radius", &psys.m_settings.blast_radius, 1, 3 );
  if ( ImGui::InputFloat( "Max Speed", &psys.m_settings.max_speed, 1.0f, 10.0f, "%.1f pixels/second" ) )
  {
    psys.m_settings.max_speed = std::clamp( psys.m_settings.max_speed, 50.f, 100.f );
  }
  ImGui::SliderFloat( "Friction Coefficient", &psys.m_settings.friction_coefficient, 0.01f, 1.f, "%.2f" );
  ImGui::SliderFloat( "Friction Falloff", &psys.m_settings.friction_falloff, 0.01f, 1.f, "%.2f" );
  ImGui::SliderFloat( "Above Water Acceleration Rate", &psys.m_settings.above_water_default_acceleration_rate, 100.f, 1000.f, "%.1f pixels/second²" );
  ImGui::SliderFloat( "Above Water Deceleration Rate", &psys.m_settings.above_water_default_deceleration_rate, 100.f, 1000.f, "%.1f pixels/second²" );
  ImGui::SliderFloat( "Under Water Acceleration Rate", &psys.m_settings.under_water_default_acceleration_rate, 50.f, 500.f, "%.1f pixels/second²" );
  ImGui::SliderFloat( "Under Water Deceleration Rate", &psys.m_settings.under_water_default_deceleration_rate, 50.f, 500.f, "%.1f pixels/second²" );
  ImGui::Separator();
  ImGui::SliderFloat( "Flood Velocity", &fsys.flood_velocity(), 1.f, 10.f, "%.1f pixels/second" );
  ImGui::Separator();
  ImGui::SliderInt( "Bomb Damage", &bsys.m_settings.player_damage, 1, 50 );
  ImGui::SliderFloat( "Fuse Delay", &bsys.m_settings.base_fuse_delay, 1.f, 10.f, "%.1f seconds" );
  if ( ImGui::InputFloat( "Armed Detonation Delay Increment", &bsys.m_settings.armed_detonation_delay_increment, 0.001f, 0.001f ) )
  {
    bsys.m_settings.armed_detonation_delay_increment = std::clamp( bsys.m_settings.armed_detonation_delay_increment, 0.001f, 0.5f );
  }
  if ( ImGui::InputFloat( "Armed Warning Delay Increment", &bsys.m_settings.armed_warning_delay_increment, 0.001f, 0.001f ) )
  {
    bsys.m_settings.armed_warning_delay_increment = std::clamp( bsys.m_settings.armed_warning_delay_increment, 0.001f, 0.5f );
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