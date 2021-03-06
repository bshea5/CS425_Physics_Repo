#include "GameApplication.h"
#include "Grid.h" // Lecture 5
#include <fstream>
#include <sstream>
#include <map> 

//-------------------------------------------------------------------------------------
GameApplication::GameApplication(void): // Lecture 12
mCurrentObject(0),
bLMouseDown(false),
bRMouseDown(false)
{
	agent = NULL; // Init member data
	target = NULL;
	launchVector = Ogre::Vector3(0, 20, -30);
	speed = 1;
	shots_fired = 0;
	score = 0;
	number_targets = 0;
}

//-------------------------------------------------------------------------------------
GameApplication::~GameApplication(void)
{
	if (agent != NULL)  // clean up memory
		delete agent; 
}

//-------------------------------------------------------------------------------------
void GameApplication::createScene(void)
{
    loadEnv();
	setupEnv();
	loadObjects();
	loadCharacters();
	//////////////////////////////////////////////////////////////////////////////////
	// Lecture 12
	//but we also want to set up our raySceneQuery after everything has been initialized
	mRayScnQuery = mSceneMgr->createRayQuery(Ogre::Ray());
}

void GameApplication::createGUI(void)
{
	if (mTrayMgr == NULL) return;
	using namespace OgreBites;
	Button* b = mTrayMgr->createButton(TL_TOPRIGHT, "MyButton", "Last position", 120.0);
	b->show();

	//sliders to control velocities and speed
	Slider* xVelocitySlider = mTrayMgr->createThickSlider(TL_BOTTOMRIGHT, "xVelocitySlider", "X Velocity", 250, 80, 0, 0, 0);
	xVelocitySlider->setRange(-10,10,21);
	xVelocitySlider->setValue(0);
	mTrayMgr->sliderMoved(xVelocitySlider);

	Slider* zVelocitySlider = mTrayMgr->createThickSlider(TL_BOTTOMRIGHT, "zVelocitySlider", "Z Velocity", 250, 80, 0, 0, 0);
	zVelocitySlider->setRange(-20,0,21);
	zVelocitySlider->setValue(-10);
	mTrayMgr->sliderMoved(zVelocitySlider);

	Slider* yVelocitySlider = mTrayMgr->createThickSlider(TL_BOTTOMRIGHT, "yVelocitySlider", "Y Velocity", 250, 80, 0, 0, 0);
	yVelocitySlider->setRange(0,15,16);
	yVelocitySlider->setValue(10);
	mTrayMgr->sliderMoved(yVelocitySlider);

	Slider* speedSlider= mTrayMgr->createThickSlider(TL_BOTTOMRIGHT, "speedSlider", "Speeeed", 250, 80, 0, 0, 0);
	speedSlider->setRange(0,10,11);
	speedSlider->setValue(5);
	mTrayMgr->sliderMoved(speedSlider);

	// Lecture 16: Setup parameter panel: Updated in addTime
	Ogre::StringVector items;
	items.push_back("V:");
	mParamsPanel = mTrayMgr->createParamsPanel(OgreBites::TL_BOTTOMLEFT,"Trajectory Velocity",250,items);
	mParamsPanel->setParamValue(0, Ogre::StringConverter::toString(Ogre::Vector3::ZERO));

	// score panel with number of shots fired
	items.clear();
	items.push_back("Score");
	items.push_back("Shots fired");
	items.push_back("Targets remaining");
	mScorePanel = mTrayMgr->createParamsPanel(OgreBites::TL_TOP, "Stats", 250, items);
	
	//mTrayMgr->create
	mTrayMgr->showAll();

}
//////////////////////////////////////////////////////////////////
// Lecture 5: Returns a unique name for loaded objects and agents
std::string getNewName() // return a unique name 
{
	static int count = 0;	// keep counting the number of objects

	std::string s;
	std::stringstream out;	// a stream for outputing to a string
	out << count++;			// make the current count into a string
	s = out.str();

	return "object_" + s;	// append the current count onto the string
}

