
//--------------------------------------------------------------
//
//  Kevin M. Smith
//
//
// 
//
//  Student Name:   Giovanni Tran, Angela Yang
//  Date: 05/16/2025s


#include "ofApp.h"
#include "Util.h"


//--------------------------------------------------------------
// setup scene, lighting, state and load geometry
//
void ofApp::setup(){
	bWireframe = false;
	bDisplayPoints = false;
	bAltKeyDown = false;
	bCtrlKeyDown = false;
	bLanderLoaded = false;
	bTerrainSelected = true;
//	ofSetWindowShape(1024, 768);

	camPointer = &cam;

	cam.setDistance(10);
	cam.setNearClip(.1);
	cam.setFov(65.5);   // approx equivalent to 28mm in 35mm format
	ofSetVerticalSync(true);
	cam.enableMouseInput();
	cam.setTarget(ship.pos);

	// TODO: Change to appear looking out of window when model updated
	topCam.setDistance(10);
	topCam.setNearClip(.1);
	topCam.setFov(65.5);   // approx equivalent to 28mm in 35mm format
	topCam.enableMouseInput();
	topCam.setPosition(ship.pos + glm::vec3(0, 20, 0));
	topCam.lookAt(ship.getCameraLookPos());

	ofEnableSmoothing();
	ofEnableDepthTest();

	rocket.load("sounds/rocket.mp3");
	titleSong.load("sounds/title2.mp3");
	gameOverSound.load("sounds/gameover.mp3");
	explosion.load("sounds/explosion.mp3");
	menuScroll.load("sounds/menu-scroll.mp3");
	menuSelect.load("sounds/menu-select.mp3");
	backgroundMusic.load("sounds/background.mp3");
	winSound.load("sounds/win.mp3");


	backgroundMusic.setLoop(true);
	backgroundMusic.setVolume(0.0f);
	titleSong.setLoop(true);
	titleSong.play();

	titleFont.load("fonts/titleFont.otf", 60, true, true, true);
	subtitleFont.load("fonts/subtitle.ttf", 20);
	gameOverFont.load("fonts/subtitle.ttf", 40);
	gameFont.load("fonts/gameFont.ttf", 15);
	
	fuelTimer = 120000; // 2 minutes in miliseconds
	lastTime = 0;

	backgroundImage.load("images/background.jpg");

	// setup rudimentary lighting 
	//initLightingAndMaterials();

	cout << "Moon Test Data: " << endl;
	mars.loadModel("geo/terrain.obj");
	mars.setScaleNormalization(false);

	target.loadModel("geo/target.obj");
	target.setScaleNormalization(false);
	target.setPosition(20, -10, 20);

	target1.loadModel("geo/target.obj");
	target1.setScaleNormalization(false);
	target1.setPosition(-40, -1, -270);

	target2.loadModel("geo/target.obj");
	target2.setScaleNormalization(false);
	target2.setPosition(300, -1, -100);

	spacebuilding1.loadModel("geo/spacebuilding1.obj");
	spacebuilding1.setScaleNormalization(false);
	spacebuilding1.setPosition(-80, 10, -320);

	
	initThreePointLighting();

	// create sliders for testing
	gui.setup();
	gui.add(numLevels.setup("Number of Octree Levels", 1, 1, 10));
	gui.add(bTimingInfo.setup("Timing Info", true));
	bHide = false;

	//  Create Octree for testing.
	ofMesh terrainmesh = mars.getMesh(0);

    std::vector<ofMesh> meshes = {
    target.getMesh(0),
    target1.getMesh(0),
    target2.getMesh(0),
	spacebuilding1.getMesh(0)
	};

	std::vector<ofxAssimpModelLoader*> models = {
    &target,
    &target1,
    &target2,
	&spacebuilding1
	};

    // Transform the vertices of the spacebuilding mesh to the correct position
    for (int m = 0; m < meshes.size(); ++m) {
    ofMesh& mesh = meshes[m];  // reference so we can modify it
    ofMatrix4x4 modelMatrix = models[m]->getModelMatrix();

    for (int i = 0; i < mesh.getNumVertices(); ++i) {
        ofVec3f v = mesh.getVertex(i);
        ofVec3f transformed = modelMatrix.preMult(v);
        mesh.setVertex(i, transformed);
    }
	terrainmesh.append(mesh);
	}	

    //terrainmesh.append(targetMesh);
    octree.create(terrainmesh, 20);

	//octree.create(mars.getMesh(0), 20);

	ship.loadModel();

	ofDisableArbTex();     // disable rectangular textures

	// load textures
	//
	if (!ofLoadImage(particleTex, "images/smoke.png")) {
		cout << "Particle Texture File: images/smoke.png not found" << endl;
		ofExit();
	}

	if (!ofLoadImage(explosionTex, "images/explosion.png")) {
		cout << "Explosion Texture File: images/explosion.png not found" << endl;
		ofExit();
	}

	initEmitters();

	ship.pos = glm::vec3(0.1, 2.0, 0.1); // DO NOT CHANGE, WILL BREAK ALTITUDE CALCULATIONS
	ship.rot = 220;
	explosionForce = glm::vec3(ofRandom(-1000, 1000), ofRandom(600, 800), ofRandom(-1000, 1000));

	// load the shader
	//
	#ifdef TARGET_OPENGLES
		shader.load("shaders_gles/shader");
	#else
		shader.load("shaders/shader");
	#endif
}



