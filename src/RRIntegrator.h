/// Emil Hedemalm
/// 2016-08-03
/// Integrator. Mostly just sets Y so collisions go to hell.

#include "Physics/Integrators/FirstPersonIntegrator.h"

class RRIntegrator : public FirstPersonIntegrator 
{
public:
	/** All entities sent here should be fully dynamic! 
		Kinematic ones may or may not work (consider adding own integration function).
	*/
	virtual void IntegrateDynamicEntities(List<Entity*> & dynamicEntities, float timeInSeconds)
	{
		FirstPersonIntegrator::IntegrateDynamicEntities(dynamicEntities, timeInSeconds);
		for (int i = 0; i < dynamicEntities.Size(); ++i)
		{
			Entity * e = dynamicEntities[i];
			if (e->worldPosition.y < 0) // quick fix.
				e->localPosition.y += 0.01f;
		}
	}
	/** All entities sent here should be fully kinematic! 
		If not subclassed, the standard IntegrateEntities is called.
	*/
	virtual void IntegrateKinematicEntities(List<Entity*> & kinematicEntities, float timeInSeconds)
	{
		FirstPersonIntegrator::IntegrateKinematicEntities(kinematicEntities, timeInSeconds);
	}

};