// Lecture 5: Load level from file!
void // Load the buildings or ground plane, etc
GameApplication::loadEnv()
{
	using namespace Ogre;	// use both namespaces
	using namespace std;

	class readEntity // need a structure for holding entities
	{
	public:
		string filename;
		float y;
		float scale;
		float orient;
		bool agent;
	};

	ifstream inputfile;		// Holds a pointer into the file

	string path = __FILE__; //gets the current cpp file's path with the cpp file
	path = path.substr(0,1+path.find_last_of('\\')); //removes filename to leave path
	path+= "level001.txt"; //if txt file is in the same directory as cpp file
	inputfile.open(path);

	//inputfile.open("D:/CS425-2012/Lecture 8/GameEngine-loadLevel/level001.txt"); // bad explicit path!!!
	if (!inputfile.is_open()) // oops. there was a problem opening the file
	{
		cout << "ERROR, FILE COULD NOT BE OPENED" << std::endl;	// Hmm. No output?
		return;
	}

	// the file is open
	int x,z;
	inputfile >> x >> z;	// read in the dimensions of the grid
	string matName;
	inputfile >> matName;	// read in the material name

	// create floor mesh using the dimension read
	MeshManager::getSingleton().createPlane("floor", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, 
		Plane(Vector3::UNIT_Y, 0), x*NODESIZE, z*NODESIZE, x, z, true, 1, x, z, Vector3::UNIT_Z);
	
	//create a floor entity, give it material, and place it at the origin
	Entity* floor = mSceneMgr->createEntity("Floor", "floor");
	floor->setMaterialName(matName);
	floor->setCastShadows(false);
	mSceneMgr->getRootSceneNode()->attachObject(floor);

	this->grid = new Grid(mSceneMgr, z, x); // Set up the grid

	string buf;
	inputfile >> buf;	// Start looking for the Objects section
	while  (buf != "Objects")
		inputfile >> buf;
	if (buf != "Objects")	// Oops, the file must not be formated correctly
	{
		cout << "ERROR: Level file error" << endl;
		return;
	}

	// read in the objects
	readEntity *rent = new readEntity();	// hold info for one object
	std::map<string,readEntity*> objs;		// hold all object and agent types;
	while (!inputfile.eof() && buf != "Characters") // read through until you find the Characters section
	{ 
		inputfile >> buf;			// read in the char
		if (buf != "Characters")
		{
			inputfile >> rent->filename >> rent->y >> rent->orient >> rent->scale;  // read the rest of the line
			rent->agent = false;		// these are objects
			objs[buf] = rent;			// store this object in the map
			rent = new readEntity();	// create a new instance to store the next object
		}
	}

	while (buf != "Characters")	// get through any junk
		inputfile >> buf;
	
	// Read in the characters
	while (!inputfile.eof() && buf != "World") // Read through until the world section
	{
		inputfile >> buf;		// read in the char
		if (buf != "World")
		{
			inputfile >> rent->filename >> rent->y >> rent->scale; // read the rest of the line
			rent->agent = true;			// this is an agent
			objs[buf] = rent;			// store the agent in the map
			rent = new readEntity();	// create a new instance to store the next object
		}
	}
	delete rent; // we didn't need the last one

	// read through the placement map
	char c;
	for (int i = 0; i < z; i++)			// down (row)
		for (int j = 0; j < x; j++)		// across (column)
		{
			inputfile >> c;			// read one char at a time
			buf = c + '\0';			// convert char to string
			rent = objs[buf];		// find cooresponding object or agent
			if (rent != NULL)		// it might not be an agent or object
				if (rent->agent)	// if it is an agent...
				{
					// Use subclasses instead!
					agent = new Agent(this->mSceneMgr, getNewName(), rent->filename, rent->y, rent->scale);
					agentList.push_back(agent);
					agent->setPosition(grid->getPosition(i,j).x, 0, grid->getPosition(i,j).z);
				}
				else	// Load objects
				{
					grid->loadObject(getNewName(), rent->filename, i, rent->y, j, rent->scale);
					if (c == 'd') //object is the target drum
					{
						target = grid->getNode(i,j)->entity;
						target->setVisible(false);		//hide barrels!
						targetList.push_back(target);	
					}
				}
			else // not an object or agent
			{
				if (c == 'w') // create a wall
				{
					Entity* ent = mSceneMgr->createEntity(getNewName(), Ogre::SceneManager::PT_CUBE);
					ent->setMaterialName("Examples/RustySteel");
					Ogre::SceneNode* mNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
					mNode->attachObject(ent);
					mNode->scale(0.1f,0.2f,0.1f); // cube is 100 x 100
					grid->getNode(i,j)->setOccupied();  // indicate that agents can't pass through
					mNode->setPosition(grid->getPosition(i,j).x, 10.0f, grid->getPosition(i,j).z);
				}
				else if (c == 'e')
				{
					ParticleSystem::setDefaultNonVisibleUpdateTimeout(5);  // set nonvisible timeout
					ParticleSystem* ps = mSceneMgr->createParticleSystem(getNewName(), "Examples/PurpleFountain");
					Ogre::SceneNode* mNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
					mNode->attachObject(ps);
					mNode->setPosition(grid->getPosition(i,j).x, 0.0f, grid->getPosition(i,j).z);
				}
			}
		}
	
	target->setVisible(true); //toggle last target scanned in visible
	number_targets = targetList.size();

	// delete all of the readEntities in the objs map
	rent = objs["s"]; // just so we can see what is going on in memory (delete this later)
	
	std::map<string,readEntity*>::iterator it;
	for (it = objs.begin(); it != objs.end(); it++) // iterate through the map
	{
		delete (*it).second; // delete each readEntity
	}
	objs.clear(); // calls their destructors if there are any. (not good enough)
	
	inputfile.close();
	grid->printToFile(); // see what the initial grid looks like.
}

