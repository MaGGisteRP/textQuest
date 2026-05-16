#include "Game.hpp"
#include "States.hpp"

#include <memory>

// ============================================================================
//  Game::run  ::  the main loop
//  ---------------------------------------------------------------------------
//  A classic engine loop: while the application is alive, tick the top state
//  of the stack. The state machine drives all transitions; the loop itself
//  stays tiny and stable — exactly what you want at the engine core.
// ============================================================================
void Game::run() {
    fsm_.replace(std::make_unique<MainMenuState>(), *this);

    while (running_ && !fsm_.empty()) {
        IGameStateFSM* st = fsm_.current();
        if (!st) break;

        bool keepRunning = st->update(*this);
        if (!keepRunning) {
            running_ = false;
        }
    }

    io_.clear();
    io_.println("Thanks for playing. Goodbye.");
}
