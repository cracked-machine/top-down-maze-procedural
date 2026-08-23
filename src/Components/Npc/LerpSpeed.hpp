#ifndef SRC_COMPONENTS_NPC_LERPSPEED_HPP__
#define SRC_COMPONENTS_NPC_LERPSPEED_HPP__

namespace Game::Cmp::Npc
{

//! @brief Per-entity movement interpolation speed, copied from Cmp::Npc::NPC::m_lerp_speed at spawn
//! time so it can be looked up per-NPC-instance during position lerping.
struct LerpSpeed
{
  //! @brief The interpolation speed used when lerping the entity's rendered position.
  float speed;
};

} // namespace Game::Cmp::Npc

#endif // SRC_COMPONENTS_NPC_LERPSPEED_HPP__
