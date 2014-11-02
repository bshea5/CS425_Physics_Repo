#include "BaseApplication.h"
#include <deque>

class Agent
{
private:
	Ogre::SceneManager* mSceneMgr;		// pointer to scene graph
	Ogre::SceneNode* mBodyNode;			
	Ogre::Entity* mBodyEntity;
	float height;						// height the character should be moved up
	float scale;						// scale of character from original model

	// lecture 12
	bool projectile;			// is this agent going to be launched?
	Ogre::Vector3 initPos;		// initial position
	Ogre::Vector3 vel;			// velocity of agent
	Ogre::Vector3 gravity; 
	void shoot(Ogre::Real deltaTime); // shoots the agent through the air
	Ogre::ParticleSystem* ps;

	Ogre::Real mTimer;						// general timer to see how long animations have been playing
	Ogre::Real mVerticalVelocity;			// for jumping

	// for locomotion
	Ogre::Real mDistance;					// The distance the agent has left to travel
	Ogre::Vector3 mDirection;				// The direction the object is moving
	Ogre::Vector3 mDestination;				// The destination the object is moving towards
	std::deque<Ogre::Vector3> mWalkList;	// The list of points we are walking to
	Ogre::Real mWalkSpeed;					// The speed at which the object is moving
	bool nextLocation();					// Is there another destination?
	void updateLocomote(Ogre::Real deltaTime);			// update the character's walking

	bool procedural;						// Is this character performing a procedural animation

public:
	Agent(Ogre::SceneManager* SceneManager, std::string name, std::string filename, float height, float scale);
	~Agent();

	void fire(Ogre::Real vx, Ogre::Real vy, Ogre::Real vz);		// lecture 12: launches the character
																// adjust velocity with parameters

	void setPosition(float x, float y, float z);
	void lastPosition();

	void update(Ogre::Real deltaTime);		// update the agent

	
};