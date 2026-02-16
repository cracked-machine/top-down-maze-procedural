#include <Factory/MultiblockFactory.hpp>

// Include all the actual component definitions
#include <Components/Altar/AltarMultiBlock.hpp>
#include <Components/Altar/AltarSegment.hpp>
#include <Components/Crypt/CryptEntrance.hpp>
#include <Components/Crypt/CryptInteriorMultiBlock.hpp>
#include <Components/Crypt/CryptInteriorSegment.hpp>
#include <Components/Crypt/CryptMultiBlock.hpp>
#include <Components/Crypt/CryptObjectiveMultiBlock.hpp>
#include <Components/Crypt/CryptObjectiveSegment.hpp>
#include <Components/Crypt/CryptSegment.hpp>
#include <Components/Grave/GraveMultiBlock.hpp>
#include <Components/Grave/GraveSegment.hpp>
#include <Components/HolyWell/HolyWellEntrance.hpp>
#include <Components/HolyWell/HolyWellMultiBlock.hpp>
#include <Components/HolyWell/HolyWellSegment.hpp>
#include <Components/Ruin/RuinBuildingMultiBlock.hpp>
#include <Components/Ruin/RuinEntrance.hpp>
#include <Components/Ruin/RuinSegment.hpp>
#include <Components/Ruin/RuinStairsSegment.hpp>
#include <Sprites/MultiSprite.hpp>

namespace ProceduralMaze::Factory
{

// clang-format off
// Explicit instantiation definitions
// template void createMultiblock<Cmp::AltarMultiBlock>( entt::registry &, entt::entity, Cmp::Position, const Sprites::MultiSprite &, int );
// template void createMultiblock<Cmp::CryptMultiBlock>( entt::registry &, entt::entity, Cmp::Position, const Sprites::MultiSprite &, int );
// template void createMultiblock<Cmp::CryptInteriorMultiBlock>( entt::registry &, entt::entity, Cmp::Position, const Sprites::MultiSprite &, int );
// template void createMultiblock<Cmp::CryptObjectiveMultiBlock>( entt::registry &, entt::entity, Cmp::Position, const Sprites::MultiSprite &, int );
// template void createMultiblock<Cmp::GraveMultiBlock>( entt::registry &, entt::entity, Cmp::Position, const Sprites::MultiSprite &, int );
// template void createMultiblock<Cmp::HolyWellMultiBlock>( entt::registry &, entt::entity, Cmp::Position, const Sprites::MultiSprite &, int );
// template void createMultiblock<Cmp::RuinBuildingMultiBlock>( entt::registry &, entt::entity, Cmp::Position, const Sprites::MultiSprite &, int );

// template void createMultiblockSegments<Cmp::AltarMultiBlock, Cmp::AltarSegment>( entt::registry &, entt::entity, Cmp::Position, const Sprites::MultiSprite & );
// template void createMultiblockSegments<Cmp::CryptMultiBlock, Cmp::CryptSegment>( entt::registry &, entt::entity, Cmp::Position, const Sprites::MultiSprite & );
// template void createMultiblockSegments<Cmp::CryptInteriorMultiBlock, Cmp::CryptInteriorSegment>( entt::registry &, entt::entity, Cmp::Position, const Sprites::MultiSprite & );
// template void createMultiblockSegments<Cmp::CryptObjectiveMultiBlock, Cmp::CryptObjectiveSegment>( entt::registry &, entt::entity, Cmp::Position, const Sprites::MultiSprite & );
// template void createMultiblockSegments<Cmp::GraveMultiBlock, Cmp::GraveSegment>( entt::registry &, entt::entity, Cmp::Position, const Sprites::MultiSprite & );
// template void createMultiblockSegments<Cmp::HolyWellMultiBlock, Cmp::HolyWellSegment>( entt::registry &, entt::entity, Cmp::Position, const Sprites::MultiSprite & );
// template void createMultiblockSegments<Cmp::RuinBuildingMultiBlock, Cmp::RuinSegment>( entt::registry &, entt::entity, Cmp::Position, const Sprites::MultiSprite & );
// clang-format on

} // namespace ProceduralMaze::Factory