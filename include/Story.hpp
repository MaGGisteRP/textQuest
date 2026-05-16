#pragma once

#include <string>
#include <vector>
#include <functional>

class GameState;

// ============================================================================
//  Story Graph
//  ---------------------------------------------------------------------------
//  A visual novel is fundamentally a directed graph of narrative nodes.
//  Each StoryNode shows text, then either:
//    (a) offers the player a set of Choices (branching), or
//    (b) auto-advances to a single next node (linear), or
//    (c) is terminal (an ending).
//
//  Effects and conditions are stored as std::function so the data layer
//  stays decoupled from the script that wires gameplay logic. This is the
//  data-driven approach used by modern narrative engines (Ink, Yarn, Twine).
// ============================================================================

// A condition: given the state, should this choice be visible/available?
using Condition = std::function<bool(const GameState&)>;

// An effect: mutate the state when this path is taken.
using Effect = std::function<void(GameState&)>;

struct Choice {
    std::string text;          // What the player sees
    std::string targetNode;    // Where this choice leads
    Condition   condition;     // Optional gate (nullptr = always available)
    Effect      effect;        // Optional side effect when chosen

    Choice(std::string t, std::string target,
           Condition cond = nullptr, Effect eff = nullptr)
        : text(std::move(t)), targetNode(std::move(target)),
          condition(std::move(cond)), effect(std::move(eff)) {}
};

struct StoryNode {
    std::string id;
    std::string speaker;        // "" for narration, otherwise character name
    std::vector<std::string> lines;  // Text shown line-by-line

    std::vector<Choice> choices;     // Empty => linear or ending
    std::string autoNext;            // Used when there are no choices
    bool isEnding = false;

    Effect onEnter;             // Runs when the node is reached

    bool isTerminal() const {
        return isEnding || (choices.empty() && autoNext.empty());
    }
};
