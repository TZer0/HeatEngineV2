#ifndef __HeatEngine_h_
#define __HeatEngine_h_

#include<OgreCamera.h>
#include<OgreEntity.h>
#include<OgreLogManager.h>
#include<OgreRoot.h>
#include<OgreViewport.h>
#include<OgreSceneManager.h>
#include<OgreRenderWindow.h>
#include<OgreConfigFile.h>

#include<OISEvents.h>
#include<OISInputManager.h>
#include<OISKeyboard.h>
#include<OISMouse.h>

#include<OgreVector3.h>
#include<OgreFileSystem.h>

#include<SdkTrays.h>
#include<SdkCameraMan.h>

#include<array>
#include<cmath>


#include "simulation.h"
#include "defines.h"

enum AddMode { XWALL, YWALL, ZWALL, BOX };
std::array<Ogre::Vector3, 24> boxPositions = {{
	Ogre::Vector3(0,0,0)*TILESIZE, Ogre::Vector3(1,0,0)*TILESIZE, Ogre::Vector3(1,1,0)*TILESIZE, Ogre::Vector3(0,1,0)*TILESIZE, 
	Ogre::Vector3(0,0,0)*TILESIZE, Ogre::Vector3(0,1,0)*TILESIZE, Ogre::Vector3(0,1,1)*TILESIZE, Ogre::Vector3(0,0,1)*TILESIZE,
	Ogre::Vector3(0,0,0)*TILESIZE, Ogre::Vector3(1,0,0)*TILESIZE, Ogre::Vector3(1,0,1)*TILESIZE, Ogre::Vector3(0,0,1)*TILESIZE,
	Ogre::Vector3(0,0,1)*TILESIZE, Ogre::Vector3(1,0,1)*TILESIZE, Ogre::Vector3(1,1,1)*TILESIZE, Ogre::Vector3(0,1,1)*TILESIZE,
	Ogre::Vector3(1,0,0)*TILESIZE, Ogre::Vector3(1,1,0)*TILESIZE, Ogre::Vector3(1,1,1)*TILESIZE, Ogre::Vector3(1,0,1)*TILESIZE,
	Ogre::Vector3(0,1,0)*TILESIZE, Ogre::Vector3(1,1,0)*TILESIZE, Ogre::Vector3(1,1,1)*TILESIZE, Ogre::Vector3(0,1,1)*TILESIZE,
}};

class HeatEngine : public Ogre::FrameListener, public Ogre::WindowEventListener, public OIS::KeyListener, public OIS::MouseListener, OgreBites::SdkTrayListener
{
public:
	HeatEngine(char *file = nullptr);
	~HeatEngine(void);
	
	void go(void);
	
protected:
	bool setup();
	bool configure(void);
	void chooseSceneManager(void);
	void createCamera(void);
	void createFrameListener(void);
	void createScene(void); // Override me!
	void destroyScene(void);
	void createViewports(void);
	void setupResources(void);
	void createResourceListener(void);
	void loadResources(void);
	
	// Ogre::FrameListener
	bool frameRenderingQueued(const Ogre::FrameEvent& evt);
	
	// OIS::KeyListener
	bool keyPressed( const OIS::KeyEvent &arg );
	bool keyReleased( const OIS::KeyEvent &arg );
	// OIS::MouseListener
	bool mouseMoved( const OIS::MouseEvent &arg );
	bool mousePressed( const OIS::MouseEvent &arg, OIS::MouseButtonID id );
	bool mouseReleased( const OIS::MouseEvent &arg, OIS::MouseButtonID id );
	
	// Ogre::WindowEventListener
	//Adjust mouse clipping area
	void windowResized(Ogre::RenderWindow* rw);
	//Unattach OIS before window shutdown (very important under Linux)
	void windowClosed(Ogre::RenderWindow* rw);
	
	void updateCamera();
	void updateEnginePanels();
	
	void updateSimulationObj();
	void updateWallObj();
	void manObjBoxAdd(Ogre::ManualObject *obj, Ogre::Vector3 pos, uint *count, AddMode mode = BOX, bool useOverride = false, Ogre::Real overrideTex = -1);
	void useTexCoord(Ogre::ManualObject *obj, int c, bool useOverride = false, Ogre::Real overrideTex = -1);
	
	Ogre::Root *mRoot;
	Ogre::Camera* mCamera;
	Ogre::SceneManager* mSceneMgr;
	Ogre::RenderWindow* mWindow;
	Ogre::String mResourcesCfg;
	Ogre::String mPluginsCfg;
	
	// OgreBites
	OgreBites::SdkTrayManager* mTrayMgr;
	OgreBites::ParamsPanel* mDetailsPanel;     // sample details panel
	OgreBites::ParamsPanel *mEnginePanel, *mToolPanel;
	bool mCursorWasVisible;                    // was cursor visible before dialog appeared
	bool mShutDown;
	
	// OIS Input devices
	OIS::InputManager* mInputManager;
	OIS::Mouse*    mMouse;
	OIS::Keyboard* mKeyboard;
	Ogre::ManualObject *mWalls, *mObjs;
	
	// Simulation
	Simulation *mSim;
	int mDepth;
	bool mShiftDown;
	bool mHideState[3];
	
	// Camera variables
	Ogre::Vector3 mCamPos;
	Ogre::Vector3 mLookPos;
};

int getInt(Ogre::ConfigFile *cf, std::string v, std::string def = "-1");

#endif // #ifndef __HeatEngine_h_
