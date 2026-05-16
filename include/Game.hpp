#pragma once

#include "GameState.hpp"
#include "ScriptEngine.hpp"
#include "SaveManager.hpp"
#include "StateMachine.hpp"
#include "IO.hpp"

#include <chrono>

// ============================================================================
//  Game
//  ---------------------------------------------------------------------------
//  The composition root. It owns and wires together every subsystem and
//  exposes them to states. This is the classic "Game" / "Application" object
//  found in essentially every engine — the single place that holds the
//  lifetime of all systems and runs the main loop.
// ============================================================================
class Game {
public:
    Game()
        : saveManager_("saves") {
        buildStory();
    }

    // --- Subsystem accessors --------------------------------------------
    GameState&    state()        { return state_; }
    ScriptEngine& script()       { return script_; }
    SaveManager&  saves()        { return saveManager_; }
    IO&           io()           { return io_; }
    StateMachine& fsm()          { return fsm_; }

    // --- Main loop -------------------------------------------------------
    void run();

    void requestQuit() { running_ = false; }

    // Accumulates wall-clock playtime into the save data.
    void tickPlaytime() {
        auto now = std::chrono::steady_clock::now();
        auto delta = std::chrono::duration_cast<std::chrono::seconds>(
                         now - lastTick_).count();
        if (delta > 0) {
            state_.playtimeSeconds += static_cast<std::uint64_t>(delta);
            lastTick_ = now;
        }
    }

private:
    void buildStory();   // Defined in Story content file.

    GameState     state_;
    ScriptEngine  script_;
    SaveManager   saveManager_;
    IO            io_;
    StateMachine  fsm_;

    bool running_ = true;
    std::chrono::steady_clock::time_point lastTick_ =
        std::chrono::steady_clock::now();
};
