#include "States.hpp"
#include "Game.hpp"

#include <memory>

// ============================================================================
//  State Implementations
// ============================================================================

namespace {
    std::string formatTime(std::uint64_t secs) {
        std::uint64_t h = secs / 3600;
        std::uint64_t m = (secs % 3600) / 60;
        std::uint64_t s = secs % 60;
        char buf[32];
        std::snprintf(buf, sizeof(buf), "%02llu:%02llu:%02llu",
                      (unsigned long long)h,
                      (unsigned long long)m,
                      (unsigned long long)s);
        return buf;
    }
}

// ---------------------------------------------------------------------------
//  MainMenuState
// ---------------------------------------------------------------------------
bool MainMenuState::update(Game& game) {
    IO& io = game.io();
    io.clear();
    io.rule('=');
    io.println("            T H E   L A S T   S I G N A L");
    io.println("              a text adventure  ::  v1.0");
    io.rule('=');
    io.println();
    io.println("  1) New Game");
    io.println("  2) Continue / Load");
    io.println("  3) About");
    io.println("  4) Quit");
    io.println();

    int choice = io.readChoice(1, 4);

    switch (choice) {
        case 1: {
            game.state().reset();
            game.fsm().replace(std::make_unique<GameplayState>(), game);
            break;
        }
        case 2: {
            game.fsm().push(
                std::make_unique<SaveLoadState>(SaveLoadState::Mode::Load),
                game);
            break;
        }
        case 3: {
            io.clear();
            io.rule();
            io.println(" Built on a data-driven story-graph engine with a");
            io.println(" pushdown state machine and slot-based save system.");
            io.println(" Press Enter to return.");
            io.rule();
            io.waitForEnter();
            break;
        }
        case 4:
            game.requestQuit();
            return false;
    }
    return true;
}

// ---------------------------------------------------------------------------
//  GameplayState
// ---------------------------------------------------------------------------
void GameplayState::onEnter(Game& game) {
    // Ensure the current node is valid; fall back to start otherwise.
    if (!game.script().hasNode(game.state().currentNode)) {
        game.state().currentNode = "start";
    }
}

bool GameplayState::update(Game& game) {
    IO& io = game.io();
    GameState& gs = game.state();
    ScriptEngine& script = game.script();

    game.tickPlaytime();

    const StoryNode& node = script.getNode(gs.currentNode);

    // Run the node's enter-effect exactly once per visit.
    if (node.onEnter) node.onEnter(gs);

    io.clear();
    io.rule();
    if (!node.speaker.empty()) {
        io.println("  " + node.speaker + ":");
    }
    for (const std::string& line : node.lines) {
        io.typewriter("  " + line);
    }
    io.rule();

    // Terminal node => ending screen.
    if (node.isTerminal()) {
        io.println();
        io.println("  *** THE END ***");
        io.println("  Playtime: " + formatTime(gs.playtimeSeconds));
        io.println();
        io.waitForEnter("  Press Enter to return to the main menu... ");
        game.fsm().replace(std::make_unique<MainMenuState>(), game);
        return true;
    }

    // Collect available choices + always-available system commands.
    auto choices = script.availableChoices(gs);

    io.println();
    int index = 1;
    for (const Choice* c : choices) {
        io.println("  " + std::to_string(index++) + ") " + c->text);
    }
    io.println();
    int pauseOption = index;
    io.println("  " + std::to_string(pauseOption) + ") [ Menu / Save ]");

    int picked = io.readChoice(1, pauseOption);

    if (picked == pauseOption) {
        game.fsm().push(std::make_unique<PauseState>(), game);
        return true;
    }

    const Choice* chosen = choices[picked - 1];

    // Apply the choice's side effect, then move the breakpoint forward.
    if (chosen->effect) chosen->effect(gs);
    gs.currentNode = chosen->targetNode;

    // If the destination is a linear chain, allow auto-advance handled
    // naturally on the next update() call.
    if (script.hasNode(gs.currentNode)) {
        const StoryNode& next = script.getNode(gs.currentNode);
        if (next.choices.empty() && !next.autoNext.empty()
            && !next.isTerminal()) {
            // We still render it next frame; autoNext is resolved there.
        }
    }
    return true;
}

// ---------------------------------------------------------------------------
//  PauseState
// ---------------------------------------------------------------------------
bool PauseState::update(Game& game) {
    IO& io = game.io();
    io.clear();
    io.rule('=');
    io.println("                   P A U S E D");
    io.rule('=');
    io.println();
    io.println("  1) Resume");
    io.println("  2) Save game");
    io.println("  3) Load game");
    io.println("  4) Quit to main menu");
    io.println();

    int choice = io.readChoice(1, 4);

    switch (choice) {
        case 1:
            game.fsm().pop(game);
            break;
        case 2:
            game.fsm().push(
                std::make_unique<SaveLoadState>(SaveLoadState::Mode::Save),
                game);
            break;
        case 3:
            game.fsm().push(
                std::make_unique<SaveLoadState>(SaveLoadState::Mode::Load),
                game);
            break;
        case 4:
            game.fsm().replace(std::make_unique<MainMenuState>(), game);
            break;
    }
    return true;
}

// ---------------------------------------------------------------------------
//  SaveLoadState
// ---------------------------------------------------------------------------
bool SaveLoadState::update(Game& game) {
    IO& io = game.io();
    SaveManager& sm = game.saves();

    constexpr int kSlots = 3;

    io.clear();
    io.rule('=');
    io.println(mode_ == Mode::Save ? "                S A V E   G A M E"
                                   : "                L O A D   G A M E");
    io.rule('=');
    io.println();

    for (int i = 1; i <= kSlots; ++i) {
        SaveManager::SlotInfo info = sm.peek(i);
        std::string label = "  " + std::to_string(i) + ") Slot " +
                            std::to_string(i) + " :: ";
        if (info.exists) {
            label += info.saveName + "  [" + info.node + "]  " +
                     formatTime(info.playtime);
        } else {
            label += "<empty>";
        }
        io.println(label);
    }
    io.println();
    io.println("  " + std::to_string(kSlots + 1) + ") Back");
    io.println();

    int choice = io.readChoice(1, kSlots + 1);

    if (choice == kSlots + 1) {
        game.fsm().pop(game);
        return true;
    }

    if (mode_ == Mode::Save) {
        game.tickPlaytime();
        game.state().saveName =
            io.readLine("  Name this save: ");
        if (game.state().saveName.empty())
            game.state().saveName = "Save " + std::to_string(choice);

        bool ok = sm.save(game.state(), choice);
        io.println(ok ? "  Saved successfully." : "  Save FAILED.");
        io.waitForEnter();
        game.fsm().pop(game);
    } else {
        auto loaded = sm.load(choice);
        if (!loaded) {
            io.println("  Nothing to load in that slot.");
            io.waitForEnter();
            return true;
        }
        game.state() = *loaded;

        // Loading must drop straight into gameplay at the saved breakpoint,
        // discarding any menu/pause stack above it.
        game.fsm().replace(std::make_unique<GameplayState>(), game);
        io.println("  Loaded. Resuming from: " + game.state().currentNode);
        io.waitForEnter();
    }
    return true;
}
