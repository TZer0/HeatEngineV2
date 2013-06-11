#include "heatengine.h"

HeatEngine::HeatEngine(void)
	: mRoot(0),
	mCamera(0),
	mSceneMgr(0),
	mWindow(0),
	mResourcesCfg(Ogre::StringUtil::BLANK),
	mPluginsCfg(Ogre::StringUtil::BLANK),
	mTrayMgr(0),
	mDetailsPanel(0),
	mCursorWasVisible(false),
	mShutDown(false),
	mInputManager(0),
	mMouse(0),
	mKeyboard(0),
	mSim(0)
{
	mSim = new Simulation();
}


HeatEngine::~HeatEngine(void)
{
	if (mTrayMgr) delete mTrayMgr;
	
	//Remove ourself as a Window listener
	Ogre::WindowEventUtilities::removeWindowEventListener(mWindow, this);
	windowClosed(mWindow);
	delete mRoot;
}


bool HeatEngine::configure(void)
{
	// Show the configuration dialog and initialise the system
	// You can skip this and use root.restoreConfig() to load configuration
	// settings if you were sure there are valid ones saved in ogre.cfg
	if(mRoot->showConfigDialog())
	{
		// If returned true, user clicked OK so initialise
		// Here we choose to let the system create a default rendering window by passing 'true'
		mWindow = mRoot->initialise(true, "HeatEngineV2");
		
		return true;
	}
	else
	{
		return false;
	}
}

void HeatEngine::chooseSceneManager(void)
{
	// Get the SceneManager, in this case a generic one
	mSceneMgr = mRoot->createSceneManager(Ogre::ST_GENERIC);
}

void HeatEngine::createCamera(void)
{
	// Create the camera
	mCamera = mSceneMgr->createCamera("PlayerCam");
	
	// Position it at 500 in Z direction
	mCamera->setPosition(Ogre::Vector3(20,20,80));
	// Look back along -Z
	mCamera->lookAt(Ogre::Vector3(0,0,0));
	mCamera->setNearClipDistance(5);
	mCamera->setFarClipDistance(600);
	
}

void HeatEngine::createFrameListener(void)
{
	Ogre::LogManager::getSingletonPtr()->logMessage("*** Initializing OIS ***");
	OIS::ParamList pl;
	size_t windowHnd = 0;
	std::ostringstream windowHndStr;
	
	mWindow->getCustomAttribute("WINDOW", &windowHnd);
	windowHndStr << windowHnd;
	pl.insert(std::make_pair(std::string("WINDOW"), windowHndStr.str()));
	
	mInputManager = OIS::InputManager::createInputSystem( pl );
	
	mKeyboard = static_cast<OIS::Keyboard*>(mInputManager->createInputObject( OIS::OISKeyboard, true ));
	mMouse = static_cast<OIS::Mouse*>(mInputManager->createInputObject( OIS::OISMouse, true ));
	
	mMouse->setEventCallback(this);
	mKeyboard->setEventCallback(this);
	
	//Set initial mouse clipping size
	windowResized(mWindow);
	
	//Register as a Window listener
	Ogre::WindowEventUtilities::addWindowEventListener(mWindow, this);
	
	mTrayMgr = new OgreBites::SdkTrayManager("InterfaceName", mWindow, mMouse, this);
	mTrayMgr->showFrameStats(OgreBites::TL_BOTTOMLEFT);
	
	// create a params panel for displaying sample details
	Ogre::StringVector items;
	items.push_back("cam.pX");
	items.push_back("cam.pY");
	items.push_back("cam.pZ");
	items.push_back("");
	items.push_back("cam.oW");
	items.push_back("cam.oX");
	items.push_back("cam.oY");
	items.push_back("cam.oZ");
	items.push_back("");
	items.push_back("Filtering");
	items.push_back("Poly Mode");
	
	mDetailsPanel = mTrayMgr->createParamsPanel(OgreBites::TL_NONE, "DetailsPanel", 200, items);
	mDetailsPanel->setParamValue(9, "Bilinear");
	mDetailsPanel->setParamValue(10, "Solid");
	mDetailsPanel->hide();
	
	mRoot->addFrameListener(this);
}