void // Set up lights, shadows, etc
GameApplication::setupEnv()
{
	using namespace Ogre;

	// set shadow properties
	mSceneMgr->setShadowTechnique(SHADOWTYPE_TEXTURE_MODULATIVE);
	mSceneMgr->setShadowColour(ColourValue(0.5, 0.5, 0.5));
	mSceneMgr->setShadowTextureSize(1024);
	mSceneMgr->setShadowTextureCount(1);

	// disable default camera control so the character can do its own 
	mCameraMan->setStyle(OgreBites::CS_FREELOOK); // CS_FREELOOK, CS_ORBIT, CS_MANUAL
	mCamera->setPosition(-9, 6.9, 100.5);

	// use small amount of ambient lighting
	mSceneMgr->setAmbientLight(ColourValue(0.3f, 0.3f, 0.3f));

	// add a bright light above the scene
	Light* light = mSceneMgr->createLight();
	light->setType(Light::LT_POINT);
	light->setPosition(-10, 40, 20);
	light->setSpecularColour(ColourValue::White);

	//mSceneMgr->setSkyDome(true, "Examples/CloudySky", 5, 8); // Lecture 4
}

void // Load other props or objects
GameApplication::loadObjects()
{

}

void // Load actors, agents, characters
GameApplication::loadCharacters()
{
	// Lecture 5: now loading from file
	// agent = new Agent(this->mSceneMgr, "Sinbad", "Sinbad.mesh");
}

void
GameApplication::resetGame()
{
	// reset fish and game states
	score = 0;
	shots_fired = 0;
	agent->reload();

	// reset targets
	target->setVisible(false);
	number_targets = targetList.size();
	target = targetList[number_targets-1];
	target->setVisible(true);
}

