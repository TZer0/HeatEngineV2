#include "heatengine.h"

HeatEngine::HeatEngine(char *file)
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
	mPause = true;
	mFreeRunningDelta = true;
	mDeltaValue = 1;
	bool fileLoaded = false;
	if (file != nullptr) {
		Ogre::FileSystemArchive fileMan("../testScenes", "");
		if (!fileMan.exists(file)) {
			std::cout << "Error: file " << file << " does not exist." << std::endl;
			throw;
		}
		Ogre::DataStreamPtr stream = fileMan.open(file);
		Ogre::ConfigFile conf;
		conf.load(stream);
		int sizes[3];
		
		// Load sizes
		for (int i = 0; i < 3; i++) {
			sizes[i] = getInt(&conf, SizeStrings[i]);
			if (sizes[i] < 1) {
				std::cout << "Error: " << SizeStrings[i] << " not defined or less than 1." << std::endl;
				throw;
			}
		}
		
		// Load materials
		Ogre::StringVector matStrings = conf.getMultiSetting("mat");
		if (matStrings.size() == 0) {
			std::cout << "Error: requires at least one defined material" << std::endl;
			throw;
		}
		
		
		std::vector<Material> mats;
		for (uint i = 0; i < matStrings.size(); i++) {
			std::vector<std::pair<double, double>> mPropVectors[3];
			Ogre::StringVector matProps = Ogre::StringUtil::split(matStrings[i], ",");
			if (matProps.size() != 6) {
				std::cout << "Error at mat[" << i << "], requires exactly three temperatures given" << std::endl;
				throw;
			}
			std::vector<double> props;
			for (int j = 0; j < 2; j++) {
				props.push_back(Ogre::StringConverter::parseReal(matProps[j]));
			}
			if (props[0] > props[1]) {
				std::cout << "Error: mat[" << i << "][0] is greater than mat[" 
					<< i << "][1]" << std::endl;
				throw;
			}
			
			for (int j = 2; j < 5; j++) {
				extractDoublePairs(conf.getSetting(matProps[j]), &mPropVectors[j-2]);
				if (mPropVectors[j-2].size() == 0) {
					std::cout << "Error: property " << matProps[j] << " for material[" << i << "] was not set correctly" << std::endl;
					throw;
				}
			}
			mats.push_back(Material(props[0], props[1], mPropVectors[0], mPropVectors[1], mPropVectors[2], matProps[5]));
		}
		
		double spacing = Ogre::StringConverter::parseReal(conf.getSetting("spacing", Ogre::StringUtil::BLANK, "0.1"));
		fileLoaded = true;
		mSim = new Simulation(sizes[0], sizes[1], sizes[2], spacing);
		CommonData *data = mSim->getData();
		for (uint i = 0; i < mats.size(); i++) {
			data->materials.push_back(mats[i]);
		}
		
		// Load blocks of material
		Ogre::StringVector blockStrings = conf.getMultiSetting("block");
		for (uint i = 0; i < blockStrings.size(); i++) {
			Ogre::StringVector blockProps = Ogre::StringUtil::split(blockStrings[i], ",");
			if (blockProps.size() != 9) {
				std::cout << "Error in block[" << i << "], size expected: 9, got: " << blockProps.size() << std::endl;
				throw;
			}
			std::vector<uint> props;
			for (uint j = 0; j < 8; j++) {
				props.push_back(Ogre::StringConverter::parseInt(blockProps[j]));
			}
			if (props[6] < 0 || props[6] >= mats.size()) {
				std::cout << "Skipping block[" << i << "], not such material: " << props[6] << std::endl;
				continue;
			}
			double temp = Ogre::StringConverter::parseReal(blockProps[8]);
			mSim->insertMaterialBlock(props[0], props[1], props[2], props[3], props[4], props[5], props[6], props[7], temp);
		}
		mDeltaValue = Ogre::StringConverter::parseReal(conf.getSetting("delta", Ogre::StringUtil::BLANK, "1"));
		mFreeRunningDelta = Ogre::StringConverter::parseInt(conf.getSetting("freedelta", Ogre::StringUtil::BLANK, "1"));
	}
	
	if (!fileLoaded) {
		mSim = new Simulation();
	}
	mSim->updateStatesAndLinks(true);
	CommonData *data = mSim->getData();
	mCamPos = Ogre::Vector3((data->xSize+2)*TILESIZE, (data->ySize+2)*TILESIZE, (data->zSize+2)*TILESIZE);
	mLookPos = Ogre::Vector3(0,0,0);
	mDepth = 1;
	mHideState[0] = mHideState[1] = mShiftDown = false;
	mHideState[2] = true;
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
	mCamera->setNearClipDistance(CLOSECLIP);
	mCamera->setFarClipDistance(FARCLIP);
	
	updateCamera();
}

