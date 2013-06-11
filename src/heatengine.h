#ifndef __HeatEngine_h_
#define __HeatEngine_h_

#include <OgreCamera.h>
#include <OgreEntity.h>
#include <OgreLogManager.h>
#include <OgreRoot.h>
#include <OgreViewport.h>
#include <OgreSceneManager.h>
#include <OgreRenderWindow.h>
#include <OgreConfigFile.h>

#include <OISEvents.h>
#include <OISInputManager.h>
#include <OISKeyboard.h>
#include <OISMouse.h>

#include <SdkTrays.h>
#include <SdkCameraMan.h>

#include "simulation.h"

#define TILESIZE 10

enum AddMode { XWALL, YWALL, ZWALL };

class HeatEngine : public Ogre::FrameListener, public Ogre::WindowEventListener, public OIS::KeyListener, public OIS::MouseListener, OgreBites::SdkTrayListener
{
public:
	HeatEngine(void);
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
	
	void updateManObj();
	void manObjBoxAdd(Ogre::ManualObject *obj, int xt, int yt, int z, RenderData *data);
	void useTexCoord(Ogre::ManualObject *obj, int c);
	
	Ogre::Root *mRoot;
	Ogre::Camera* mCamera;
	Ogre::SceneManager* mSceneMgr;
	Ogre::RenderWindow* mWindow;
	Ogre::String mResourcesCfg;
	Ogre::String mPluginsCfg;
	
	// OgreBites
	OgreBites::SdkTrayManager* mTrayMgr;
	OgreBites::ParamsPanel* mDetailsPanel;     // sample details panel
	bool mCursorWasVisible;                    // was cursor visible before dialog appeared
	bool mShutDown;
	
	//OIS Input devices
	OIS::InputManager* mInputManager;
	OIS::Mouse*    mMouse;
	OIS::Keyboard* mKeyboard;
	Ogre::ManualObject *mWalls, *mObjs;
	Simulation *mSim;
};

#endif // #ifndef __HeatEngine_h_
