#ifndef __CMP_BASEPERSISTENT_HPP__
#define __CMP_BASEPERSISTENT_HPP__

namespace ProceduralMaze::Cmp::Persistent {

template <typename T> class BasePersistent
{
public:
  BasePersistent( T value_in ) : value( value_in ) {}
  virtual ~BasePersistent() = default;
  T &operator()() { return value; };

protected:
  T value{};
};

} // namespace ProceduralMaze::Cmp::Persistent

#endif // __CMP_BASEPERSISTENT_HPP__
