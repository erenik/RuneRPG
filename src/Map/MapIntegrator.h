/// Evergreen IT AB
/// 2024-03-07

#pragma once

#include "Physics/Integrator.h"

class MapIntegrator : public Integrator {
public:
	// All entities sent here should be fully dynamic!
	virtual void IntegrateDynamicEntities(List< Entity* >& dynamicEntities, float timeInSeconds) override;
	// All entities sent here should be fully kinematic! 
	virtual void IntegrateKinematicEntities(List< Entity* >& kinematicEntities, float timeInSeconds) override;
private:

	void IntegratePosition(List< Entity* >& forEntities, float timeInSeconds);
	void IntegrateVelocity(List< Entity* >& entities, float timeInSeconds);
};