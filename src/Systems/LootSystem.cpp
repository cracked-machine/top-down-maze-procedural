#include <Systems/LootSystem.hpp>

namespace ProceduralMaze::Sys {

LootSystem::LootSystem( ProceduralMaze::SharedEnttRegistry reg, sf::RenderWindow &window,
                        Sprites::SpriteFactory &sprite_factory )
    : BaseSystem( reg, window, sprite_factory )
{
  std::ignore = getEventDispatcher()
                    .sink<Events::LootContainerDestroyedEvent>()
                    .connect<&LootSystem::on_loot_container_destroyed>( this );
  SPDLOG_DEBUG( "LootSystem initialized" );
}

void LootSystem::detonate_loot_container( const Events::LootContainerDestroyedEvent &event )
{
  // the loot container is now destroyed by the bomb, replace with a random loot component
  auto [obstacle_type, random_obstacle_texture_index] = m_sprite_factory.get_random_type_and_texture_index(
      std::vector<std::string>{ "EXTRA_HEALTH", "EXTRA_BOMBS", "INFINI_BOMBS", "CHAIN_BOMBS", "WEAPON_BOOST" } );
  m_reg->remove<Cmp::LootContainer>( event.m_entity );
  m_reg->remove<Cmp::ReservedPosition>( event.m_entity );
  m_reg->emplace_or_replace<Cmp::Loot>( event.m_entity, obstacle_type, random_obstacle_texture_index );
  if ( m_break_pot_sound_player.getStatus() == sf::Sound::Status::Stopped ) { m_break_pot_sound_player.play(); }
}

} // namespace ProceduralMaze::Sys