void HeatEngine::createScene(void) 
{
	mObjs = mSceneMgr->createManualObject();
	mWalls = mSceneMgr->createManualObject();
	Ogre::SceneNode *root = mSceneMgr->getRootSceneNode();
	root->attachObject(mObjs);
	root->attachObject(mWalls);
}

void HeatEngine::destroyScene(void)
{
}

void HeatEngine::createViewports(void)
{
	// Create one viewport, entire window
	Ogre::Viewport* vp = mWindow->addViewport(mCamera);
	vp->setBackgroundColour(Ogre::ColourValue(0.9,0.9,0.9));
	
	// Alter the camera aspect ratio to match the viewport
	mCamera->setAspectRatio(
		Ogre::Real(vp->getActualWidth()) / Ogre::Real(vp->getActualHeight()));
}

void HeatEngine::setupResources(void)
{
	// Load resource paths from config file
	Ogre::ConfigFile cf;
	cf.load(mResourcesCfg);
	
	// Go through all sections & settings in the file
	Ogre::ConfigFile::SectionIterator seci = cf.getSectionIterator();
	
	Ogre::String secName, typeName, archName;
	while (seci.hasMoreElements())
	{
		secName = seci.peekNextKey();
		Ogre::ConfigFile::SettingsMultiMap *settings = seci.getNext();
		Ogre::ConfigFile::SettingsMultiMap::iterator i;
		for (i = settings->begin(); i != settings->end(); ++i)
		{
			typeName = i->first;
			archName = i->second;
			Ogre::ResourceGroupManager::getSingleton().addResourceLocation(
				archName, typeName, secName);
		}
	}
}

void HeatEngine::createResourceListener(void)
{
	
}

void HeatEngine::loadResources(void)
{
	Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups();
}

void HeatEngine::go(void)
{
	#ifdef _DEBUG
	mResourcesCfg = "resources_d.cfg";
	mPluginsCfg = "plugins_d.cfg";
	#else
	mResourcesCfg = "resources.cfg";
	mPluginsCfg = "plugins.cfg";
	#endif
	
	if (!setup())
		return;
	
	mRoot->startRendering();
	
	// clean up
	destroyScene();
}

bool HeatEngine::setup(void)
{
	mRoot = new Ogre::Root(mPluginsCfg);
	
	setupResources();
	
	bool carryOn = configure();
	if (!carryOn) return false;
	
	chooseSceneManager();
	createCamera();
	createViewports();
	
	// Set default mipmap level (NB some APIs ignore this)
	Ogre::TextureManager::getSingleton().setDefaultNumMipmaps(5);
	
	// Create any resource listeners (for loading screens)
	createResourceListener();
	// Load resources
	loadResources();
	
	// Create the scene
	createScene();
	
	createFrameListener();
	
	return true;
};

bool HeatEngine::frameRenderingQueued(const Ogre::FrameEvent& evt)
{
	if(mWindow->isClosed()) {
		return false;
	}
	
	if(mShutDown) {
		return false;
	}
	
	mSim->tick(evt.timeSinceLastFrame);
	updateManObj();
	//Need to capture/update each device
	mKeyboard->capture();
	mMouse->capture();
	
	mTrayMgr->frameRenderingQueued(evt);
	
	if (!mTrayMgr->isDialogVisible())
	{
		if (mDetailsPanel->isVisible())   // if details panel is visible, then update its contents
		{
			mDetailsPanel->setParamValue(0, Ogre::StringConverter::toString(mCamera->getDerivedPosition().x));
			mDetailsPanel->setParamValue(1, Ogre::StringConverter::toString(mCamera->getDerivedPosition().y));
			mDetailsPanel->setParamValue(2, Ogre::StringConverter::toString(mCamera->getDerivedPosition().z));
			mDetailsPanel->setParamValue(4, Ogre::StringConverter::toString(mCamera->getDerivedOrientation().w));
			mDetailsPanel->setParamValue(5, Ogre::StringConverter::toString(mCamera->getDerivedOrientation().x));
			mDetailsPanel->setParamValue(6, Ogre::StringConverter::toString(mCamera->getDerivedOrientation().y));
			mDetailsPanel->setParamValue(7, Ogre::StringConverter::toString(mCamera->getDerivedOrientation().z));
		}
	}
	
	return true;
}

