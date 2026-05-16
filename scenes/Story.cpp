#include "Game.hpp"

// ============================================================================
//  Story Content  ::  "The Last Signal"
//  ---------------------------------------------------------------------------
//  All narrative lives here, separate from the engine. This is the
//  data-driven content layer: writers/designers edit this file; engineers
//  never touch it. Branching uses flags/vars from GameState so paths can
//  converge and diverge naturally.
//
//  Story summary: You are the lone night-shift operator on orbital station
//  KEPLER-9 when an unknown signal arrives. Choices affect 'trust' and
//  'caution' variables and several flags, leading to 4 distinct endings.
// ============================================================================
void Game::buildStory() {
    auto& s = script_;

    // ---- Opening -------------------------------------------------------
    {
        StoryNode n;
        n.id = "start";
        n.speaker = "";
        n.lines = {
            "Orbital Station KEPLER-9. 03:14 station time.",
            "You are alone on the night shift. The hum of life support",
            "is the only sound — until a console blinks crimson.",
            "An unscheduled transmission is bleeding through deep-space band.",
        };
        n.choices = {
            Choice("Open the transmission immediately.", "open_signal",
                   nullptr,
                   [](GameState& g){ g.vars["caution"] -= 1;
                                     g.flags["opened_fast"] = true; }),
            Choice("Run a quarantine scan first.", "scan_signal",
                   nullptr,
                   [](GameState& g){ g.vars["caution"] += 2; }),
            Choice("Wake the captain.", "wake_captain"),
        };
        s.addNode(std::move(n));
    }

    // ---- Branch: open immediately --------------------------------------
    {
        StoryNode n;
        n.id = "open_signal";
        n.lines = {
            "You patch it through. Static — then a voice, layered over",
            "itself, speaking your name. Your full name. Nobody up here",
            "knows it.",
            "\"We have been waiting for someone who would simply listen.\"",
        };
        n.choices = {
            Choice("Answer: 'Who are you?'", "ask_identity",
                   nullptr,
                   [](GameState& g){ g.vars["trust"] += 1; }),
            Choice("Cut the feed and log it.", "cut_feed",
                   nullptr,
                   [](GameState& g){ g.vars["caution"] += 1; }),
        };
        s.addNode(std::move(n));
    }

    // ---- Branch: scan first --------------------------------------------
    {
        StoryNode n;
        n.id = "scan_signal";
        n.lines = {
            "The scan runs for ninety seconds that feel like an hour.",
            "Result: the signal contains no malware, no carrier exploit.",
            "But its structure matches no human encoding standard at all.",
            "It is, the system insists, 'mathematically polite'.",
        };
        n.onEnter = [](GameState& g){ g.flags["scanned"] = true; };
        n.choices = {
            Choice("Now open it, carefully.", "open_signal"),
            Choice("Too strange. Seal the channel.", "seal_channel",
                   nullptr,
                   [](GameState& g){ g.vars["caution"] += 2; }),
        };
        s.addNode(std::move(n));
    }

    // ---- Branch: wake captain ------------------------------------------
    {
        StoryNode n;
        n.id = "wake_captain";
        n.speaker = "Captain Reyes";
        n.lines = {
            "Reyes arrives in three minutes, eyes sharp despite the hour.",
            "\"Good. You didn't touch it. That's the right instinct.\"",
            "\"We do this together, or not at all. Your call, operator.\"",
        };
        n.onEnter = [](GameState& g){ g.flags["captain_awake"] = true;
                                      g.vars["trust"] += 1; };
        n.choices = {
            Choice("Open it together.", "open_signal"),
            Choice("Recommend sealing it.", "seal_channel",
                   nullptr,
                   [](GameState& g){ g.vars["caution"] += 1; }),
        };
        s.addNode(std::move(n));
    }

    // ---- Mid: ask identity ---------------------------------------------
    {
        StoryNode n;
        n.id = "ask_identity";
        n.lines = {
            "\"We are what your kind becomes, far ahead of where you stand.",
            " We sent this echo backward along a thread we do not fully",
            " understand. We need you to make one choice for us.\"",
            "The console offers two coordinates. One pulls the station's",
            "antenna inward, toward Earth. One pushes it outward, to the dark.",
        };
        n.choices = {
            Choice("Aim the reply toward Earth.", "ending_earth",
                   nullptr,
                   [](GameState& g){ g.flags["chose_earth"] = true; }),
            Choice("Aim the reply outward.", "ending_outward",
                   [](const GameState& g){ return g.getVar("trust") >= 1; },
                   [](GameState& g){ g.flags["chose_outward"] = true; }),
            Choice("Refuse to choose. Shut it all down.", "ending_silence"),
        };
        s.addNode(std::move(n));
    }

    // ---- Mid: cut feed -------------------------------------------------
    {
        StoryNode n;
        n.id = "cut_feed";
        n.lines = {
            "You sever the connection. The console goes dark, then calm.",
            "For six hours, nothing. Then every screen on KEPLER-9 lights",
            "with the same three words, gently, without threat:",
            "\"We will wait.\"",
        };
        n.autoNext = "ending_silence";
        s.addNode(std::move(n));
    }

    // ---- Mid: seal channel ---------------------------------------------
    {
        StoryNode n;
        n.id = "seal_channel";
        n.lines = {
            "You hard-seal the channel and file a full incident report.",
            "Procedure followed. Risk contained. Curiosity unanswered.",
        };
        if (true) {
            n.choices = {
                Choice("Stand by your decision.", "ending_duty"),
                Choice("Reopen it before your shift ends.", "open_signal",
                       nullptr,
                       [](GameState& g){ g.vars["caution"] -= 1; }),
            };
        }
        s.addNode(std::move(n));
    }

    // ---- Endings -------------------------------------------------------
    {
        StoryNode n;
        n.id = "ending_earth";
        n.isEnding = true;
        n.lines = {
            "The reply lances down toward home. Days later, every radio",
            "telescope on Earth hears the same patient voice — and humanity,",
            "for the first time, answers as one.",
            "ENDING: 'Homecoming'",
        };
        s.addNode(std::move(n));
    }
    {
        StoryNode n;
        n.id = "ending_outward";
        n.isEnding = true;
        n.lines = {
            "You aim the dish at the dark between stars. The voice softens:",
            "\"Then we will keep this thread for you, until you are ready",
            " to walk it yourselves.\" The signal folds itself away.",
            "ENDING: 'The Long Thread'",
        };
        s.addNode(std::move(n));
    }
    {
        StoryNode n;
        n.id = "ending_silence";
        n.isEnding = true;
        n.lines = {
            "You choose silence. Some doors, once seen, are left closed",
            "on purpose. KEPLER-9 hums on. The night shift continues.",
            "ENDING: 'The Closed Door'",
        };
        s.addNode(std::move(n));
    }
    {
        StoryNode n;
        n.id = "ending_duty";
        n.isEnding = true;
        n.lines = {
            "Your report is commended. The signal is never explained.",
            "You sleep well, mostly. Sometimes you wonder. That is allowed.",
            "ENDING: 'By the Book'",
        };
        s.addNode(std::move(n));
    }
}
