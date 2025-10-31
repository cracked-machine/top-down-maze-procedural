#ifndef __COMPONENTS_PLAYER_SCORE_HPP__
#define __COMPONENTS_PLAYER_SCORE_HPP__

namespace ProceduralMaze::Cmp {

class PlayerScore
{
public:
  PlayerScore( unsigned int score )
      : m_score( score )
  {
  }
  void increment_score( unsigned int score ) { m_score += score; }
  void decrement_score( unsigned int score )
  {
    if ( m_score == 0 ) return;
    m_score -= score;
  }

  unsigned int get_score() const { return m_score; }

private:
  unsigned int m_score;
};

} // namespace ProceduralMaze::Cmp

#endif // __COMPONENTS_PLAYER_SCORE_HPP__