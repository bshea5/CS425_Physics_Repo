#include "Agent.h"
#include "MovableText.h"

Agent::Agent(Ogre::SceneManager* SceneManager, std::string name, std::string filename, float height, float scale)
{
	using namespace Ogre;

	mSceneMgr = SceneManager; // keep a pointer to where this agent will be

	if (mSceneMgr == NULL)
	{
		std::cout << "ERROR: No valid scene manager in Agent constructor" << std::endl;
		return;
	}

	this->height = height;
	this->scale = scale;

	mBodyNode = mSceneMgr->getRootSceneNode()->createChildSceneNode(); // create a new scene node
	mBodyEntity = mSceneMgr->createEntity(name, filename); // load the model
	mBodyNode->attachObject(mBodyEntity);	// attach the model to the scene node

	mBodyNode->translate(0,height,0); // make the Ogre stand on the plane (almost)
	mBodyNode->scale(scale,scale,scale); // Scale the figure

	// Lecture 16: Movable text
	//Ogre::MovableText* msg = new Ogre::MovableText("TXT_001", "this is the caption");
	//msg->setTextAlignment(Ogre::MovableText::H_CENTER, Ogre::MovableText::V_ABOVE); // Center horizontally and display above the node
	//mBodyNode->attachObject(msg);

	projectile = false; // lecture 12
	mass = 1.0;
	ParticleSystem::setDefaultNonVisibleUpdateTimeout(5);  // set nonvisible timeout
	ps = mSceneMgr->createParticleSystem("Fountain1", "Examples/PurpleFountain");
	Ogre::SceneNode* mnode = mBodyNode->createChildSceneNode();
	mnode->roll(Degree(180));
	mnode->attachObject(ps);
	ps->setVisible(false);

	//initPos = Ogre::Vector3::ZERO;	//default position if none given
	initPos = this->mBodyNode->getPosition();	//save starting position
	vel = Ogre::Vector3::ZERO;
}

Agent::~Agent(){
	// mSceneMgr->destroySceneNode(mBodyNode); // Note that OGRE does not recommend doing this. It prefers to use clear scene
	// mSceneMgr->destroyEntity(mBodyEntity);
}

void 
Agent::setPosition(float x, float y, float z)
{
	this->mBodyNode->setPosition(x, y + height, z);
	initPos = mBodyNode->getPosition();	//save this position for shooting
}

//////////////////////////////////////////////////////////////////
// adjust the fish to face where it is about to be shot
void
Agent::setOrientation(Ogre::Real r)
{
	Ogre::Quaternion q(Ogre::Degree(r), Ogre::Vector3::UNIT_Y);	//set up Quaternion
	mBodyNode->setOrientation(q);
}

//////////////////////////////////////////////////////////////////
//intializes the initial velocities and toggles the agent to shoot
void
Agent::fire(Ogre::Real vx, Ogre::Real vy, Ogre::Real vz, Ogre::Real speed)
{
	projectile = true; // turns on the movement, which will call shoot
	// set up the initial state
	vel.x = vx;
	vel.y = vy;
	vel.z = vz;
	gravity.x = 0;
	gravity.y = Ogre::Real(-9.81);
	gravity.z = 0;
	this->speed = speed;
	ps->setVisible(true);
	this->mBodyNode->yaw(Ogre::Degree(180));
	this->mBodyNode->pitch(Ogre::Degree(45));
	this->mBodyNode->showBoundingBox(true); 
}

/////////////////////////////////////////////////////////////////
//updates the current velocity and position of the agent
//sets a new position according to math
//agent stops when he touches the ground
//need to adjust for barrel collision
void
Agent::shoot(Ogre::Real deltaTime) // lecture 12 call for every frame of the animation
{
	using namespace Ogre;

	Vector3 pos = this->mBodyNode->getPosition();
	vel = vel + (mass*gravity * deltaTime);
	pos = pos + (vel * speed * deltaTime); // velocity
	pos = pos + 0.5 * mass*gravity * deltaTime * deltaTime; // acceleration

	this->mBodyNode->setPosition(pos);
	this->mBodyNode->pitch(Ogre::Degree(20));

	Ogre::AxisAlignedBox objBox = this->mBodyEntity->getWorldBoundingBox();
	objBox.intersects(objBox);

	if (this->mBodyNode->getPosition().y <= -0.5) // if it get close to the ground, stop
	{
		reload();	// finished reset
	}
}

///////////////////////////////////////////////////////////////
// reset the agent to starting position
// ready to fire again!
void
Agent::reload()
{
	projectile = false;
	ps->setVisible(false);
	this->mBodyNode->pitch(Ogre::Degree(-45));
	this->mBodyNode->yaw(Ogre::Degree(180));
	this->mBodyNode->setPosition(initPos);
}

///////////////////////////////////////////////////////////////
// collides with any other bounding boxes?
bool
Agent::intersects(Ogre::Entity* e)
{
	if (e == NULL) { return false; }

	Ogre::AxisAlignedBox mBox = mBodyEntity->getWorldBoundingBox();
	Ogre::AxisAlignedBox eBox = e->getWorldBoundingBox();

	return mBox.intersects(eBox);
}

////////////////////////////////////////////////////////////////
// update is called at every frame from GameApplication::addTime
void
Agent::update(Ogre::Real deltaTime) 
{
	if (projectile) // Lecture 12
		shoot(deltaTime);
}