void HeatEngine::updateCamera()
{
	mCamera->setPosition(mCamPos+mLookPos);
	mCamera->lookAt(mLookPos);
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
	
	Ogre::StringVector engineItems;
	engineItems.push_back("Selected box");
	engineItems.push_back("State");
	engineItems.push_back("Heat");
	engineItems.push_back("Freezing point");
	engineItems.push_back("Melting point");
	engineItems.push_back("Material");
	engineItems.push_back("Changable");
	engineItems.push_back("Time");
	engineItems.push_back("Selection depth");
	for (int i = 0; i < 3; i++) {
		engineItems.push_back(StateStrings[i]);
	}
	engineItems.push_back("State");
	mEnginePanel = mTrayMgr->createParamsPanel(OgreBites::TL_NONE, "EnginePanel", 200, engineItems);	
	mTrayMgr->moveWidgetToTray(mEnginePanel, OgreBites::TL_TOPLEFT, 0);
	mEnginePanel->show();
	
	
	Ogre::StringVector toolItems;
	mToolPanel = mTrayMgr->createParamsPanel(OgreBites::TL_NONE, "ToolPanel", 300, 5);
	mTrayMgr->moveWidgetToTray(mToolPanel, OgreBites::TL_TOPRIGHT);
	mToolPanel->show();
	
	mRoot->addFrameListener(this);
}

void HeatEngine::updateEnginePanels()
{
	CommonData *data = mSim->getData();
	int x = data->lastX; int y = data->lastY; int z = data->lastZ;
	Ogre::StringVector engineParams, toolParams, toolItems;
	if (data->withinArea(x,y,z)) {
		Area *ar = data->area[x][y][z];
		Material mat = data->materials.at(ar->mMat);
		engineParams.push_back(Ogre::StringConverter::toString(x) + " " +
			Ogre::StringConverter::toString(y) + " " + Ogre::StringConverter::toString(z));
		engineParams.push_back(StateStrings[ar->mState]);
		engineParams.push_back(Ogre::StringConverter::toString((Ogre::Real)ar->mH[data->latest]));
		for (int i = 0; i < 2; i++) {
			engineParams.push_back(Ogre::StringConverter::toString((Ogre::Real)mat.mTransPoints[i]));
		}
		engineParams.push_back(Ogre::StringConverter::toString(ar->mMat+1) + ":" + mat.mName);
		if (ar->mSource) {
			engineParams.push_back("No");
		} else {
			engineParams.push_back("Yes");
		}
	} else {
		engineParams.push_back("None");
		for (int i = 0; i < 6; i++) {
			engineParams.push_back("-");
		}
	}
	engineParams.push_back(Ogre::StringConverter::toString(data->time));
	engineParams.push_back(Ogre::StringConverter::toString(mDepth));
	for (int i = 0; i < 3; i++) {
		if (mHideState[i]) {
			engineParams.push_back("Hidden");
		} else {
			engineParams.push_back("Shown");
		}
	}
	if (mPause) {
		engineParams.push_back("Paused");
	} else {
		engineParams.push_back("Running");
	}
	
	toolItems.push_back("Selected tool");
	toolParams.push_back(SimToolStrings[data->tool]);
	if (data->tool == INSERTMATERIAL) {
		Material mat = data->materials.at(data->curMat);
		toolItems.push_back("Freezing point");
		toolItems.push_back("Melting point point");
		toolItems.push_back("Selected material");
		toolItems.push_back("Available materials");
		for (int i = 0; i < 2; i++) {
			toolParams.push_back(Ogre::StringConverter::toString((Ogre::Real)mat.mTransPoints[i]));
		}
		toolParams.push_back(Ogre::StringConverter::toString(data->curMat+1) + ":" + mat.mName);
		toolParams.push_back(Ogre::StringConverter::toString(data->materials.size()));
	}
	
	mEnginePanel->setAllParamValues(engineParams);
	mToolPanel->setAllParamNames(toolItems);
	mToolPanel->setAllParamValues(toolParams);
}