void
GameApplication::addTime(Ogre::Real deltaTime)
{
	if (shots_fired >= MAXSHOTS)
	{ // end game and give the option to reset game
		Ogre::String uScored = "Hit 'OK' to restart.\nScore: ";
		uScored +=  Ogre::StringConverter::toString(score);
		mTrayMgr->showOkDialog("Game Over!", uScored);
	}

	//update agent, aka the fish
	agent->update(deltaTime);

	//or check for collision here since there is a pointer to both the agent 
	//and the target in the GameApp
	if (target != NULL && agent->intersects(target))
	{
		std::cout << "hit! " << std::endl;
		agent->reload();
		score = score + 100/shots_fired;	//rewards are higher for those that get it done
											//in less shots
		//lets rotate the barrel too on collision!
		//setting invisible for now
		target->setVisible(false);

		//load new barrel as target
		number_targets--;
		if (number_targets < 1) //no more targets, toggle menu and reset game
		{
			Ogre::String uScored = "Hit 'OK' to restart.\nScore: ";
			uScored +=  Ogre::StringConverter::toString(score);
			mTrayMgr->showOkDialog("You Win!", uScored);
		}
		else //more targets to hit!
		{
			target = targetList[number_targets-1];
			target->setVisible(true);
		}
	}

	// Velocity Panel 
	mParamsPanel->setParamValue(0, Ogre::StringConverter::toString(agent->getVelocity()));

	// Score Panel
	mScorePanel->setParamValue(0, Ogre::StringConverter::toString(score));
	mScorePanel->setParamValue(1, Ogre::StringConverter::toString(shots_fired));
	mScorePanel->setParamValue(2, Ogre::StringConverter::toString(number_targets));
}

bool 
GameApplication::keyPressed( const OIS::KeyEvent &arg ) // Moved from BaseApplication
{
    if (mTrayMgr->isDialogVisible()) return true;   // don't process any more keys if dialog is up

    if (arg.key == OIS::KC_F)   // toggle visibility of advanced frame stats
    {
        mTrayMgr->toggleAdvancedFrameStats();
    }
    else if (arg.key == OIS::KC_G)   // toggle visibility of even rarer debugging details
    {
        if (mDetailsPanel->getTrayLocation() == OgreBites::TL_NONE)
        {
            mTrayMgr->moveWidgetToTray(mDetailsPanel, OgreBites::TL_TOPRIGHT, 0);
            mDetailsPanel->show();
        }
        else
        {
            mTrayMgr->removeWidgetFromTray(mDetailsPanel);
            mDetailsPanel->hide();
        }
    }
    else if (arg.key == OIS::KC_T)   // cycle polygon rendering mode
    {
        Ogre::String newVal;
        Ogre::TextureFilterOptions tfo;
        unsigned int aniso;

        switch (mDetailsPanel->getParamValue(9).asUTF8()[0])
        {
        case 'B':
            newVal = "Trilinear";
            tfo = Ogre::TFO_TRILINEAR;
            aniso = 1;
            break;
        case 'T':
            newVal = "Anisotropic";
            tfo = Ogre::TFO_ANISOTROPIC;
            aniso = 8;
            break;
        case 'A':
            newVal = "None";
            tfo = Ogre::TFO_NONE;
            aniso = 1;
            break;
        default:
            newVal = "Bilinear";
            tfo = Ogre::TFO_BILINEAR;
            aniso = 1;
        }

        Ogre::MaterialManager::getSingleton().setDefaultTextureFiltering(tfo);
        Ogre::MaterialManager::getSingleton().setDefaultAnisotropy(aniso);
        mDetailsPanel->setParamValue(9, newVal);
    }
    else if (arg.key == OIS::KC_R)   // cycle polygon rendering mode
    {
        Ogre::String newVal;
        Ogre::PolygonMode pm;

        switch (mCamera->getPolygonMode())
        {
        case Ogre::PM_SOLID:
            newVal = "Wireframe";
            pm = Ogre::PM_WIREFRAME;
            break;
        case Ogre::PM_WIREFRAME:
            newVal = "Points";
            pm = Ogre::PM_POINTS;
            break;
        default:
            newVal = "Solid";
            pm = Ogre::PM_SOLID;
        }

        mCamera->setPolygonMode(pm);
        mDetailsPanel->setParamValue(10, newVal);
    }
    else if(arg.key == OIS::KC_F5)   // refresh all textures
    {
        Ogre::TextureManager::getSingleton().reloadAll();
    }
    else if (arg.key == OIS::KC_SYSRQ)   // take a screenshot
    {
        mWindow->writeContentsToTimestampedFile("screenshot", ".jpg");
    }
    else if (arg.key == OIS::KC_ESCAPE)
    {
        mShutDown = true;
    }
	else if (arg.key == OIS::KC_SPACE)
	{
		if (shots_fired < MAXSHOTS) //limit the number of shots per game to 20
		{
			this->agent->fire(launchVector[0], launchVector[1], launchVector[2], speed); 
			shots_fired++;
		}
	}
	//adjust input for initial velocities
	//adjust z direction velocity
	else if (arg.key == OIS::KC_EQUALS)
	{
		if (launchVector[2] < 0)
		{
			this->launchVector[2]++;
		}
	}
	else if (arg.key == OIS::KC_MINUS)
	{
		this->launchVector[2]--;
	}
	//adjust y direction velocity
	else if (arg.key == OIS::KC_RBRACKET)
	{
		this->launchVector[1]++;
	}
	else if (arg.key == OIS::KC_LBRACKET)
	{
		if (launchVector[1] > 0)
		{
			this->launchVector[1]--;
		}
	}
	//adjust speed
	else if (arg.key == OIS::KC_9)
	{
		if (speed > 0) { this->speed--; }
	}
	else if (arg.key == OIS::KC_0)
	{
		if (speed < 10) { this->speed++; }
	}
   
    mCameraMan->injectKeyDown(arg);
    return true;
}

