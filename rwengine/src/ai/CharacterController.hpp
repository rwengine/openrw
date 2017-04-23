#pragma once
#ifndef _CHARACTERCONTROLLER_HPP_
#define _CHARACTERCONTROLLER_HPP_
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <string>

struct AIGraphNode;
class CharacterObject;
class VehicleObject;

/**
 * @class CharacterController
 * Character Controller Interface, translates high-level behaviours into low
 * level actions.
 */
class CharacterController {
public:
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

    /**
     * Available AI goals.
     */
    enum Goal {
        /**
         * No goal, will idle or execute external Activities.
         */
        None,
        /**
         * Keep close to leader character
         */
        FollowLeader,
        /**
         * Wander randomly around the map
         */
        TrafficWander
    };

protected:
    /**
     * The character being controlled.
     */
    CharacterObject* character;

    Activity* _currentActivity;
    Activity* _nextActivity;

    bool updateActivity();
    void setActivity(Activity* activity);

    float m_closeDoorTimer;

    // Goal related variables
    Goal currentGoal;
    CharacterObject* leader;
    AIGraphNode* targetNode;

public:
    CharacterController(CharacterObject* character);

    virtual ~CharacterController() {
    }

    Activity* getCurrentActivity() const {
        return _currentActivity;
    }

    Activity* getNextActivity() const {
        return _nextActivity;
    }

    /**
     * @brief skipActivity Cancel the current activity immediatley, if possible.
     */
    void skipActivity();

    /**
     * @brief setNextActivity Sets the next Activity with a parameter.
     * @param activity
     * @param position
     */
    void setNextActivity(Activity* activity);

    /**
     * @brief IsCurrentActivity
     * @param activity Name of activity to check for
     * @return if the given activity is the current activity
     */
    bool isCurrentActivity(const std::string& activity) const;

    /**
     * @brief update Updates the controller.
     * @param dt
     */
    virtual void update(float dt);

    virtual glm::vec3 getTargetPosition() = 0;

    /**
     * @brief
     * @return Returns the Character Object
     */
    CharacterObject* getCharacter() const;

    void setMoveDirection(const glm::vec3& movement);
    void setLookDirection(const glm::vec2& look);

    void setRunning(bool run);

    void setGoal(Goal goal) {
        currentGoal = goal;
    }
    Goal getGoal() const {
        return currentGoal;
    }

    void setTargetCharacter(CharacterObject* c) {
        leader = c;
    }
    CharacterObject* getTargetCharacter() const {
        return leader;
    }
};

#endif
