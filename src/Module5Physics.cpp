#include <Module5Physics.h>
#include "PxPhysicsAPI.h"
#include "PxRigidStatic.h"
#include "PxMaterial.h"
#include "PxActor.h"
#include "PxShape.h"
#include "Model.h"

using namespace physx;

Module5Physics::Module5Physics(PxPhysics* p, PxScene* s) : IFace(this), WO()
{
	this->p = p;
	this->s = s;
	this->a = nullptr;
}

Module5Physics* Module5Physics::New(PxPhysics* p, PxScene* s, const std::string& path, Aftr::Vector scale, Aftr::MESH_SHADING_TYPE mst) {
	Module5Physics* wo = new Module5Physics(p, s);
	wo->onCreate(path, scale, mst);
	return wo;
}

void Module5Physics::onCreate(const std::string& path, const Aftr::Vector& scale, Aftr::MESH_SHADING_TYPE mst)
{
	WO::onCreate(path, scale, mst);
	PxMaterial* gMaterial = p->createMaterial(0.5f, 0.5f, 0.6f);
	PxShape* shape = p->createShape(PxBoxGeometry(1, 1, 1), *gMaterial, true);
	PxTransform t({ 0,0,0 });

	a = p->createRigidDynamic(t);
	a->attachShape(*shape);

	a->userData = this;
	this->s->addActor(*a);
}

void Module5Physics::setActor(PxRigidActor* actor) {
	this->a = actor;
}

PxRigidActor* Module5Physics::getActor() {
	return this->a;
}


void Module5Physics::setPosition(Aftr::Vector v) {
	WO* wo = this->getWO();
	wo->setPosition(v);
}

void Module5Physics::updatePoseFromPhysicsEngine(PxActor* actor) {
	PxRigidActor* rigid = (PxRigidActor*)actor;
	PxTransform trans = rigid->getGlobalPose();
	setPosition({ trans.p.x, trans.p.y, trans.p.z });
}