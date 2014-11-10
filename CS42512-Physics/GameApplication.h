#ifndef __GameApplication_h_
#define __GameApplication_h_

#include "BaseApplication.h"
#include "Agent.h"

#define MAXSHOTS 21

//foward declarations
class Grid;
class GridNode;
/////////////////////

class GameApplication : public BaseApplication
{
private:
	Agent* agent;				// store a pointer to the character
	Ogre::Entity* target;		//pointer to target barrel
	std::list<Agent*> agentList; // Lecture 5: now a list of agents
	Ogre::Vector3 launchVector;	// determines trajectory
	Ogre::Real speed;			//speed of agent shot
	int shots_fired;			//number of fish fired
	int score;					//score from hitting barrels with fish
	int number_targets;			//number of barrels to hit
	Grid* grid;					//just using the same name to save refactoring time
	std::vector<Ogre::Entity*> targetList; //list of target barrels

public:
    GameApplication(void);
    virtual ~GameApplication(void);

	void loadEnv();			// Load the buildings or ground plane, etc.
	void setupEnv();		// Set up the lights, shadows, etc
	void loadObjects();		// Load other props or objects (e.g. furniture)
	void loadCharacters();	// Load actors, agents, characters
	void resetGame();		// reset agent, level, and targets

	void addTime(Ogre::Real deltaTime);		// update the game state

	//////////////////////////////////////////////////////////////////////////
	// Lecture 4: keyboard interaction
	// moved from base application
	// OIS::KeyListener
    bool keyPressed( const OIS::KeyEvent &arg );
    bool keyReleased( const OIS::KeyEvent &arg );
    // OIS::MouseListener
    bool mouseMoved( const OIS::MouseEvent &arg );
    bool mousePressed( const OIS::MouseEvent &arg, OIS::MouseButtonID id );
    bool mouseReleased( const OIS::MouseEvent &arg, OIS::MouseButtonID id );
	////////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////////////
	// Lecture 12
	bool bLMouseDown, bRMouseDown;		//true if mouse buttons are held down
	Ogre::SceneNode *mCurrentObject;	//pointer to our currently selected object
	/////////////////////////////////////////////////////////////////////////////////

protected:
    virtual void createScene(void);

	virtual void createGUI(void); // generates gui 
    virtual void buttonHit(OgreBites::Button* b); // Lecture 16
	virtual void okDialogClosed(const Ogre::DisplayString& message);
	virtual void sliderMoved(OgreBites::Slider* s); // Lecture 16
	OgreBites::ParamsPanel* mParamsPanel;	//display vectors and speed
	OgreBites::ParamsPanel* mScorePanel;	//display score and shots fired
	 //mEndScore;		//display game over and final score
};

#endif // #ifndef __TutorialApplication_h_