// Emitter rendering buffer
void ofApp::loadVbo() {
	if (emitter.sys->particles.size() < 1) return;

	vector<ofVec3f> sizes; 
	vector<ofVec3f> points;
	for (int i = 0; i < emitter.sys->particles.size(); i++) {
		points.push_back(emitter.sys->particles[i].position);
		sizes.push_back(ofVec3f(100.0) / (0.0006 * glm::distance2(cam.getPosition(), ship.pos)));
	}
	// upload the data to the vbo
	//
	int total = (int)points.size();
	vbo.clear();
	vbo.setVertexData(&points[0], total, GL_STATIC_DRAW);
	vbo.setNormalData(&sizes[0], total, GL_STATIC_DRAW);
}

void ofApp::loadVbo2() {
	if (explosionEmitter.sys->particles.size() < 1) return;

	vector<ofVec3f> sizes; 
	vector<ofVec3f> points;
	for (int i = 0; i < explosionEmitter.sys->particles.size(); i++) {
		points.push_back(explosionEmitter.sys->particles[i].position);
		sizes.push_back(ofVec3f(100.0) / (0.0006 * glm::distance2(cam.getPosition(), ship.pos)));
	}
	// upload the data to the vbo
	//
	int total = (int)points.size();
	vbo2.clear();
	vbo2.setVertexData(&points[0], total, GL_STATIC_DRAW);
	vbo2.setNormalData(&sizes[0], total, GL_STATIC_DRAW);
}

 

