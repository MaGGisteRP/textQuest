#pragma once

#include <string>
#include <unordered_map>
#include <cstdint>

// ============================================================================
//  GameState
//  ---------------------------------------------------------------------------
//  Holds the entire mutable state of a play session. This is the single source
//  of truth that gets serialized to disk for save/continue functionality.
//
//  Modern engines (Unity/Unreal/Godot) separate "world data" from "engine
//  systems". GameState is our world-data container: pure data, no logic.
// ============================================================================
class GameState {
public:
    // The story node the player is currently at. This IS the "breakpoint".
    std::string currentNode = "start";

    // Story flags (e.g. "met_alice", "has_key"). Branching reads these.
    std::unordered_map<std::string, bool> flags;

    // Numeric variables (e.g. "reputation", "gold", "trust_level").
    std::unordered_map<std::string, int> vars;

    // Free-form string memory (e.g. "player_name").
    std::unordered_map<std::string, std::string> strings;

    // Playthrough metadata.
    std::uint64_t playtimeSeconds = 0;
    std::string saveName = "Autosave";

    // --- Convenience accessors -------------------------------------------
    bool getFlag(const std::string& key) const {
        auto it = flags.find(key);
        return it != flags.end() && it->second;
    }

    int getVar(const std::string& key) const {
        auto it = vars.find(key);
        return it != vars.end() ? it->second : 0;
    }

    std::string getString(const std::string& key) const {
        auto it = strings.find(key);
        return it != strings.end() ? it->second : std::string{};
    }

    void reset() {
        currentNode = "start";
        flags.clear();
        vars.clear();
        strings.clear();
        playtimeSeconds = 0;
        saveName = "Autosave";
    }
};