bool HeatEngine::keyPressed( const OIS::KeyEvent &arg )
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
    
    return true;
}

bool HeatEngine::keyReleased( const OIS::KeyEvent &arg )
{
	return true;
}

bool HeatEngine::mouseMoved( const OIS::MouseEvent &arg )
{
	if (mTrayMgr->injectMouseMove(arg)) return true;
	return true;
}

bool HeatEngine::mousePressed( const OIS::MouseEvent &arg, OIS::MouseButtonID id )
{
	if (mTrayMgr->injectMouseDown(arg, id)) return true;
	return true;
}

bool HeatEngine::mouseReleased( const OIS::MouseEvent &arg, OIS::MouseButtonID id )
{
	if (mTrayMgr->injectMouseUp(arg, id)) return true;
	return true;
}

//Adjust mouse clipping area
void HeatEngine::windowResized(Ogre::RenderWindow* rw)
{
	unsigned int width, height, depth;
	int left, top;
	rw->getMetrics(width, height, depth, left, top);
	
	const OIS::MouseState &ms = mMouse->getMouseState();
	ms.width = width;
	ms.height = height;
}

//Unattach OIS before window shutdown (very important under Linux)
void HeatEngine::windowClosed(Ogre::RenderWindow* rw)
{
	//Only close for window that created OIS (the main window in these demos)
	if( rw == mWindow )
	{
		if( mInputManager )
		{
			mInputManager->destroyInputObject( mMouse );
			mInputManager->destroyInputObject( mKeyboard );
			
			OIS::InputManager::destroyInputSystem(mInputManager);
			mInputManager = 0;
		}
	}
}

void HeatEngine::updateManObj()
{
	RenderData *data = mSim->getData();
	//mObjs->estimateVertexCount(data->xSize*data->ySize*data->zSize*6);
	//mObjs->estimateIndexCount(data->xSize*data->ySize*data->zSize*6);
	mObjs->clear();
	mObjs->begin("defaultwall");
	uint count = 0;
	for (int x = 0; x < data->xSize; x++) {
		for (int y = 0; y < data->ySize; y++) {
			for (int z = 0; z < data->zSize; z++) {
				count += 4;
				manObjAdd(mObjs, x, y, z, data);
				mObjs->quad(count-1, count-2, count-3, count-4);
				
			}
		}
	}
	mObjs->end();
}


void HeatEngine::manObjAdd(Ogre::ManualObject *obj, int xt, int yt, int z, RenderData *data)
{
	obj->position(xt*TILESIZE, yt*TILESIZE, z*TILESIZE);
	useTexCoord(obj, 0);
	obj->position((xt+1)*TILESIZE, yt*TILESIZE, z*TILESIZE);
	useTexCoord(obj, 1);
	obj->position((xt+1)*TILESIZE, (yt+1)*TILESIZE, z*TILESIZE);
	useTexCoord(obj, 2);
	obj->position(xt*TILESIZE, (yt+1)*TILESIZE, z*TILESIZE);
	useTexCoord(obj, 3);
}


void HeatEngine::useTexCoord(Ogre::ManualObject *obj, int c)
{
	switch(c) {
		case 0:
			obj->textureCoord(0,1,0);
			break;
		case 1:
			obj->textureCoord(1,1,0);
			break;
		case 2:
			obj->textureCoord(1,0,0);
			break;
		case 3:
			obj->textureCoord(0,0,0);
			break;
	}
	
}


#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
#define WIN32_LEAN_AND_MEAN
#include "windows.h"
#endif

#ifdef __cplusplus
extern "C" {
	#endif
	
	#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
	INT WINAPI WinMain( HINSTANCE hInst, HINSTANCE, LPSTR strCmdLine, INT )
	#else
	int main(int argc, char *argv[])
	#endif
	{
		// Create application object
		HeatEngine app;
		
		try {
			app.go();
		} catch( Ogre::Exception& e ) {
			#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
			MessageBox( NULL, e.getFullDescription().c_str(), "An exception has occured!", MB_OK | MB_ICONERROR | MB_TASKMODAL);
			#else
			std::cerr << "An exception has occured: " <<
			e.getFullDescription().c_str() << std::endl;
			#endif
		}
		
		return 0;
	}
	
	#ifdef __cplusplus
}
#endif
