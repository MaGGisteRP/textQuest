#pragma once

#include <string>
#include <iostream>
#include <vector>
#include <limits>

// ============================================================================
//  IO Layer
//  ---------------------------------------------------------------------------
//  All console reading/writing goes through here. By abstracting I/O behind
//  one class, the rest of the engine never touches std::cin/std::cout
//  directly. This makes the game testable and lets you swap the frontend
//  (e.g. to a GUI or networked client) without touching game logic — the
//  same reason engines separate "presentation" from "simulation".
// ============================================================================
class IO {
public:
    void print(const std::string& s) const { std::cout << s; }
    void println(const std::string& s = "") const { std::cout << s << "\n"; }

    void clear() const {
        // ANSI clear; harmless if the terminal doesn't support it.
        std::cout << "\033[2J\033[1;1H";
    }

    void rule(char c = '-', int width = 64) const {
        std::cout << std::string(width, c) << "\n";
    }

    void typewriter(const std::string& s) const {
        // Kept instant here for portability; a real build could add a delay.
        std::cout << s << "\n";
    }

    void waitForEnter(const std::string& prompt = "[Enter] ") const {
        std::cout << prompt;
        std::string dummy;
        std::getline(std::cin, dummy);
    }

    // Reads an integer in [min, max]. Re-prompts on bad input.
    int readChoice(int min, int max, const std::string& prompt = "> ") const {
        while (true) {
            std::cout << prompt;
            std::string raw;
            if (!std::getline(std::cin, raw)) {
                return min;  // EOF: fail safe
            }
            try {
                size_t pos = 0;
                int value = std::stoi(raw, &pos);
                if (pos == trimmedLen(raw) && value >= min && value <= max) {
                    return value;
                }
            } catch (...) {
                // fallthrough to error message
            }
            std::cout << "  (Enter a number between " << min
                      << " and " << max << ")\n";
        }
    }

    std::string readLine(const std::string& prompt = "> ") const {
        std::cout << prompt;
        std::string s;
        std::getline(std::cin, s);
        return s;
    }

private:
    static size_t trimmedLen(const std::string& s) {
        size_t end = s.size();
        while (end > 0 && (s[end - 1] == ' ' || s[end - 1] == '\r'
                           || s[end - 1] == '\n' || s[end - 1] == '\t'))
            --end;
        return end;
    }
};
