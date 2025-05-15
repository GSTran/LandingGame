#pragma once

#include "ofMain.h"
#include "ofxGui.h"
#include  "ofxAssimpModelLoader.h"
#include "Octree.h"
#include <glm/gtx/intersect.hpp>
#include "lander.h"
#include "ParticleEmitter.h"



class ofApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();

		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void mouseEntered(int x, int y);
		void mouseExited(int x, int y);
		void windowResized(int w, int h);
		void dragEvent2(ofDragInfo dragInfo);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);
		void drawAxis(ofVec3f);
		void initLightingAndMaterials();
		void savePicture();
		void toggleWireframeMode();
		void togglePointsDisplay();
		void toggleSelectTerrain();
		void setCameraTarget();
		bool mouseIntersectPlane(ofVec3f planePoint, ofVec3f planeNorm, ofVec3f &point);
		bool raySelectWithOctree(ofVec3f &pointRet);
		glm::vec3 getMousePointOnPlane(glm::vec3 p , glm::vec3 n);
		void loadVbo();
		void loadVbo2();
		void drawParticles();
		void initEmitters();
		void initThreePointLighting();
		void resetGame();
		void checkLanding(glm::vec2 point, bool &landedFlag);


		bool landed1 = false;
		bool landed2 = false;
		bool landed3 = false;
		int landingCount = 0;

		ofEasyCam cam, topCam;
		ofCamera *camPointer;
		ofxAssimpModelLoader mars, lander, target, target1, target2;
		ofLight keyLight, fillLight, ambLight, rimLight, spaceLight;
		Box boundingBox, landerBounds;
		vector<Box> colBoxList;
		bool bLanderSelected = false;
		Octree octree;
		TreeNode selectedNode;
		glm::vec3 mouseDownPos, mouseLastPos;
		ofImage backgroundImage;
		int fuelTimer, lastTime, currentTime;

		ofTrueTypeFont titleFont;
		ofTrueTypeFont subtitleFont;
		ofTrueTypeFont gameOverFont;
		int menuList = 1;


		bool bInDrag = false;
		bool toggleLight = false;
		string altitude;
		bool bTitleScreen = true;
		bool bDebugMode = false;
		bool bDisplayInstructs = false;

		float titleCamAngle = 0.0f;
		bool bFadingOut = false;
		float fadeAlpha = 0.0f;
		bool bGameOver = false;
		bool bGameWin = false;
		float fadeStartTime = 0.0f;
		float fadeDuration = 2.0f;
		float gameOverDelay = 3.0f;
		bool toggleAltitude = false;

		//sound bools and floats
		
		ofSoundPlayer rocket, titleSong, backgroundMusic, gameOverSound, explosion, menuScroll, menuSelect;

		bool titleFadingOut = false;
		float titleSongVolume = 1.0f;
		float backgroundMusicVolume = 0.0f;
		float backgroundFadingIn = false;
		float backgroundFadingOut = false;


		float fadeSpeed = 0.1f;
		float fadeSpeedBG = 0.05f;


		Lander ship;

		ParticleEmitter emitter, explosionEmitter;
		bool shipExplode = false;
		bool bMoveCamera = false;

		TurbulenceForce *turbForce;
		GravityForce *gravityForce;
		ImpulseRadialForce *radialForce;
		CyclicForce *cyclicForce;

		ofVec3f explosionForce;

		ofTexture  particleTex, explosionTex;

		
		

		// shaders
		//
		ofVbo vbo, vbo2;
		ofShader shader;

		ofxIntSlider numLevels;
		ofxPanel gui;
		ofxToggle bTimingInfo;

		bool bAltKeyDown;
		bool bCtrlKeyDown;
		bool bWireframe;
		bool bDisplayPoints;
		bool bPointSelected;
		bool bHide;
		bool pointSelected = false;
		bool bDisplayLeafNodes = false;
		bool bDisplayOctree = false;
		bool bDisplayBBoxes = false;
		
		bool bLanderLoaded;
		bool bTerrainSelected;

		ofVec3f selectedPoint;
		ofVec3f intersectPoint;

		vector<Box> bboxList;

		const float selectionRange = 4.0;

		map<int, bool> keymap;
		int cameraSelector = 1;

};
