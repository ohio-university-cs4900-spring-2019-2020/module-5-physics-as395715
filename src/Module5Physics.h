#pragma once

#include <WO.h>
#include "PxActor.h"

class Module5Physics : public Aftr::WO {
public:

	static Module5Physics* New(physx::PxPhysics* p, physx::PxScene* s, const std::string& path, Aftr::Vector scale, Aftr::MESH_SHADING_TYPE mst);
	
	void setActor(physx::PxRigidActor* actor);
	physx::PxRigidActor* getActor();
	virtual void setPosition(Aftr::Vector v);

	void updatePoseFromPhysicsEngine(physx::PxActor* actor);

protected:
	
	Module5Physics(physx::PxPhysics* p, physx::PxScene* s);
	void onCreate(const std::string& path, const Aftr::Vector& scale, Aftr::MESH_SHADING_TYPE mst);
	
	physx::PxPhysics* p;
	physx::PxScene* s;
	physx::PxRigidActor* a;

};
