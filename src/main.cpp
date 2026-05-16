#include "Game.hpp"

// ============================================================================
//  Entry Point
//  ---------------------------------------------------------------------------
//  Deliberately minimal. main() does one job: construct the application and
//  hand control to it. All real logic lives in the engine and content layers.
// ============================================================================
int main() {
    try {
        Game game;
        game.run();
    } catch (const std::exception& e) {
        std::cerr << "\nFatal error: " << e.what() << "\n";
        return 1;
    }
    return 0;
}
