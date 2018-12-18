#ifndef RWGAME_STATEMANAGER_HPP
#define RWGAME_STATEMANAGER_HPP
#include "State.hpp"

#include <memory>
#include <queue>

/**
 * @brief Handles current state focus and transitions
 *
 * Possible states:
 * 	Foreground (topmost state)
 *  Background (any other position)
 *
 * Transitions:
 *  New State (at the top)
 *  Suspended (a state was created above us)
 *  Resumed   (blocking state was removed)
 */
class StateManager {
public:
    std::deque<std::unique_ptr<State>> states;

    void clear() {
        cleared = true;
    }

    template <class T, class... Targs>
    void enter(Targs&&... args) {
        // Notify the previous state it has been suspended
        if (!states.empty()) {
            states.back()->exit();
        }
        states.emplace_back(std::move(std::make_unique<T>(args...)));
        states.back()->enter();
    }

    void updateStack() {
        if (cleared) {
            states.clear();
            cleared = false;
        }

        while (!states.empty() && states.back()->hasExited()) {
            states.back()->exit();
            states.pop_back();
            if (!states.empty()) {
                states.back()->enter();
            }
        }
    }

    void tick(float dt) {
        states.back()->tick(dt);
    }

    void draw(GameRenderer& r) {
        states.back()->draw(r);
    }

    State* currentState() {
        if (states.empty()) {
            return nullptr;
        }
        return states.back().get();
    }

private:
    bool cleared = false;
};

#endif
