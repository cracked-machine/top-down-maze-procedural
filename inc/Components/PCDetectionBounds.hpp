#ifndef __CMP_PCAGGROBOUNDS_HPP__
#define __CMP_PCAGGROBOUNDS_HPP__

#include <RectBounds.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/System/Vector2.hpp>

namespace ProceduralMaze::Cmp {

class PCDetectionBounds : public RectBounds {
public:
  PCDetectionBounds(sf::Vector2f position, sf::Vector2f size)
      : RectBounds(position, size, 7.f) {}
};

} // namespace ProceduralMaze::Cmp

#endif // __CMP_PCAGGROBOUNDS_HPP__
