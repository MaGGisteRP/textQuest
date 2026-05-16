#pragma once

#include <memory>
#include <stack>
#include <vector>

// ============================================================================
//  State Machine
//  ---------------------------------------------------------------------------
//  A pushdown automaton (stack-based FSM) — the standard pattern for game flow
//  in modern engines. States can be pushed (e.g. open Pause over Gameplay)
//  and popped (close Pause, resume Gameplay underneath).
//
//  GameState (the save data) is intentionally a DIFFERENT concept from these
//  control-flow States. We name this one IGameStateFSM to avoid confusion.
// ============================================================================

class Game;  // forward declaration

class IGameStateFSM {
public:
    virtual ~IGameStateFSM() = default;

    virtual void onEnter(Game& /*game*/) {}
    virtual void onExit(Game& /*game*/)  {}

    // One frame/tick of this state. Returns false to request the game to quit.
    virtual bool update(Game& game) = 0;

    virtual const char* name() const = 0;
};

class StateMachine {
public:
    void push(std::unique_ptr<IGameStateFSM> s, Game& game) {
        if (!stack_.empty()) {
            // We don't exit the underlying state — it's paused, not removed.
        }
        s->onEnter(game);
        stack_.push_back(std::move(s));
    }

    void pop(Game& game) {
        if (stack_.empty()) return;
        stack_.back()->onExit(game);
        stack_.pop_back();
    }

    // Replace the entire stack with a single state (e.g. Menu -> Gameplay).
    void replace(std::unique_ptr<IGameStateFSM> s, Game& game) {
        while (!stack_.empty()) {
            stack_.back()->onExit(game);
            stack_.pop_back();
        }
        s->onEnter(game);
        stack_.push_back(std::move(s));
    }

    bool empty() const { return stack_.empty(); }

    IGameStateFSM* current() {
        return stack_.empty() ? nullptr : stack_.back().get();
    }

private:
    std::vector<std::unique_ptr<IGameStateFSM>> stack_;
};
