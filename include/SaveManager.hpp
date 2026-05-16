#pragma once

#include "GameState.hpp"

#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <filesystem>
#include <optional>

// ============================================================================
//  SaveManager
//  ---------------------------------------------------------------------------
//  Handles persistence: serializing GameState to disk and back. This gives us
//  the "save and continue from a breakpoint" feature — the breakpoint is
//  simply GameState::currentNode plus all flags/vars.
//
//  Format: a simple line-based key/value text format. It's:
//    - human-readable & debuggable (important for game dev)
//    - dependency-free (no JSON lib needed)
//    - forward-tolerant (unknown keys are skipped)
//
//  A production engine would version this and likely use a binary or JSON
//  format; the structure here is intentionally easy to extend.
// ============================================================================
class SaveManager {
public:
    explicit SaveManager(std::string saveDir = "saves")
        : saveDir_(std::move(saveDir)) {
        std::filesystem::create_directories(saveDir_);
    }

    static constexpr int kSaveVersion = 1;

    // Build a slot file path like "saves/slot1.sav".
    std::string slotPath(int slot) const {
        return saveDir_ + "/slot" + std::to_string(slot) + ".sav";
    }

    bool slotExists(int slot) const {
        return std::filesystem::exists(slotPath(slot));
    }

    // --- Serialization ---------------------------------------------------
    bool save(const GameState& state, int slot) const {
        std::ofstream out(slotPath(slot), std::ios::trunc);
        if (!out) return false;

        out << "VERSION=" << kSaveVersion << "\n";
        out << "SAVENAME=" << escape(state.saveName) << "\n";
        out << "NODE=" << escape(state.currentNode) << "\n";
        out << "PLAYTIME=" << state.playtimeSeconds << "\n";

        for (const auto& [k, v] : state.flags)
            out << "FLAG=" << escape(k) << "|" << (v ? 1 : 0) << "\n";

        for (const auto& [k, v] : state.vars)
            out << "VAR=" << escape(k) << "|" << v << "\n";

        for (const auto& [k, v] : state.strings)
            out << "STR=" << escape(k) << "|" << escape(v) << "\n";

        return out.good();
    }

    // --- Deserialization -------------------------------------------------
    std::optional<GameState> load(int slot) const {
        std::ifstream in(slotPath(slot));
        if (!in) return std::nullopt;

        GameState state;
        state.flags.clear();
        state.vars.clear();
        state.strings.clear();

        std::string line;
        while (std::getline(in, line)) {
            if (line.empty()) continue;
            auto eq = line.find('=');
            if (eq == std::string::npos) continue;

            std::string key = line.substr(0, eq);
            std::string val = line.substr(eq + 1);

            if (key == "VERSION") {
                // Reserved for migration logic across versions.
            } else if (key == "SAVENAME") {
                state.saveName = unescape(val);
            } else if (key == "NODE") {
                state.currentNode = unescape(val);
            } else if (key == "PLAYTIME") {
                state.playtimeSeconds = std::stoull(val);
            } else if (key == "FLAG") {
                auto [k, v] = splitPair(val);
                state.flags[unescape(k)] = (v == "1");
            } else if (key == "VAR") {
                auto [k, v] = splitPair(val);
                state.vars[unescape(k)] = std::stoi(v);
            } else if (key == "STR") {
                auto [k, v] = splitPair(val);
                state.strings[unescape(k)] = unescape(v);
            }
            // Unknown keys are silently ignored (forward compatibility).
        }
        return state;
    }

    // Metadata preview without loading the whole session into the game.
    struct SlotInfo {
        bool exists = false;
        std::string saveName;
        std::string node;
        std::uint64_t playtime = 0;
    };

    SlotInfo peek(int slot) const {
        SlotInfo info;
        if (!slotExists(slot)) return info;
        auto loaded = load(slot);
        if (!loaded) return info;
        info.exists   = true;
        info.saveName = loaded->saveName;
        info.node     = loaded->currentNode;
        info.playtime = loaded->playtimeSeconds;
        return info;
    }

private:
    std::string saveDir_;

    // Escape newline and pipe so they don't break the line format.
    static std::string escape(const std::string& s) {
        std::string out;
        for (char c : s) {
            if (c == '\\') out += "\\\\";
            else if (c == '\n') out += "\\n";
            else if (c == '|') out += "\\p";
            else out += c;
        }
        return out;
    }

    static std::string unescape(const std::string& s) {
        std::string out;
        for (std::size_t i = 0; i < s.size(); ++i) {
            if (s[i] == '\\' && i + 1 < s.size()) {
                char n = s[++i];
                if (n == '\\') out += '\\';
                else if (n == 'n') out += '\n';
                else if (n == 'p') out += '|';
                else out += n;
            } else {
                out += s[i];
            }
        }
        return out;
    }

    static std::pair<std::string, std::string> splitPair(const std::string& s) {
        auto bar = s.find('|');
        if (bar == std::string::npos) return {s, ""};
        return {s.substr(0, bar), s.substr(bar + 1)};
    }
};
