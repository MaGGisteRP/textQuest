# The Last Signal — C++ Text Adventure Engine

A text/visual-novel game in modern C++17 with a save-and-continue system,
built on an architecture that mirrors real game engines.

## Build & Run

```bash
# Option A: CMake (recommended)
cmake -B build && cmake --build build
./build/novel

# Option B: Make
make run

# Option C: direct
g++ -std=c++17 -O2 -Iinclude src/*.cpp scenes/*.cpp -o novel && ./novel
```

## Architecture

The codebase follows the layering used by modern engines (Unity / Unreal /
Godot): a stable engine core, a data-driven content layer, and pure data.

```
                +-------------------+
                |       main        |   entry point only
                +-------------------+
                          |
                +-------------------+
                |       Game        |   composition root: owns all systems,
                |  (main loop)      |   runs the loop
                +-------------------+
                  |   |   |   |   |
   +--------------+   |   |   |   +------------------+
   |                  |   |   |                      |
+--------+   +-------------+ +-----------+   +----------------+
|   IO   |   |StateMachine | |SaveManager|   |  ScriptEngine  |
|(render/|   | (pushdown   | |(serialize |   | (story graph + |
| input) |   |   FSM)      | | to disk)  |   |  traversal)    |
+--------+   +-------------+ +-----------+   +----------------+
                   |                                  |
            +-------------+                  +-----------------+
            |   States    |                  |   GameState     |
            | Menu/Play/  |  operate on -->  | (flags, vars,   |
            | Pause/Save  |                  |  current node)  |
            +-------------+                  +-----------------+
                                                      ^
                                             scenes/Story.cpp
                                          (data-driven content)
```

### Key design decisions

- **Pushdown state machine** (`StateMachine`): the standard game-flow
  pattern. Pause is *pushed* over Gameplay so the session underneath is
  preserved, not destroyed.

- **Data-driven story graph** (`ScriptEngine` + `Story.hpp`): the narrative
  is a directed graph of `StoryNode`s with conditional `Choice`s. Branching
  conditions and effects are `std::function`s, so the content layer
  (`scenes/Story.cpp`) is fully decoupled from the engine. Writers edit
  the story without touching engine code.

- **Single source of truth** (`GameState`): all mutable session data lives
  in one serializable struct. The "breakpoint" for save/continue is just
  `GameState::currentNode` plus the flags/vars map.

- **Decoupled I/O** (`IO`): no engine code touches `std::cin`/`cout`
  directly, so the frontend could be swapped (GUI, network) without
  changing game logic.

- **Versioned, human-readable saves** (`SaveManager`): line-based key/value
  format, escape-safe, forward-tolerant (unknown keys ignored), with a
  `VERSION` field reserved for save migration.

## Save / Continue

- 3 save slots, plus the engine reuses the same slot UI for Save and Load.
- Saving captures the exact story node + all flags/variables + playtime.
- Loading drops straight back into gameplay at the saved node, correctly
  restoring branching state (e.g. choices gated behind `trust >= 1`).
- Saves live in `./saves/slotN.sav` and are plain text for easy debugging.

## Extending the story

Open `scenes/Story.cpp`. Add a `StoryNode`, give it an `id`, `lines`, and
either `choices` (branching), an `autoNext` (linear), or `isEnding = true`.
Conditions/effects let you read and write `GameState` flags and vars.
No engine recompilation logic required — it's pure content.
```cpp
StoryNode n;
n.id = "my_node";
n.lines = { "Some narration." };
n.choices = {
    Choice("Brave option", "next_node",
           [](const GameState& g){ return g.getVar("trust") >= 2; },
           [](GameState& g){ g.flags["was_brave"] = true; }),
};
s.addNode(std::move(n));
```
```
File layout:
  include/   engine headers (data + systems)
  src/       engine implementation + entry point
  scenes/    story content (data-driven)
```