void ofApp::resetGame(){
	bFadingOut = false;
	bTitleScreen = true;
	fadeAlpha = 0.0f;
	fadeStartTime = 0.0f;
	bGameOver = false;
	bGameWin = false;

	landingCount = 0;
	landed1 = false;
	landed2 = false;
	landed3 = false;

	titleCamAngle = 0.0f;

	fuelTimer = 120000;

	titleSong.setVolume(1.0f);
	titleSongVolume = 1.0f;
	titleSong.play();
	

	ship.velocity = glm::vec3(0.0, 0.0, 0.0);
	ship.pos = glm::vec3(0.1, 2.0, 0.1);
	ship.rot = 220;
	ship.forces = glm::vec3(0.0, 0.0, 0.0);
	ship.rotForce = 0.0;
	emitter.sys->reset();

	camPointer = &cam;
}
//--------------------------------------------------------------
// incrementally update scene (animation)
//
void ofApp::update() {
	//menu screen
	//rotate camera around the ship until the user presses enter
	if(bTitleScreen){
		if (!bDebugMode) {
			float radius = 40.0f;
			titleCamAngle += 0.1f;
			float rad = glm::radians(titleCamAngle);
			glm::vec3 camPos = ship.pos + glm::vec3(cos(rad) * radius, 10, sin(rad) * radius);
			camPointer->setPosition(camPos);
			camPointer->lookAt(ship.pos);
			cam.disableMouseInput();
		}
		if (bDebugMode) {
			// cam.enableMouseInput();
		}
		
		return;
	}


	//music logic
	if (titleFadingOut) {
		titleSongVolume -= fadeSpeed;
		if (titleSongVolume <= 0.0f) {
			titleSongVolume = 0.0f;
			titleFadingOut = false;
			titleSong.stop();
			backgroundMusic.play();
			backgroundFadingIn = true;
		}
		titleSong.setVolume(titleSongVolume);
	}

	if(backgroundFadingIn){
		backgroundMusicVolume += fadeSpeedBG;
		if (backgroundMusicVolume >= 0.5f) {
			backgroundMusicVolume = 0.5f;
			backgroundFadingIn = false;
		}
		backgroundMusic.setVolume(backgroundMusicVolume);
	}

	if(backgroundFadingOut){
		backgroundMusicVolume -= fadeSpeedBG;
		if (backgroundMusicVolume <= 0.0f) {
			backgroundMusicVolume = 0.0f;
			backgroundFadingOut = false;
			backgroundMusic.stop();
		}
		backgroundMusic.setVolume(backgroundMusicVolume);
	}

	//end of music logic
	
	if (keymap[OF_KEY_UP] && fuelTimer > 0)  {
		currentTime = ofGetElapsedTimeMillis();
		if (lastTime == 0) lastTime = currentTime;
		fuelTimer -= (currentTime - lastTime);
		lastTime = ofGetElapsedTimeMillis();
		lastTime = currentTime;

		if (ofGetFrameNum() % 5 == 0) {
			emitter.sys->reset();
			emitter.start();
    }
		ship.forces += 3 * ship.headingY();
		if (!rocket.isPlaying()) {
    		rocket.setLoop(true);
			rocket.setSpeed(2.0);
    		rocket.setVolume(0.5);
    		rocket.play();
		}
	} else lastTime = 0;

	if (keymap['w'] || keymap['W']) ship.forces += -5 * ship.headingX();
	if (keymap['s'] || keymap['S']) ship.forces += 5 * ship.headingX();
	if (keymap['a'] || keymap['A']) ship.forces += 5 * ship.headingZ();
	if (keymap['d'] || keymap['D']) ship.forces += -5 * ship.headingZ();	
	if (keymap['e'] || keymap['E']) ship.rotForce += -30.0;
	if (keymap['q'] || keymap['Q']) ship.rotForce += 30.0;

	ship.forces += glm::vec3(0.0, -2.0, 0.0); // Gravity Force

	if (colBoxList.size() > 2 && !bGameOver) {
		ship.forces += glm::vec3(0.0, 2.0, 0.0); // Impulse Force
		if (ship.velocity.length() > 5.0) {
			shipExplode = true;
			explosionEmitter.sys->reset();
			explosionEmitter.start();
			explosion.setVolume(0.3);
			explosion.play();
			bGameOver = true;
			bFadingOut = true;
			fadeStartTime = ofGetElapsedTimef();

			if (!gameOverSound.isPlaying()) {
    			gameOverSound.play();
			}
		}
		if (!keymap[OF_KEY_UP] && !bGameOver)
			ship.landedLogic();

			glm::vec2 point1 = glm::vec2(20.0, 20.0);
			glm::vec2 point2 = glm::vec2(-40.0, -270.0);
			glm::vec2 point3 = glm::vec2(300.0, -100.0);
			

			checkLanding(point1, landed1);
			checkLanding(point2, landed2);
			checkLanding(point3, landed3);

		}
	
	if(landingCount == 3 && bGameWin == false){
			bGameWin = true;
			bFadingOut = true;
			fadeStartTime = ofGetElapsedTimef();

			if (!winSound.isPlaying()) {
    			winSound.play();
			}
	}

	if(bGameOver){
		camPointer = &cam;
		toggleAltitude = false;
		toggleLight = false;
		ship.forces += explosionForce;
		float elapsedTime = ofGetElapsedTimef() - fadeStartTime;
		fadeAlpha = ofMap(elapsedTime, 0.0f, fadeDuration, 0.0f, 255.0f, true);
		backgroundFadingOut = true;

		if(elapsedTime > gameOverDelay){
			resetGame();
		}
	}

	if(bGameWin){
		camPointer = &cam;
		toggleAltitude = false;
		toggleLight = false;
		float elapsedTime = ofGetElapsedTimef() - fadeStartTime;
		fadeAlpha = ofMap(elapsedTime, 0.0f, fadeDuration, 0.0f, 255.0f, true);
		backgroundFadingOut = true;

		if(elapsedTime > gameOverDelay){
			resetGame();
		}
		return;
	}

	colBoxList.clear();
	octree.intersect(ship.getTransformBounds(), octree.root, colBoxList);
	ship.integrate();

	if (toggleAltitude && ofGetFrameNum() % 20 == 0)
		altitude = "Altitude: " + ofToString(ship.calculateAltitude(octree));

	emitter.setPosition(ship.pos - glm::vec3(0.0, 5.0, 0.0));
	emitter.update();
	explosionEmitter.setPosition(ship.pos);
	explosionEmitter.update();

	cam.setTarget(ship.pos);

	topCam.setPosition(ship.getCameraPos());
	topCam.lookAt(ship.getCameraLookPos());

	//spaceLights 
	if(toggleLight){
		spaceLight.enable();
		spaceLight.setPosition(ship.pos - glm::vec3(0, 5, 0));
		spaceLight.lookAt(ship.pos - glm::vec3(0, 6, 0)); 
	}else{
		spaceLight.disable();
	}
  
	if (bMoveCamera) cam.disableMouseInput();
	else cam.enableMouseInput();
}

void ofApp::checkLanding(glm::vec2 point, bool &landedFlag){
	glm::vec2 ship2DPos = glm::vec2(ship.pos.x, ship.pos.z);
	if( glm::distance(ship2DPos, point) < 20.0 && ship.velocity.y == 0.0 && !landedFlag){
		landedFlag = true;
		landingCount++;
		showLandingMessage = true;
		lastLandingTime = ofGetElapsedTimef();
	}
}
 
