
#include <Components/Neighbours.hpp>

namespace ProceduralMaze::Cmp
{

void Neighbours::set( Dir dir, entt::entity entity_u32 ) { m_entities[dir] = entity_u32; }

std::optional<entt::entity> Neighbours::get( Dir dir )
{
  if ( m_entities.find( dir ) != m_entities.end() ) { return m_entities[dir]; }
  else { return std::nullopt; }
}

std::string Neighbours::to_string( Dir dir ) const
{
  std::string result;
  if ( dir == Dir::UP )
    result = "UP";
  else if ( dir == Dir::DOWN )
    result = "DOWN";
  else if ( dir == Dir::LEFT )
    result = "LEFT";
  else if ( dir == Dir::RIGHT )
    result = "RIGHT";
  else if ( dir == Dir::UP_LEFT )
    result = "UP_LEFT";
  else if ( dir == Dir::UP_RIGHT )
    result = "UP_RIGHT";
  else if ( dir == Dir::DOWN_LEFT )
    result = "DOWN_LEFT";
  else if ( dir == Dir::DOWN_RIGHT )
    result = "DOWN_RIGHT";
  else
    SPDLOG_ERROR( "Unknown direction: {}", static_cast<int>( dir ) );
  return result;
}
void Neighbours::clear() { m_entities.clear(); }

void Neighbours::remove( Dir dir )
{
  if ( m_entities.find( dir ) != m_entities.end() ) { m_entities.erase( dir ); }
  else { SPDLOG_WARN( "Attempted to remove non-existent entity in direction: {}", static_cast<int>( dir ) ); }
}

int Neighbours::count() const { return m_entities.size(); }

auto Neighbours::begin() { return m_entities.begin(); }
auto Neighbours::end() { return m_entities.end(); }

} // namespace ProceduralMaze::Cmp
