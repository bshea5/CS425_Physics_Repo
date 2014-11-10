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

	// for projectile 
	bool projectile;			// is this agent going to be launched?
	Ogre::Real mass;
	Ogre::Real speed;
	Ogre::Vector3 initPos;		// initial position
	Ogre::Vector3 vel;			// velocity of agent
	Ogre::Vector3 gravity; 
	void shoot(Ogre::Real deltaTime); // shoots the agent through the air
	
	Ogre::ParticleSystem* ps;	//particle effect for when the agent is a projectile

	Ogre::Real mTimer;						// general timer to see how long animations have been playing
	Ogre::Real mVerticalVelocity;			// for jumping

	bool procedural;						// Is this character performing a procedural animation

public:
	Agent(Ogre::SceneManager* SceneManager, std::string name, std::string filename, float height, float scale);
	~Agent();

	// lecture 12: launches the character
	// adjust velocity and speed with parameters
	void fire(Ogre::Real vx, Ogre::Real vy, Ogre::Real vz, Ogre::Real speed);
	void reload();	// halts all agent movement and resets in start position
	Ogre::Vector3  getVelocity(){ return vel; }		// returns agents velocity
	void setMass(Ogre::Real m) { this->mass = m; }
	void setPosition(float x, float y, float z);
	void setOrientation(Ogre::Real r);				// adjust the direction the fish is facing
	bool intersects(Ogre::Entity* e);				//intersects with an entity e
	void update(Ogre::Real deltaTime);				// update the agent

};