//--------------------------------------------------------------
void ofApp::draw() {
	//background
	ofPushMatrix();
	ofDisableDepthTest();
	ofSetColor(ofColor::white);
	ofDisableLighting();
	backgroundImage.draw(-500, -500);
	ofEnableDepthTest();
	ofPopMatrix();

	glDepthMask(false);
	//if (!bHide) gui.draw();
	glDepthMask(true);

	if (showLandingMessage) {
		float timeSinceLanding = ofGetElapsedTimef() - lastLandingTime;
		if (timeSinceLanding < landingMessageDuration) {
			ofSetColor(255, 255, 0); 
			string message = "Landed   on  " + ofToString(landingCount) + " out  of   3   targets";
			gameFont.drawString(message, ofGetWidth() - gameFont.stringWidth(message) - 30, 30);
		} else {
			showLandingMessage = false;
		}
	}

	
	
	camPointer->begin();

	ofPushMatrix();
	ofEnableLighting();              // shaded mode
	mars.drawFaces();
	target.drawFaces();
	target1.drawFaces();
	target2.drawFaces();
	spacebuilding1.drawFaces();
	ofMesh mesh;

	// Game ship draw code starts here
	ship.draw();

	// draw colliding boxes
	//
	// ofSetColor(ofColor::lightGreen);
	// for (int i = 0; i < colBoxList.size(); i++) {
	// 	ofNoFill();
	// 	Octree::drawBox(colBoxList[i]);
	// }


	if(toggleLight){
		ofPushMatrix();
		ofTranslate(ship.pos - glm::vec3(0, 4, 0));
		ofRotateDeg(180, 1, 0, 0);
		ofSetColor(ofColor::cyan);
		ofEnableBlendMode(OF_BLENDMODE_ADD);     
		ofDrawCone(0, 0, 0, 2, 1);
		ofPopMatrix();
	}
	
	// Game ship draw code ends here

	// recursively draw octree
	//
	ofDisableLighting();
	int level = 0;
	//	ofNoFill();

	if (bDisplayLeafNodes) {
		octree.drawLeafNodes(octree.root);
		cout << "num leaf: " << octree.numLeaf << endl;
  } else if (bDisplayOctree) {
		ofNoFill();
		octree.draw(numLevels, 0);
	}

	ofPopMatrix();
	camPointer->end();

	
	//draw Text for Title Screen
	if (bTitleScreen) {
		if (!bDebugMode && !bDisplayInstructs) {
			float time = ofGetElapsedTimef();
			ofSetColor(ofColor::white);
			//blinking text effect 
			if (fmod(time, 1.0) < 0.5)  subtitleFont.drawString("Press      Enter      to    Confirm", ofGetWidth() / 2 - 190, ofGetHeight() - 160);
			if (fmod(time, 1.0) < 0.5)  subtitleFont.drawString("Use        Up    and    Down    to    Select", ofGetWidth() / 2 - 235, ofGetHeight() - 130);

			ofNoFill();
			ofSetColor(255, 165, 0);
			titleFont.drawStringAsShapes("Catstronauts", ofGetWidth() / 2 - 310, ofGetHeight() / 2 -240);
			
			ofFill();
			ofSetColor(114, 204, 242);
			titleFont.drawString("Catstronauts", ofGetWidth() / 2 - 310, ofGetHeight() / 2 -230);

			ofSetColor(ofColor::white);
			subtitleFont.drawString("Crash       Landing", ofGetWidth() / 2 - 100, ofGetHeight() /2 - 180);

			// Interactive Menu Elements
			if (menuList == 1) ofSetColor(114, 204, 242);
			subtitleFont.drawString("Start    Game", ofGetWidth() / 2 - 80, ofGetHeight() /2 - 80);
			ofSetColor(ofColor::white);

			if (menuList == 2) ofSetColor(114, 204, 242);
			subtitleFont.drawString("Instructions", ofGetWidth() / 2 - 75, ofGetHeight() /2);
			ofSetColor(ofColor::white);

			if (menuList == 3) ofSetColor(114, 204, 242);
			subtitleFont.drawString("Debug   Mode", ofGetWidth() / 2 - 82, ofGetHeight() /2 + 80);
			ofSetColor(ofColor::white);
		}
		if (bDisplayInstructs) {
			float x = ofGetWidth() / 2 - 450;
			float y = ofGetHeight() / 2 - 400;
			float lineHeight = subtitleFont.getLineHeight();

		
		  ofSetColor(ofColor::white);
			subtitleFont.drawString("Private   Kyuruga!", x, y);
			y += lineHeight * 2;
			ofSetColor(ofColor::red);
			subtitleFont.drawString("The   time   has   come   for   cats   to   claim   whatever   this   place   is!", x, y);
			y += lineHeight;


			ofSetColor(ofColor::white);
			subtitleFont.drawString("But   first   you   must   learn   how   to   use   this   ship", x, y);
			y += lineHeight * 2;
			string controls1 = "              To   move   the   ship   around   use   the   ";
			subtitleFont.drawString(controls1, x, y);
			ofSetColor(ofColor::blue);
			subtitleFont.drawString("WASD   keys", x + subtitleFont.stringWidth(controls1), y);
			ofSetColor(ofColor::white);
			y += lineHeight;
			string controls2 = "              To   rotate   the   ship   use   the   ";
			subtitleFont.drawString(controls2, x, y);
			ofSetColor(ofColor::blue);
			subtitleFont.drawString("Q   and   E   keys", x + subtitleFont.stringWidth(controls2), y);
			y += lineHeight;
			ofSetColor(ofColor::white);
			string controls3 = "              To   use   the   ship   thrusters   press   the   ";
			subtitleFont.drawString(controls3, x, y);
			ofSetColor(ofColor::blue);
			subtitleFont.drawString("Arrow   Up   key", x + subtitleFont.stringWidth(controls3), y);
			y += lineHeight;

			ofSetColor(ofColor::white);
			string controls4 = "              To   activate   the   onboard   lights   press   the   ";
			subtitleFont.drawString(controls4, x, y);
			ofSetColor(ofColor::blue);
			subtitleFont.drawString("1   key", x + subtitleFont.stringWidth(controls4), y);
			y += lineHeight;
			ofSetColor(ofColor::white);
			string controls5 = "              To   show   the   altitude   and   velocity   sensors   press   the   ";
			subtitleFont.drawString(controls5, x, y);
			ofSetColor(ofColor::blue);
			subtitleFont.drawString("2   key", x + subtitleFont.stringWidth(controls5), y);
			y += lineHeight;
			ofSetColor(ofColor::white);
			string controls6 = "              To   change   camera   views   press   the   ";
			subtitleFont.drawString(controls6, x, y);
			ofSetColor(ofColor::blue);
			subtitleFont.drawString("C   key", x + subtitleFont.stringWidth(controls6), y);
			y += lineHeight;
			ofSetColor(ofColor::white);
			string controls7 = "              To   change   the   third   person   camera   position   press   the   ";
			subtitleFont.drawString(controls7, x, y);
			ofSetColor(ofColor::blue);
			subtitleFont.drawString("V   key   and   click", x + subtitleFont.stringWidth(controls7), y);
			
			y += lineHeight * 2;
			ofSetColor(ofColor::white);


			string fuelText1 = "Be   careful!   We   could   only   afford ";
			subtitleFont.drawString(fuelText1, x, y);

			ofSetColor(ofColor::orange);
			subtitleFont.drawString("2   minutes   of   fuel!", x + subtitleFont.stringWidth(fuelText1), y);


			y += lineHeight * 2;


			ofSetColor(ofColor::white);
			string landingText1 = "Once   you   land   in   all ";
			subtitleFont.drawString(landingText1, x, y);


			ofSetColor(ofColor::green);  
			subtitleFont.drawString("three   landing   zones", x + subtitleFont.stringWidth(landingText1), y);


					ofSetColor(ofColor::white);
			y += lineHeight;
			subtitleFont.drawString("the   mission   is   complete!", x, y );


			y += lineHeight * 2;
			subtitleFont.drawString("Commander    Felicette", x, y);




			ofSetColor(114, 204, 242);
			subtitleFont.drawString("Return   to   Menu", ofGetWidth() / 2 - 160, ofGetHeight() - 160);
		}

		

		if (bDebugMode) {
			float time = ofGetElapsedTimef();
			ofSetColor(ofColor::white);
			//blinking text effect 
			if (fmod(time, 1.0) < 0.5)  subtitleFont.drawString("Press      Enter      to    Start", ofGetWidth() / 2 - 160, ofGetHeight() - 160);
		}
    return; 	
	}
	//end of Title Screen

	if(bFadingOut){
		ofFill();
		ofSetColor(0, 0, 0, fadeAlpha);
		ofDrawRectangle(0, 0, ofGetWidth(), ofGetHeight());

		ofSetColor(255, 255, 255, fadeAlpha);
		string endingMessage;
		if(bGameWin) endingMessage = "Mission Complete!";
		if(bGameOver) endingMessage = "Mission Failed!";
		ofDrawBitmapString(endingMessage, ofGetWindowWidth()/2 - 50, ofGetHeight()/2);
	}

	// Ship GUI
	if(toggleAltitude){
		ofSetColor(ofColor::white);
		gameFont.drawString(altitude,	ofGetWindowWidth() / 2 - gameFont.stringWidth(altitude) / 2, ofGetWindowHeight() - 30);
	}

	string fuelLeft = "Fuel: " + ofToString(fuelTimer / 1000) + "s";
	if (fuelTimer > 30000) {
		ofSetColor(ofColor::white);
		gameFont.drawString(fuelLeft, ofGetWindowWidth() - gameFont.stringWidth(fuelLeft) - 30, ofGetWindowHeight() - 30);
	} else {
		ofSetColor(ofColor::red);
		float time = ofGetElapsedTimef();
		//blinking text effect 
		if (fmod(time, 1.0) < 0.8) gameFont.drawString(fuelLeft, ofGetWindowWidth() - gameFont.stringWidth(fuelLeft) - 30, ofGetWindowHeight() - 30);
	}

	string velocity = "Velocity: " + ofToString(ship.velocity.length());
	if (ship.velocity.length() < 5.0) {
		ofSetColor(ofColor::white);
		gameFont.drawString(velocity, 30, ofGetWindowHeight() - 30);
	} else {
		ofSetColor(ofColor::red);
		float time = ofGetElapsedTimef();
		//blinking text effect 
		if (fmod(time, 1.0) < 0.9) gameFont.drawString(velocity, 30, ofGetWindowHeight() - 30);
	}

	string lights = "Light: ";
	if (toggleLight) {
		ofSetColor(ofColor::white);
		gameFont.drawString(lights, 30, 30);
		ofSetColor(ofColor::green);
		gameFont.drawString("ON", 30 + gameFont.stringWidth(lights) + 10, 30);
	} else {
		ofSetColor(ofColor::white);
		gameFont.drawString(lights, 30, 30);
		ofSetColor(ofColor::red);
		gameFont.drawString("OFF", 30 + gameFont.stringWidth(lights) + 10, 30);
	}

	string camera = "Move Camera: ";
	if (bMoveCamera) {
		ofSetColor(ofColor::white);
		gameFont.drawString(camera, 30, 35 + gameFont.stringHeight(lights));
		ofSetColor(ofColor::green);
		gameFont.drawString("ON", 30 + gameFont.stringWidth(camera) + 10, 35 + gameFont.stringHeight(lights));
	} else {
		ofSetColor(ofColor::white);
		gameFont.drawString(camera, 30, 40 + gameFont.stringHeight(lights));
		ofSetColor(ofColor::red);
		gameFont.drawString("OFF", 30 + gameFont.stringWidth(camera) + 10, 40 + gameFont.stringHeight(lights));
	}

	drawParticles();

}

