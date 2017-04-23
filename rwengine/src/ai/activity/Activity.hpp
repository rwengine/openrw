#pragma once
#ifndef _ACTIVITY_HPP_
#define _ACTIVITY_HPP_
#include <string>
#include <ai/CharacterController.hpp>
#include <objects/CharacterObject.hpp>

/**
 * @brief The Activity struct interface
 */
struct Activity {
    virtual ~Activity() {
    }

    virtual std::string name() const = 0;

    /**
     * @brief canSkip
     * @return true if the activity can be skipped.
     */
    virtual bool canSkip(CharacterObject*, CharacterController*) const {
        return false;
    }

    virtual bool update(CharacterObject* character,
                        CharacterController* controller) = 0;
};


#define DECL_ACTIVITY(activity_name)                     \
    static constexpr auto ActivityName = #activity_name; \
    std::string name() const {                           \
        return ActivityName;                             \
    }

#endif