void HeatEngine::createScene(void) 
{
	mObjs = mSceneMgr->createManualObject();
	mWalls = mSceneMgr->createManualObject();
	Ogre::SceneNode *root = mSceneMgr->getRootSceneNode();
	root->attachObject(mObjs);
	root->attachObject(mWalls);
	updateWallObj();
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
	updateCamera();
	updateEnginePanels();
	
	if(mWindow->isClosed()) {
		return false;
	}
	
	if(mShutDown) {
		return false;
	}

	if (mFreeRunningDelta) {
		mSim->tick(evt.timeSinceLastFrame*mDeltaValue, evt.timeSinceLastFrame, mPause);
	} else {
		mSim->tick(mDeltaValue, evt.timeSinceLastFrame, mPause);
	}
	
	updateSimulationObj();
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
			mTrayMgr->moveWidgetToTray(mDetailsPanel, OgreBites::TL_BOTTOMRIGHT, 0);
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
	else if (arg.key == OIS::KC_Q) {
		if (mSim->getData()->tool == INSERTMATERIAL) {
			mSim->changeMaterial(1);
		}
	} 
	else if (arg.key == OIS::KC_A) {
		if (mSim->getData()->tool == INSERTMATERIAL) {
			mSim->changeMaterial(-1);
		}
	}
	else if (arg.key == OIS::KC_W) {
		mSim->setTool(INSERTMATERIAL);
	}
	else if (arg.key == OIS::KC_S) 
	{
		mSim->setTool(MOVE);
	}
	else if (arg.key == OIS::KC_E)
	{
		mSim->setTool(HEAT);
	}
	else if (arg.key == OIS::KC_D)
	{
		mSim->setTool(COOL);
	} 
	else if (arg.key == OIS::KC_C)
	{
		mSim->setTool(TOGGLESOURCE);
	} 
	else if (arg.key == OIS::KC_1)
	{
		mHideState[0] = !mHideState[0];
	}
	else if (arg.key == OIS::KC_2)
	{
		mHideState[1] = !mHideState[1];
	}
	else if (arg.key == OIS::KC_3)
	{
		mHideState[2] = !mHideState[2];
	}
	else if (arg.key == OIS::KC_P) {
		mPause = !mPause;
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
	Ogre::Vector3 diff = mLookPos-mCamPos;
	diff.y = 0;
	diff.normalise();
	Ogre::Vector3 rotDiff = Ogre::Quaternion(Ogre::Degree(90), Ogre::Vector3::UNIT_Y)*diff;
	rotDiff.y = 0;
	rotDiff.normalise();
	Ogre::Vector3 movVec = diff*arg.state.Y.rel + rotDiff*arg.state.X.rel;
	if (movVec.length() > 100) {
		return true;
	}
	if (arg.state.buttonDown(OIS::MB_Right))
	{
		mLookPos += movVec;
	} else if (arg.state.buttonDown(OIS::MB_Middle)) {
		mCamPos += movVec;
	} else if (arg.state.Z.rel != 0) {
		if (mKeyboard->isModifierDown(OIS::Keyboard::Shift)) {
			mDepth = std::max(mDepth + 1 - (arg.state.Z.rel < 0)*2, 1);
		} else {
			mCamPos *= pow(1.0001, -arg.state.Z.rel);
			if (mCamPos.length() < 10) {
				mCamPos /= mCamPos.length();
			}
		}
	}
	Ogre::Ray mouseRay = mTrayMgr->getCursorRay(mCamera);
	mSim->injectDepthAndMouse(mDepth, mouseRay.getOrigin(), mouseRay.getDirection());
	return true;
}

bool HeatEngine::mousePressed( const OIS::MouseEvent &arg, OIS::MouseButtonID id )
{
	if (mTrayMgr->injectMouseDown(arg, id)) return true;
	if (id == OIS::MB_Left) {
		mSim->click(true);
	}
	return true;
}

bool HeatEngine::mouseReleased( const OIS::MouseEvent &arg, OIS::MouseButtonID id )
{
	if (mTrayMgr->injectMouseUp(arg, id)) return true;
	if (id == OIS::MB_Left) {
		mSim->click(false);
	}
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

void HeatEngine::updateSimulationObj()
{
	CommonData *data = mSim->getData();
	mObjs->clear();
	mObjs->estimateVertexCount(data->xSize*data->ySize*data->zSize*6);
	mObjs->estimateIndexCount(data->xSize*data->ySize*data->zSize*6);
	for (int s = 0; s <= State::UNDEFINED; s++) {
		uint count = 0;
		State s_cast = (State) s;
		if (s == SOLID) {
			mObjs->begin("solidgradient");
		} else if (s == LIQUID) {
			mObjs->begin("liquidgradient");
		} else if (s == GAS) {
			mObjs->begin("gasgradient");
		} else if (s == UNDEFINED) {
			mObjs->begin("selection");
		}
		for (int x = 0; x < data->xSize; x++) {
			for (int y = 0; y < data->ySize; y++) {
				for (int z = 0; z < data->zSize; z++) {
					Area *area = data->area[x][y][z];
					State aState = area->mState;
					if (((aState == s_cast && s_cast != UNDEFINED) && (!mHideState[s] && !area->mHover))
						|| (s_cast == UNDEFINED && area->mHover)) {
						double *trans = data->materials.at(area->mMat).mTransPoints;
						double H = area->mH[data->latest];
						double texPos = 0;
						switch(s_cast) {
							case UNDEFINED:
								texPos = 0.5;
								break;
							default:
								H -= trans[s-1];
							case SOLID:
								texPos = H / trans[s];
						}
						manObjBoxAdd(mObjs, Ogre::Vector3(x, y, z)*TILESIZE, &count, BOX, true, texPos);
					}
				}
			}
		}
		mObjs->end();
	}
}

void HeatEngine::updateWallObj()
{
	CommonData *data = mSim->getData();
	mWalls->clear();
	mWalls->begin("defaultwall");
	uint count = 0;
	for (int x = 0; x < data->xSize; x++) {
		for (int y = 0; y < data->ySize; y++ ){
			manObjBoxAdd(mWalls, Ogre::Vector3(x, y, 0)*TILESIZE, &count, XWALL);
		}
	}
	for (int z = 0; z < data->zSize; z++) {
		for (int y = 0; y < data->ySize; y++ ){
			manObjBoxAdd(mWalls, Ogre::Vector3(0, y, z)*TILESIZE, &count, YWALL);
		}
	}
	for (int z = 0; z < data->zSize; z++) {
		for (int x = 0; x < data->xSize; x++ ){
			manObjBoxAdd(mWalls, Ogre::Vector3(x, 0, z)*TILESIZE, &count, ZWALL);
		}
	}
	mWalls->end();
}

void HeatEngine::manObjBoxAdd(Ogre::ManualObject *obj, Ogre::Vector3 pos, uint *count, AddMode mode, bool useOverride, Ogre::Real overrideTex)
{
	if (useOverride) {
		if (overrideTex >= 1) {
			overrideTex = 0.99;
		} else if (overrideTex <= 0) {
			overrideTex = 0.01;
		}
	}
	uint i = 0;
	uint lim = boxPositions.size();
	if (mode == BOX) {
	} else if (mode == XWALL) {
		lim = 4;
	} else if (mode == YWALL) {
		i = 4;
		lim = 8;
	} else if (mode == ZWALL) {
		i = 8;
		lim = 12;
	}
	for (; i < lim; i+=4)
	{
		*count += 4;
		obj->position(boxPositions[i] + pos);
		useTexCoord(obj, 0, useOverride, overrideTex);
		obj->position(boxPositions[i+1] + pos);
		useTexCoord(obj, 1, useOverride, overrideTex);
		obj->position(boxPositions[i+2] + pos);
		useTexCoord(obj, 2, useOverride, overrideTex);
		obj->position(boxPositions[i+3] + pos);
		useTexCoord(obj, 3, useOverride, overrideTex);
		obj->quad(*count-1, *count-2, *count-3, *count-4);
	}
}


void HeatEngine::useTexCoord(Ogre::ManualObject *obj, int c, bool useOverride, Ogre::Real overrideTex)
{
	if (useOverride) {
		obj->textureCoord(overrideTex, 0.5, 0);
		return;
	}
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


int getInt(Ogre::ConfigFile *cf, std::string v, std::string def)
{
	return Ogre::StringConverter::parseInt(cf->getSetting(v, Ogre::StringUtil::BLANK, def));
}

void extractDoublePairs(std::string v, std::vector< std::pair< double, double > >* dest)
{
	Ogre::StringVector vals = Ogre::StringUtil::split(v, ",");
	if (vals.size() % 2 == 1) {
		std::cout << "Incorrectly formated property string: " << v << std::endl;
		throw;
	}
	for (uint i = 0; i < vals.size(); i+=2) {
		double temp = Ogre::StringConverter::parseReal(vals[i]);
		double prop = Ogre::StringConverter::parseReal(vals[i+1]);
		dest->push_back(std::pair<double, double>(temp, prop));
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
		
		try {
			// Create application object
			if (argc == 1) {
				HeatEngine app;
				app.go();
			} else {
				HeatEngine app(argv[1]);
				app.go();
			}
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