void ofApp::drawParticles(){

	loadVbo();
	loadVbo2();

	ofSetColor(ofColor::dimGrey);
	glDepthMask(GL_FALSE);


	// this makes everything look glowy :)
	//
	ofEnableBlendMode(OF_BLENDMODE_ADD);
	ofEnablePointSprites();
	ofEnableLighting();


	// begin drawing in the camera
	//
	shader.begin();
	camPointer->begin();

	// draw particle emitter here..
	//
	// emitter.draw();
	particleTex.bind();
	vbo.draw(GL_POINTS, 0, (int)emitter.sys->particles.size());
	particleTex.unbind();

	explosionTex.bind();
	vbo2.draw(GL_POINTS, 0, (int)explosionEmitter.sys->particles.size());
	explosionTex.unbind();

	//  end drawing in the camera
	// 
	camPointer->end();
	shader.end();

	ofDisablePointSprites();
	ofDisableBlendMode();
	ofDisableLighting();
	ofEnableAlphaBlending();

	// set back the depth mask
	//
	glDepthMask(GL_TRUE);
}

void ofApp::keyPressed(int key) {

	switch (key) {
	case OF_KEY_RETURN:
		if(bTitleScreen){
			if (bDisplayInstructs) {
				bDisplayInstructs = false;
				menuSelect.play();
				break;
			}
			if (bDebugMode) {
				bTitleScreen = false;
				bDebugMode = false;
				menuSelect.play();
				break;
			}
			if (!bDisplayInstructs && !bDebugMode) {
				switch (menuList) {
					case 1:
						bTitleScreen = false;
						titleFadingOut = true;
						cam.enableMouseInput();
						break;
					case 2:
						bDisplayInstructs = true;
						// Display Instructions Here
						break;
					case 3:
						// Debug Mode Here
						bDebugMode = true;
						break;
					default:
						break;
				}
				menuSelect.play();
			}
		}
		break;
	case OF_KEY_DOWN:
		if(bTitleScreen && !bDisplayInstructs && !bDebugMode) {
			menuList++;
			menuScroll.play();
			if (menuList > 3) menuList = 1;
			if (menuList < 1) menuList = 3;
		}
		break;
	case OF_KEY_UP:
		if(bTitleScreen && !bDisplayInstructs && !bDebugMode) {
			menuList--;
			menuScroll.play();
			if (menuList > 3) menuList = 1;
			if (menuList < 1) menuList = 3;
		}
		break;
	case '1':
		toggleLight = !toggleLight;
		break;
	case '2':
		toggleAltitude = !toggleAltitude;
		break;
	case 'C':
	case 'c':
		if (cameraSelector == 1) {
			camPointer = &topCam;
			cameraSelector *= -1;
		} else {
			camPointer = &cam;
			cameraSelector *= -1;
		}
		break;
	case 'o':
		//bDisplayOctree = !bDisplayOctree;
		break;
	case 'V':
	case 'v':
		bMoveCamera = !bMoveCamera;
		break;
	default:
		break;
	}
	keymap[key] = true;

}