bool GameApplication::keyReleased( const OIS::KeyEvent &arg )
{
    mCameraMan->injectKeyUp(arg);
    return true;
}

bool GameApplication::mouseMoved( const OIS::MouseEvent &arg )
{
	// Lecture 12
	//if the left mouse button is held down
	if(bLMouseDown)
	{
		//find the current mouse position
		Ogre::Vector3 mousePos; 
		mousePos.x = arg.state.X.abs;
		mousePos.y = arg.state.Y.abs;
		mousePos.z = arg.state.Z.abs;
 
		//create a raycast straight out from the camera at the mouse's location
		Ogre::Ray mouseRay = mCamera->getCameraToViewportRay(mousePos.x/float(arg.state.width), mousePos.y/float(arg.state.height));
		mRayScnQuery->setRay(mouseRay);
		mRayScnQuery->setSortByDistance(false);	//world geometry is at the end of the list if we sort it, so lets not do that
 
		Ogre::RaySceneQueryResult& result = mRayScnQuery->execute();
		Ogre::RaySceneQueryResult::iterator iter = result.begin();
 
		//check to see if the mouse is pointing at the world and put our current object at that location
		for(iter; iter != result.end(); iter++)
		{
			//if(iter->worldFragment) // Only works for terrain?
			//{
			//	mCurrentObject->setPosition(iter->worldFragment->singleIntersection);
			//	break;
			//}	
			if (iter->movable)
			{
				mCurrentObject->setPosition(iter->movable->getParentSceneNode()->getPosition());
			}
		}
	}
	else if(bRMouseDown)	//if the right mouse button is held down, be rotate the camera with the mouse
	{
		mCamera->yaw(Ogre::Degree(-arg.state.X.rel * 0.1));
		mCamera->pitch(Ogre::Degree(-arg.state.Y.rel * 0.1));
	}
	/////////////////////////////////////////////////////////////////////////////////////////


    if (mTrayMgr->injectMouseMove(arg)) return true;
    //mCameraMan->injectMouseMove(arg);
    return true;
}

