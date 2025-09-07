#include <Systems/RenderMenuSystem.hpp>

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

void RenderMenuSystem::render_settings( PlayerSystem &m_settings_sys, FloodSystem &m_flood_sys )
{
  // settings render begin
  ImGui::Begin(
      "Settings", nullptr, ImGuiWindowFlags_NoTitleBar
      // | ImGuiWindowFlags_NoResize
      // | ImGuiWindowFlags_NoMove
  );
  // See PlayerSystem::Settings for details of which Components these are set
  ImGui::InputInt( "Bomb Inventory", &m_settings_sys.m_player_settings.bomb_inventory );
  ImGui::SliderInt( "Blast Radius", &m_settings_sys.m_player_settings.blast_radius, 1, 3 );
  ImGui::InputFloat(
      "Max Speed", &m_settings_sys.m_player_settings.max_speed, 1.0f, 10.0f, "%.1f pixels/second"
  );
  ImGui::SliderFloat(
      "Friction Coefficient", &m_settings_sys.m_player_settings.friction_coefficient, 0.01f, 1.f,
      "%.2f"
  );
  ImGui::SliderFloat(
      "Friction Falloff", &m_settings_sys.m_player_settings.friction_falloff, 0.01f, 1.f, "%.2f"
  );
  ImGui::SliderFloat(
      "Above Water Acceleration Rate",
      &m_settings_sys.m_player_settings.above_water_default_acceleration_rate, 100.f, 1000.f,
      "%.1f pixels/second²"
  );
  ImGui::SliderFloat(
      "Above Water Deceleration Rate",
      &m_settings_sys.m_player_settings.above_water_default_deceleration_rate, 100.f, 1000.f,
      "%.1f pixels/second²"
  );
  ImGui::SliderFloat(
      "Under Water Acceleration Rate",
      &m_settings_sys.m_player_settings.under_water_default_acceleration_rate, 50.f, 500.f,
      "%.1f pixels/second²"
  );
  ImGui::SliderFloat(
      "Under Water Deceleration Rate",
      &m_settings_sys.m_player_settings.under_water_default_deceleration_rate, 50.f, 500.f,
      "%.1f pixels/second²"
  );
  ImGui::Separator();
  ImGui::SliderFloat(
      "Flood Velocity", &m_flood_sys.flood_velocity(), 1.f, 10.f, "%.1f pixels/second"
  );
  ImGui::End();

  getWindow().clear();
  {
    sf::Text title_text( m_font, "Settings", 64 );
    title_text.setFillColor( sf::Color::White );
    title_text.setPosition( { 100.f, 50.f } );
    getWindow().draw( title_text );

    sf::Text start_text( m_font, "Press <Esc> key to go back", 24 );
    start_text.setFillColor( sf::Color::White );
    start_text.setPosition( { 400.f, 95.f } );
    getWindow().draw( start_text );
  }

  ImGui::SFML::Render( getWindow() );
  getWindow().display();
  // main render end
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