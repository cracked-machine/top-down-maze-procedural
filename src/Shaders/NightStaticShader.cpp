#include <Components/Crypt/RoomLavaPit.hpp>
#include <Components/Crypt/RoomLavaPitCell.hpp>
#include <Components/Npc/Watchman.hpp>
#include <Components/Npc/WatchmanSearchlight.hpp>
#include <Components/Npc/Wisp.hpp>
#include <Components/Persistent/DisplayResolution.hpp>
#include <Components/Persistent/NpcWatchmanConeHalfAngle.hpp>
#include <Components/Persistent/NpcWatchmanConeLength.hpp>
#include <Components/Player/TorchRadius.hpp>
#include <Components/Position.hpp>
#include <Shaders/UniformBuilder.hpp>
#include <Systems/ParticleSystem.hpp>
#include <Systems/PersistSystem.hpp>
#include <Systems/Render/RenderSystem.hpp>
#include <Utils/Constants.hpp>
#include <Utils/Optimizations.hpp>
#include <Utils/Player.hpp>

#include <Shaders/NightStaticShader.hpp>

namespace Game::Sprites
{

void NightStaticShader::update( entt::registry &reg )
{
  sf::Vector2u display_size = Sys::PersistSystem::get<Cmp::Persist::DisplayResolution>( reg );
  sf::Vector2f view_center = Sys::RenderSystem::get_world_view().getCenter();
  sf::Vector2f view_size = Sys::RenderSystem::get_world_view().getSize();
  sf::Vector2f view_top_left = { view_center.x - ( view_size.x / 2.f ), view_center.y - ( view_size.y / 2.f ) };

  std::vector<sf::Vector2f> torch_positions;

  // add the radius for each flame particle sprite so the shader can render a circle of light
  for ( auto [candle_entt, particle_cmp] : reg.view<Sys::ParticleSpriteOwner>().each() )
  {
    auto ps_tag = particle_cmp.sprite->get_tag();
    if ( not ps_tag.contains( "particle.flame" ) and not ps_tag.contains( "particle.active" ) ) continue;
    if ( particle_cmp.sprite->get_view_type() != Cmp::Particle::ViewType::WORLD )
      continue; // skip UI-space particles (e.g. candle shown in inventory icon)
    torch_positions.push_back( particle_cmp.sprite->get_emitter_position() );
  }

  // flame particle sprites are paused when candle is in inventory so we need to add the radius explicitly
  auto [_, inventory_type, _] = Utils::Player::get_inventory( reg );
  if ( inventory_type.contains( "candle" ) ) { torch_positions.push_back( Utils::Player::get_position( reg ).getCenter() ); }

  for ( auto [npc_entt, npc_wisp_cmp, npc_pos_cmp] : reg.view<Cmp::Npc::Wisp, Cmp::Position>().each() )
  {
    torch_positions.push_back( npc_pos_cmp.getCenter() );
  }

  // add light for lavapit cells
  for ( auto [lava_entt, lava_cmp, zorder_cmp] : reg.view<Cmp::Crypt::RoomLavaPitCell, Cmp::ZOrderValue>().each() )
  {
    torch_positions.push_back( lava_cmp.getCenter() );
  }

  // Watchman searchlight cones — apex position + current sweep direction, computed each tick by
  // WatchmanSystem::update_searchlights so gameplay hit-testing and the rendered cone always agree
  std::vector<sf::Vector2f> npc_cone_positions;
  std::vector<sf::Vector2f> npc_cone_directions;
  for ( auto [npc_entt, watchman_cmp, npc_pos_cmp, searchlight_cmp] :
        reg.view<Cmp::Npc::Watchman, Cmp::Position, Cmp::Npc::WatchmanSearchlight>().each() )
  {
    npc_cone_positions.emplace_back( npc_pos_cmp.getCenter().x, npc_pos_cmp.getCenter().y + 3.f );
    npc_cone_directions.push_back( searchlight_cmp.cone_direction );
  }

  float fear_amount = static_cast<float>( Utils::Player::get_player_stats( reg ).fear() ) / 100.f;

  Sprites::UniformBuilder{}
      .set( "resolution", sf::Vector2f{ display_size } )
      .set( "time", elapsed().asSeconds() )
      .set( "view_top_left", view_top_left )
      .set( "view_size", view_size )
      .set( "torch_count", static_cast<int>( torch_positions.size() ) )
      .set( "torch_world_pos", torch_positions )
      .set( "torch_radius", Utils::Player::get_torch_radius( reg ).value )
      .set( "npc_count", static_cast<int>( npc_cone_positions.size() ) )
      .set( "npc_positions", npc_cone_positions )
      .set( "npc_directions", npc_cone_directions )
      .set( "npc_torch_length", Sys::PersistSystem::get<Cmp::Persist::NpcWatchmanConeLength>( reg ).get_value() )
      .set( "npc_torch_angle", Sys::PersistSystem::get<Cmp::Persist::NpcWatchmanConeHalfAngle>( reg ).get_value() )
      .set( "fear", fear_amount )
      .apply( &get_shader() );

  // shader position at the top left of the world
  set_position( { 0, 0 } );
}

} // namespace Game::Sprites