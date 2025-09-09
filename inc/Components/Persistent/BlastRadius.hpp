#ifndef __CMP_BLASTRADIUS_HPP__
#define __CMP_BLASTRADIUS_HPP__

namespace ProceduralMaze::Cmp::Persistent {

class BlastRadius
{
public:
  int &operator()() { return blast_radius; }

private:
  int blast_radius = 1;
};

} // namespace ProceduralMaze::Cmp::Persistent

#endif // __CMP_BLASTRADIUS_HPP__
