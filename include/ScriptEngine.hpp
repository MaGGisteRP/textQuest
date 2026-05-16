#pragma once

#include "Story.hpp"
#include "GameState.hpp"

#include <string>
#include <unordered_map>
#include <stdexcept>

// ============================================================================
//  ScriptEngine
//  ---------------------------------------------------------------------------
//  Owns the story graph and is responsible for graph traversal. It does NOT
//  own game state (that lives in GameState) and does NOT do I/O (that's the
//  renderer's job). This separation mirrors the "systems operate on data"
//  principle of modern ECS-style architectures.
// ============================================================================
class ScriptEngine {
public:
    void addNode(StoryNode node) {
        const std::string id = node.id;
        nodes_[id] = std::move(node);
    }

    bool hasNode(const std::string& id) const {
        return nodes_.find(id) != nodes_.end();
    }

    const StoryNode& getNode(const std::string& id) const {
        auto it = nodes_.find(id);
        if (it == nodes_.end()) {
            throw std::runtime_error("Story node not found: " + id);
        }
        return it->second;
    }

    // Returns the choices that are currently selectable given the state.
    std::vector<const Choice*> availableChoices(const GameState& state) const {
        const StoryNode& node = getNode(state.currentNode);
        std::vector<const Choice*> out;
        for (const Choice& c : node.choices) {
            if (!c.condition || c.condition(state)) {
                out.push_back(&c);
            }
        }
        return out;
    }

    std::size_t nodeCount() const { return nodes_.size(); }

private:
    std::unordered_map<std::string, StoryNode> nodes_;
};
