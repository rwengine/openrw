#ifndef _RWENGINE_RAYCASTCALLBACKS_HPP_
#define _RWENGINE_RAYCASTCALLBACKS_HPP_

#ifdef _MSC_VER
#pragma warning(disable : 4305 5033)
#endif
#include <btBulletDynamicsCommon.h>
#ifdef _MSC_VER
#pragma warning(default : 4305 5033)
#endif

/**
 * Implements raycast callback that ignores a specified btCollisionObject
 */
class ClosestNotMeRayResultCallback final
    : public btCollisionWorld::ClosestRayResultCallback {
    btCollisionObject* _self;

public:
    ClosestNotMeRayResultCallback(btCollisionObject* self,
                                  const btVector3& from, const btVector3& to)
        : ClosestRayResultCallback(from, to), _self(self) {
    }

    btScalar addSingleResult(
        btCollisionWorld::LocalRayResult& rayResult, bool normalInWorldSpace) override {
        if (rayResult.m_collisionObject == _self) {
            return 1.0;
        }
        return ClosestRayResultCallback::addSingleResult(rayResult,
                                                         normalInWorldSpace);
    }
};

#endif