void ofApp::toggleWireframeMode() {
	bWireframe = !bWireframe;
}

void ofApp::toggleSelectTerrain() {
	bTerrainSelected = !bTerrainSelected;
}

void ofApp::togglePointsDisplay() {
	bDisplayPoints = !bDisplayPoints;
}

void ofApp::keyReleased(int key) {

	switch (key) {
	case OF_KEY_ALT:
		cam.disableMouseInput();
		bAltKeyDown = false;
		break;
	case OF_KEY_CONTROL:
		bCtrlKeyDown = false;
		break;
	case OF_KEY_SHIFT:
		break;
	case OF_KEY_UP:
		rocket.stop();
		break;
	default:
		break;
	}
	keymap[key] = false;
}


//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

	
}


//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button) {

	// if moving camera, don't allow mouse interaction
	//
	if (cam.getMouseInputEnabled()) return;

	if (bMoveCamera) {
		ofVec3f p;
		raySelectWithOctree(p);
		cam.setPosition(p + ofVec3f(0.0, 1.0, 0.0));
	}

	if (bDebugMode) {
		glm::vec3 origin = cam.getPosition();
		glm::vec3 mouseWorld = cam.screenToWorld(glm::vec3(mouseX, mouseY, 0));
		glm::vec3 mouseDir = glm::normalize(mouseWorld - origin);

		Box bounds = ship.getTransformBounds();
		bool hit = bounds.intersect(Ray(Vector3(origin.x, origin.y, origin.z), Vector3(mouseDir.x, mouseDir.y, mouseDir.z)), 0, 10000);
		if (hit) {
			mouseDownPos = getMousePointOnPlane(ship.pos, cam.getZAxis());
			mouseLastPos = mouseDownPos;
			bInDrag = true;
		}
	}

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button) {
	if (cam.getMouseInputEnabled()) return;

	if (bInDrag) {
		glm::vec3 shipPos = ship.pos;

		glm::vec3 mousePos = getMousePointOnPlane(shipPos, cam.getZAxis());
		glm::vec3 delta = mousePos - mouseLastPos;
	
		shipPos += delta;
		ship.pos = glm::vec3(shipPos.x, shipPos.y, shipPos.z);
		mouseLastPos = mousePos;

		ofVec3f min = ship.model.getSceneMin() + ship.model.getPosition();
		ofVec3f max = ship.model.getSceneMax() + ship.model.getPosition();
	}

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button) {
	bInDrag = false;
}