bool GameApplication::mousePressed( const OIS::MouseEvent &arg, OIS::MouseButtonID id )
{
	//////////////////////////////////////////////////////////////////////////////////////
	// Lecture 12
	if(id == OIS::MB_Left)
	{
		//show that the current object has been deselected by removing the bounding box visual
		if(mCurrentObject)
		{
			mCurrentObject->showBoundingBox(false);
		}
 
		//find the current mouse position
		Ogre::Vector3 mousePos;
		mousePos.x = arg.state.X.abs;
		mousePos.y = arg.state.Y.abs;
		mousePos.z = arg.state.Z.abs;
		
		//then send a raycast straight out from the camera at the mouse's position
		Ogre::Ray mouseRay = mCamera->getCameraToViewportRay(mousePos.x/float(arg.state.width), mousePos.y/float(arg.state.height));
		mRayScnQuery->setRay(mouseRay);
		mRayScnQuery->setSortByDistance(true);
		
		/* This next chunk finds the results of the raycast */
		Ogre::RaySceneQueryResult& result = mRayScnQuery->execute();
		Ogre::RaySceneQueryResult::iterator iter = result.begin();
 
		for(iter; iter != result.end(); iter++)
		{
			if(iter->movable && iter->movable->getName().substr(0, 5) != "tile[")
			{
				mCurrentObject = iter->movable->getParentSceneNode();
				break;
			}
		}
 
		//now we show the bounding box so the user can see that this object is selected
		if(mCurrentObject)
		{
			mCurrentObject->showBoundingBox(true);
		}
 
		bLMouseDown = true;
	}
	else if (id == OIS::MB_Right)
		bRMouseDown = true;

	//////////////////////////////////////////////////////////////////////////////////////
   
    if (mTrayMgr->injectMouseDown(arg, id)) return true;
    mCameraMan->injectMouseDown(arg, id);
    return true;
}

bool GameApplication::mouseReleased( const OIS::MouseEvent &arg, OIS::MouseButtonID id )
{
	// Lecture 12
	if(id  == OIS::MB_Left)
	{
		bLMouseDown = false;
	}
	if(id  == OIS::MB_Right)
	{
		bRMouseDown = false;
	}
	//else
		//mCameraMan->injectMouseUp(arg, id);

	//////////////////////////////////////////////////////////////////////////////
   
    if (mTrayMgr->injectMouseUp(arg, id)) return true;
    //mCameraMan->injectMouseUp(arg, id);
    return true;
}

////////////////////////////////////////////////////////////////////
// Lecture 16
// Callback method for buttons
// overrides a previous buttonHit function?
// resets to default or last position before fired
void GameApplication::buttonHit(OgreBites::Button* b)
{
	if (b->getName() == "MyButton")
	{
		std::cout << "reload!" << std::endl;
		agent->reload();
	}
}

////////////////////////////////////////////////////////////////////
// Callback for the Game Over menu's OK button to reset game
void 
GameApplication::okDialogClosed(const Ogre::DisplayString& message)
{
	this->resetGame();
	mTrayMgr->closeDialog();
}

////////////////////////////////////////////////////////////////////
// Lecture 16
// Callback method for sliders
void GameApplication::sliderMoved(OgreBites::Slider* s)
{
	//check which slider was used and use its value to change an element
	if (s->getName()=="xVelocitySlider")
	{
		launchVector[0] = s->getValue();	//changes direction to be more left or right'ish
		agent->setOrientation(260 - (s->getValue()*2));	//adjust direction fish is facing
														//multiply by 2 to extragerate the turn more
	}
	else if (s->getName()=="yVelocitySlider")
	{
		launchVector[1] = s->getValue();	//shoots fish higher up
	}
	else if (s->getName()=="zVelocitySlider")
	{
		launchVector[2] = s->getValue();	//shoots fish farther
	}
	else if (s->getName()=="speedSlider")
	{
		speed = s->getValue();				//speed of the fish
	}
}