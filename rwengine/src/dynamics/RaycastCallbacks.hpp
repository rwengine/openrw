#pragma once
#include <btBulletDynamicsCommon.h>

/**
 * Implements raycast callback that ignores a specified btCollisionObject
 */
class ClosestNotMeRayResultCallback : public btCollisionWorld::ClosestRayResultCallback
{
	btCollisionObject* _self;
public:

	ClosestNotMeRayResultCallback( btCollisionObject* self, const btVector3& from, const btVector3& to )
		: ClosestRayResultCallback( from, to ), _self( self ) {}

	virtual btScalar addSingleResult(btCollisionWorld::LocalRayResult &rayResult, bool normalInWorldSpace)
	{
		if( rayResult.m_collisionObject == _self ) {
			return 1.0;
		}
		return ClosestRayResultCallback::addSingleResult( rayResult, normalInWorldSpace );
	}
};