// Set the camera to use the selected point as it's new target
//  
void ofApp::setCameraTarget() {

}


//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}



//--------------------------------------------------------------
// setup basic ambient lighting in GL  (for now, enable just 1 light)
//
void ofApp::initLightingAndMaterials() {

	static float ambient[] =
	{ .5f, .5f, .5, 1.0f };
	static float diffuse[] =
	{ 1.0f, 1.0f, 1.0f, 1.0f };

	static float position[] =
	{5.0, 5.0, 5.0, 0.0 };

	static float lmodel_ambient[] =
	{ 1.0f, 1.0f, 1.0f, 1.0f };

	static float lmodel_twoside[] =
	{ GL_TRUE };


	glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
	glLightfv(GL_LIGHT0, GL_POSITION, position);

	glLightfv(GL_LIGHT1, GL_AMBIENT, ambient);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, diffuse);
	glLightfv(GL_LIGHT1, GL_POSITION, position);


	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lmodel_ambient);
	glLightModelfv(GL_LIGHT_MODEL_TWO_SIDE, lmodel_twoside);

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	// glEnable(GL_LIGHT1);
	glShadeModel(GL_SMOOTH);
} 

void ofApp::initEmitters() {
	emitter.setPosition(ofVec3f(0, 0, 0));
	emitter.setVelocity(ofVec3f(0, -15, 0));
	emitter.setOneShot(true);
	emitter.setEmitterType(DirectionalEmitter);
	emitter.setParticleRadius(10);
	emitter.setLifespanRange(ofVec2f(4.0, 5.0));
	emitter.setMass(0.1);
	emitter.setDamping(0.97);
	emitter.setGroupSize(1);

	turbForce = new TurbulenceForce(ofVec3f(-2.5, 0.0, -2.5), ofVec3f(2.5, 0.0, 2.5));
	emitter.sys->addForce(turbForce);

	explosionEmitter.setPosition(ofVec3f(0, 0, 0));
	explosionEmitter.setVelocity(ofVec3f(0, 10, 0));
	explosionEmitter.setOneShot(true);
	explosionEmitter.setEmitterType(RadialEmitter);
	explosionEmitter.setParticleRadius(10);
	explosionEmitter.setLifespanRange(ofVec2f(15.0, 20.0));
	explosionEmitter.setMass(0.1);
	explosionEmitter.setDamping(0.97);
	explosionEmitter.setGroupSize(1000);

	gravityForce = new GravityForce(ofVec3f(0, -2.0, 0));
	radialForce = new ImpulseRadialForce(400);

	explosionEmitter.sys->addForce(turbForce);
	explosionEmitter.sys->addForce(gravityForce);
	explosionEmitter.sys->addForce(radialForce);


}

