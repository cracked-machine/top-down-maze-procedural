#include <Components/FootStepAlpha.hpp>
#include <Components/FootStepTimer.hpp>
#include <Components/Persistent/PcDamageDelay.hpp>
#include <Components/PlayerMortality.hpp>
#include <Components/SpriteAnimation.hpp>
#include <Components/WormholeJump.hpp>
#include <Systems/Render/RenderPlayerSystem.hpp>
#include <Systems/Render/RenderSystem.hpp>
#include <tuple>

namespace ProceduralMaze::Sys {

void RenderPlayerSystem::render_player()
{
  auto pc_view = m_reg->view<Cmp::PlayableCharacter, Cmp::Position, Cmp::Direction, Cmp::PCDetectionBounds, Cmp::SpriteAnimation,
                             Cmp::PlayerMortality>();
  for ( auto [entity, pc_cmp, pc_pos_cmp, dir_cmp, pc_detection_bounds, anim_cmp, pc_mort_cmp] : pc_view.each() )
  {

    switch ( pc_mort_cmp.state )
    {
      case Cmp::PlayerMortality::State::ALIVE: {

        // Only render if not in cooldown OR if in cooldown and blink is visible
        auto &pc_damage_cooldown = get_persistent_component<Cmp::Persistent::PcDamageDelay>();
        bool is_in_damage_cooldown = pc_cmp.m_damage_cooldown_timer.getElapsedTime().asSeconds() < pc_damage_cooldown.get_value();
        int blink_visible = static_cast<int>( pc_cmp.m_damage_cooldown_timer.getElapsedTime().asMilliseconds() / 100 ) % 2 == 0;
        if ( !is_in_damage_cooldown || ( is_in_damage_cooldown && blink_visible ) )
        {
          auto params_tuple = calc_alive_render_params( "PLAYER", pc_pos_cmp, dir_cmp, anim_cmp, entity );
          std::apply( [this]( auto &&...tpl_args ) { safe_render_sprite( std::forward<decltype( tpl_args )>( tpl_args )... ); },
                      params_tuple );
        }
        break;
      }
      case Cmp::PlayerMortality::State::FALLING: {

        auto params_tuple = calc_falling_render_params( "PLAYER", pc_pos_cmp, dir_cmp, anim_cmp, pc_mort_cmp );
        std::apply( [this]( auto &&...tpl_args ) { safe_render_sprite( std::forward<decltype( tpl_args )>( tpl_args )... ); },
                    params_tuple );

        break;
      }
      case Cmp::PlayerMortality::State::HAUNTED: {
        pc_mort_cmp.death_progress += 0.1f;
        auto params_tuple = calc_alive_render_params( "PLAYER", pc_pos_cmp, dir_cmp, anim_cmp, entity );
        std::apply( [this]( auto &&...tpl_args ) { safe_render_sprite( std::forward<decltype( tpl_args )>( tpl_args )... ); },
                    params_tuple );
        break;
      }
      case Cmp::PlayerMortality::State::DECAYING: {
        pc_mort_cmp.death_progress += 0.1f;
        auto params_tuple = calc_alive_render_params( "PLAYER", pc_pos_cmp, dir_cmp, anim_cmp, entity );
        std::apply( [this]( auto &&...tpl_args ) { safe_render_sprite( std::forward<decltype( tpl_args )>( tpl_args )... ); },
                    params_tuple );
        break;
      }
      case Cmp::PlayerMortality::State::EXPLODING: {
        pc_mort_cmp.death_progress += 0.1f;
        auto params_tuple = calc_alive_render_params( "PLAYER", pc_pos_cmp, dir_cmp, anim_cmp, entity );
        std::apply( [this]( auto &&...tpl_args ) { safe_render_sprite( std::forward<decltype( tpl_args )>( tpl_args )... ); },
                    params_tuple );
        break;
      }
      case Cmp::PlayerMortality::State::DROWNING: {
        pc_mort_cmp.death_progress += 0.1f;
        auto params_tuple = calc_alive_render_params( "PLAYER", pc_pos_cmp, dir_cmp, anim_cmp, entity );
        std::apply( [this]( auto &&...tpl_args ) { safe_render_sprite( std::forward<decltype( tpl_args )>( tpl_args )... ); },
                    params_tuple );
        break;
      }

      case Cmp::PlayerMortality::State::DEAD: {
        pc_mort_cmp.death_progress += 0.1f;
        break;
      }
        // do nothing
    };

    if ( m_show_path_finding ) { render_path_distance( pc_detection_bounds, sf::Color::Green ); }
  }
}

void RenderPlayerSystem::render_player_footsteps()
{

  // render all footsteps
  auto footstep_view = m_reg->view<Cmp::FootStepTimer, Cmp::FootStepAlpha, Cmp::Position, Cmp::Direction>();
  for ( auto [entity, timer, alpha, pos_cmp, direction] : footstep_view.each() )
  {
    std::size_t new_idx = 0;
    uint8_t new_alpha{ alpha.m_alpha };
    sf::Vector2f new_scale{ 1.f, 1.f };
    // we're changing the origin to be the center of the sprite so that
    // rotation happens around the center, this means we also need to
    // offset the position to make it look convincing depending on direction of movement
    sf::Vector2f new_origin{ BaseSystem::kGridSquareSizePixels.x / 2.f, BaseSystem::kGridSquareSizePixels.y / 2.f };
    sf::FloatRect new_pos{ pos_cmp.position, kGridSquareSizePixelsF };
    // moving in right direction: place footsteps to bottom-left of player position
    if ( direction == sf::Vector2f( 1.f, 0.f ) )
    {
      new_pos.position = { pos_cmp.position.x + ( BaseSystem::kGridSquareSizePixels.x * 0.25f ),
                           pos_cmp.position.y + ( BaseSystem::kGridSquareSizePixels.y * 0.75f ) };
    }
    // moving in left direction: place footsteps to bottom-right of player position
    else if ( direction == sf::Vector2f( -1.f, 0.f ) )
    {
      new_pos.position = { pos_cmp.position.x + ( BaseSystem::kGridSquareSizePixels.x * 0.75f ),
                           pos_cmp.position.y + ( BaseSystem::kGridSquareSizePixels.y * 0.75f ) };
    }
    // moving diagonally: down/left
    else if ( direction == sf::Vector2f( -1.f, 1.f ) )
    {
      new_pos.position = { pos_cmp.position.x + ( BaseSystem::kGridSquareSizePixels.x * 0.75f ),
                           pos_cmp.position.y + ( BaseSystem::kGridSquareSizePixels.y * 0.75f ) };
    }
    // moving diagonally: down/right
    else if ( direction == sf::Vector2f( 1.f, 1.f ) )
    {
      new_pos.position = { pos_cmp.position.x + ( BaseSystem::kGridSquareSizePixels.x * 0.5f ),
                           pos_cmp.position.y + ( BaseSystem::kGridSquareSizePixels.y * 0.8f ) };
    }
    // moving diagonally: up/left
    else if ( direction == sf::Vector2f( -1.f, -1.f ) )
    {
      new_pos.position = { pos_cmp.position.x + ( BaseSystem::kGridSquareSizePixels.x * 0.5f ),
                           pos_cmp.position.y + ( BaseSystem::kGridSquareSizePixels.y * 0.8f ) };
    }
    // moving diagonally: up/right
    else if ( direction == sf::Vector2f( 1.f, -1.f ) )
    {
      new_pos.position = { pos_cmp.position.x + ( BaseSystem::kGridSquareSizePixels.x * 0.5f ),
                           pos_cmp.position.y + ( BaseSystem::kGridSquareSizePixels.y * 0.8f ) };
    }
    // moving in up/down direction: place footsteps to center of player position
    else
    {
      new_pos.position = { pos_cmp.position.x + ( BaseSystem::kGridSquareSizePixels.x * 0.5f ),
                           pos_cmp.position.y + ( BaseSystem::kGridSquareSizePixels.y * 0.5f ) };
    }
    // only set rotation if direction is not zero vector
    sf::Angle new_angle;
    if ( direction != sf::Vector2f( 0.f, 0.f ) ) { new_angle = direction.angle(); }

    safe_render_sprite( "FOOTSTEPS", new_pos, new_idx, new_scale, new_alpha, new_origin, new_angle );
  }
}

void RenderPlayerSystem::render_npc()
{

  for ( auto [entity, npc_cmp, pos_cmp, npc_sb_cmp, dir_cmp, anim_cmp] :
        m_reg->view<Cmp::NPC, Cmp::Position, Cmp::NPCScanBounds, Cmp::Direction, Cmp::SpriteAnimation>().each() )
  {

    // make the NPC gaze follow the players relative direction
    if ( dir_cmp.x > 0 )
    {
      dir_cmp.x_scale = 1.f;
      dir_cmp.x_offset = 0.f;
    }
    else if ( dir_cmp.x < 0 )
    {
      dir_cmp.x_scale = -1.f;
      dir_cmp.x_offset = BaseSystem::kGridSquareSizePixels.x;
    }
    else
    {
      dir_cmp.x_scale = dir_cmp.x_scale; // keep last known direction
      dir_cmp.x_offset = dir_cmp.x_offset;
    }

    auto params_tuple = calc_alive_render_params( npc_cmp.m_type, pos_cmp, dir_cmp, anim_cmp, entity );
    std::apply( [this]( auto &&...tpl_args ) { safe_render_sprite( std::forward<decltype( tpl_args )>( tpl_args )... ); },
                params_tuple );

    // show npc scan distance
    if ( m_show_path_finding ) { render_path_distance( npc_sb_cmp, sf::Color::Red ); }
  }
}

RenderPlayerSystem::RenderParams RenderPlayerSystem::calc_alive_render_params( const Sprites::SpriteMetaType &sprite_type,
                                                                               const Cmp::Position &pos_cmp,
                                                                               const Cmp::Direction &direction,
                                                                               const Cmp::SpriteAnimation &anim_cmp,
                                                                               entt::entity entity )
{
  uint8_t new_alpha = 255;
  int sprite_index = 0;
  if ( sprite_type == "PLAYER" )
  {
    // Use static frame when not moving
    if ( direction == sf::Vector2f( 0.0f, 0.0f ) ) { sprite_index = anim_cmp.m_base_frame; }
    // Use animated frame: base_frame + current_frame
    else { sprite_index = anim_cmp.m_base_frame + anim_cmp.m_current_frame; }
  }
  else
  {
    // for NPCs just use the current animation frame
    sprite_index = anim_cmp.m_current_frame;
  }

  auto *wormhole_jump = m_reg->try_get<Cmp::WormholeJump>( entity );
  if ( wormhole_jump )
  {
    // Calculate fade based on elapsed time vs total cooldown
    float elapsed = wormhole_jump->jump_clock.getElapsedTime().asSeconds();
    float cooldown = wormhole_jump->jump_cooldown.asSeconds();
    float progress = std::min( elapsed / cooldown, 1.0f ); // 0.0 to 1.0

    // Fade from 255 to 0
    new_alpha = m_player_current_alpha = static_cast<uint8_t>( 255 * ( 1.0f - progress ) );
  }
  else { new_alpha = m_player_current_alpha = 255; }

  sf::FloatRect new_pos{ { pos_cmp.position.x + direction.x_offset, pos_cmp.position.y }, kGridSquareSizePixelsF };
  auto new_scale = sf::Vector2f{ direction.x_scale, 1.f };
  sf::Vector2f new_origin{ 0.f, 0.f };
  sf::Angle new_angle = sf::degrees( 0.f );
  return { sprite_type, new_pos, sprite_index, new_scale, new_alpha, new_origin, new_angle };
}

RenderPlayerSystem::RenderParams RenderPlayerSystem::calc_falling_render_params( const Sprites::SpriteMetaType &sprite_type,
                                                                                 const Cmp::Position &pos_cmp,
                                                                                 const Cmp::Direction &direction,
                                                                                 const Cmp::SpriteAnimation &anim_cmp,
                                                                                 Cmp::PlayerMortality &pc_mort_cmp )
{
  int sprite_index = 0;
  sf::Vector2f new_scale{ m_player_current_scale };
  if ( sprite_type == "PLAYER" )
  {
    // Use static frame when not moving
    if ( direction == sf::Vector2f( 0.0f, 0.0f ) ) { sprite_index = anim_cmp.m_base_frame; }
    // Use animated frame: base_frame + current_frame
    else { sprite_index = anim_cmp.m_base_frame + anim_cmp.m_current_frame; }
  }
  else
  {
    // for NPCs just use the current animation frame
    sprite_index = anim_cmp.m_current_frame;
    new_scale = sf::Vector2f{ direction.x_scale, 1.f };
  }

  sf::FloatRect new_pos{ pos_cmp.position + ( kGridSquareSizePixelsF / 2.f ), kGridSquareSizePixelsF };

  uint8_t new_alpha = 255;
  sf::Vector2f new_origin{ kGridSquareSizePixelsF / 2.f }; // center player origin for rotation
  sf::Angle new_angle = m_player_current_angle;

  if ( m_player_death_clock.getElapsedTime() >= m_fall_speed )
  {
    new_angle = m_player_current_angle += sf::radians( 20.f );
    new_scale = m_player_current_scale -= sf::Vector2f{ 0.01f, 0.01f };
    m_player_death_clock.restart();
    pc_mort_cmp.death_progress += 0.01f;
  }

  return { sprite_type, new_pos, sprite_index, new_scale, new_alpha, new_origin, new_angle };
}

void RenderPlayerSystem::render_path_distance( const Cmp::RectBounds &bounds_cmp, const sf::Color &outline_color )
{
  sf::RectangleShape npc_square( bounds_cmp.size() );
  npc_square.setFillColor( sf::Color::Transparent );
  npc_square.setOutlineColor( outline_color );
  npc_square.setOutlineThickness( 1.f );
  npc_square.setPosition( bounds_cmp.position() );
  m_window.draw( npc_square );
}

} // namespace ProceduralMaze::Sys