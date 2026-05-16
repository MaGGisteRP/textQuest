#pragma once

#include "StateMachine.hpp"
#include <string>

// ============================================================================
//  Concrete States
//  ---------------------------------------------------------------------------
//  Each screen of the game is a state. They communicate with subsystems only
//  through the Game object passed into update(). States are small and focused
//  — a hallmark of maintainable game-flow code.
// ============================================================================

// Title screen: New Game / Continue / Quit.
class MainMenuState : public IGameStateFSM {
public:
    bool update(Game& game) override;
    const char* name() const override { return "MainMenu"; }
};

// The actual novel playback loop.
class GameplayState : public IGameStateFSM {
public:
    bool update(Game& game) override;
    void onEnter(Game& game) override;
    const char* name() const override { return "Gameplay"; }
};

// In-game pause overlay: Resume / Save / Load / Quit to menu.
class PauseState : public IGameStateFSM {
public:
    bool update(Game& game) override;
    const char* name() const override { return "Pause"; }
};

// Slot picker reused for both saving and loading.
class SaveLoadState : public IGameStateFSM {
public:
    enum class Mode { Save, Load };
    explicit SaveLoadState(Mode m) : mode_(m) {}
    bool update(Game& game) override;
    const char* name() const override { return "SaveLoad"; }
private:
    Mode mode_;
};