void ofApp::initThreePointLighting() {
	keyLight.setup();
	keyLight.enable();
	keyLight.setDirectional();
	keyLight.setAreaLight(1000, 1000);
	keyLight.setAmbientColor(ofFloatColor(0.1, 0.1, 0.1));
	keyLight.setDiffuseColor(ofColor::white);
	keyLight.setPosition(glm::vec3(500, 500, 0.0));

	fillLight.setup();
	fillLight.enable();
	fillLight.setDirectional();
	fillLight.setAmbientColor(ofFloatColor(0.05, 0.05, 0.05));
	fillLight.setDiffuseColor(ofColor::orange);               
	fillLight.setPosition(glm::vec3(-400, 300, 400));          
	fillLight.lookAt(glm::vec3(0, 0, 0));   
	
	rimLight.setup();
	rimLight.enable();
	rimLight.setDirectional();
	rimLight.setDiffuseColor(ofFloatColor(0.2, 0.2, 1.0));
	rimLight.setAmbientColor(ofFloatColor(0.1, 0.1, 0.3));   
	rimLight.setPosition(glm::vec3(400, 300, -400));         
	rimLight.lookAt(glm::vec3(0, 0, 0));

	ambLight.setup();
	ambLight.enable();
	ambLight.setDirectional();
	ambLight.setDiffuseColor(ofColor::white); 
	keyLight.setAmbientColor(ofFloatColor(0.1, 0.1, 0.1));     
	ambLight.setPosition(glm::vec3(300, 300, 500));         
	ambLight.lookAt(glm::vec3(0, 0, 0));

	spaceLight.setup();
	spaceLight.setSpotlight();
	spaceLight.setDiffuseColor(ofColor::cyan);
	spaceLight.setSpotlightCutOff(45);
	spaceLight.setAttenuation(1.0, 0.01, 0.01);
	
}

void ofApp::dragEvent(ofDragInfo dragInfo) {

}

bool ofApp::raySelectWithOctree(ofVec3f &pointRet) {
	ofVec3f mouse(mouseX, mouseY);
	ofVec3f rayPoint = cam.screenToWorld(mouse);
	ofVec3f rayDir = rayPoint - cam.getPosition();
	rayDir.normalize();
	Ray ray = Ray(Vector3(rayPoint.x, rayPoint.y, rayPoint.z),
		Vector3(rayDir.x, rayDir.y, rayDir.z));
	
	int t1 = ofGetElapsedTimeMillis();
	pointSelected = octree.intersect(ray, octree.root, selectedNode);
	int t2 = ofGetElapsedTimeMillis();

	if (bTimingInfo) cout << "Time for ray selection: " << t2 - t1 << endl;

	if (pointSelected) {
		pointRet = octree.mesh.getVertex(selectedNode.points[0]);
	}

	return pointSelected;
}

glm::vec3 ofApp::getMousePointOnPlane(glm::vec3 planePt, glm::vec3 planeNorm) {
	// Setup our rays
	//
	glm::vec3 origin = cam.getPosition();
	glm::vec3 camAxis = cam.getZAxis();
	glm::vec3 mouseWorld = cam.screenToWorld(glm::vec3(mouseX, mouseY, 0));
	glm::vec3 mouseDir = glm::normalize(mouseWorld - origin);
	float distance;

	bool hit = glm::intersectRayPlane(origin, mouseDir, planePt, planeNorm, distance);

	if (hit) {
		// find the point of intersection on the plane using the distance 
		// We use the parameteric line or vector representation of a line to compute
		//
		// p' = p + s * dir;
		//
		glm::vec3 intersectPoint = origin + distance * mouseDir;

		return intersectPoint;
	}
	else return glm::vec3(0, 0, 0);
}