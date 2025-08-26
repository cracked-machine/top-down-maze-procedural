#ifndef __COMPONENTS_GAME_STATE_HPP__
#define __COMPONENTS_GAME_STATE_HPP__

namespace ProceduralMaze::Cmp {

class GameState {
public:
    GameState() = default;
    ~GameState() = default;

    enum class State {
        MENU,
        PLAYING,
        PAUSED,
        GAME_OVER,
        VICTORY
    };

    State current_state = State::MENU;
};

} // namespace ProceduralMaze::Cmp

#endif // __COMPONENTS_GAME_STATE_HPP__