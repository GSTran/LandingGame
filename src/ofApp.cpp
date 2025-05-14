
//--------------------------------------------------------------
//
//  Kevin M. Smith
//
//  Octree Test - startup scene
// 
//
//  Student Name:   < Your Name goes Here >
//  Date: <date of last version>


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
	topCam.lookAt(ship.pos);

	ofEnableSmoothing();
	ofEnableDepthTest();

	// setup rudimentary lighting 
	//
	//initLightingAndMaterials();

	cout << "Moon Test Data: " << endl;
	mars.loadModel("geo/terrain.obj");

	mars.setScaleNormalization(false);

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
	spaceLight.enable();
	spaceLight.setSpotlight();
	spaceLight.setDiffuseColor(ofColor::cyan);
	spaceLight.setSpotlightCutOff(45);
	spaceLight.setAttenuation(1.0, 0.01, 0.01);



	// create sliders for testing
	//
	gui.setup();
	gui.add(numLevels.setup("Number of Octree Levels", 1, 1, 10));
	gui.add(bTimingInfo.setup("Timing Info", true));
	bHide = false;

	//  Create Octree for testing.
	//
	
	octree.create(mars.getMesh(0), 20);

	ship.loadModel();

	ofDisableArbTex();     // disable rectangular textures

	// load textures
	//
	if (!ofLoadImage(particleTex, "images/smoke.png")) {
		cout << "Particle Texture File: images/dot.png not found" << endl;
		ofExit();
	}

	initEmitters();

	ship.pos = glm::vec3(0.0, 50, 0.0);

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
		sizes.push_back(ofVec3f(100.0));
	}
	// upload the data to the vbo
	//
	int total = (int)points.size();
	vbo.clear();
	vbo.setVertexData(&points[0], total, GL_STATIC_DRAW);
	vbo.setNormalData(&sizes[0], total, GL_STATIC_DRAW);
}
 
//--------------------------------------------------------------
// incrementally update scene (animation)
//
void ofApp::update() {
	if (keymap[OF_KEY_UP])  {
		if (ofGetFrameNum() % 5 == 0) {
			emitter.sys->reset();
			emitter.start();
    }
		ship.forces += 3 * ship.headingY();
	}
	if (keymap['a'] || keymap['A']) ship.forces += -5 * ship.headingX();
	if (keymap['d'] || keymap['D']) ship.forces += 5 * ship.headingX();
	if (keymap['s'] || keymap['S']) ship.forces += 5 * ship.headingZ();
	if (keymap['w'] || keymap['W']) ship.forces += -5 * ship.headingZ();	
	if (keymap['e'] || keymap['E']) ship.rotForce += -30.0;
	if (keymap['q'] || keymap['Q']) ship.rotForce += 30.0;

	if (colBoxList.size() < 10) {
		ship.forces += glm::vec3(0.0, -2.0, 0.0); // Gravity Force
	} else if (!keymap[OF_KEY_UP]){
		// TODO: Fix clipping into ground when up arrow held just before crash
		if (ship.velocity.length() > 5.0f) cout << "CRASH" << endl;
		ship.landedLogic();
	}

	colBoxList.clear();
	octree.intersect(ship.getTransformBounds(), octree.root, colBoxList);

	ship.integrate();
	emitter.setPosition(ship.pos - glm::vec3(0.0, 5.0, 0.0));
	emitter.update();
	// cout << ship.calculateAltitude(octree) << endl;

	// cam.setPosition(ship.pos + glm::vec3(0, 10, 20));
	cam.setTarget(ship.pos);

	topCam.setPosition(ship.pos + glm::vec3(0, 20, 0));
	topCam.lookAt(ship.pos);


	
	//spaceLights 
	if(toggleLight){
		spaceLight.enable();
		spaceLight.setPosition(ship.pos - glm::vec3(0, 5, 0));
		spaceLight.lookAt(ship.pos - glm::vec3(0, 6, 0)); 
	}else{
		spaceLight.disable();
	}
}
//--------------------------------------------------------------
void ofApp::draw() {

	ofBackground(ofColor::black);

	glDepthMask(false);
	if (!bHide) gui.draw();
	glDepthMask(true);
	
	
	camPointer->begin();

	ofPushMatrix();
	ofEnableLighting();              // shaded mode
	mars.drawFaces();
	ofMesh mesh;

	// Game ship draw code starts here
	ship.draw();

	// draw colliding boxes
	//
	ofSetColor(ofColor::lightGreen);
	for (int i = 0; i < colBoxList.size(); i++) {
		Octree::drawBox(colBoxList[i]);
	}

	//debugging
	//keyLight.draw();
	//fillLight.draw();
	//rimLight.draw();
	//ambLight.draw();


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
    }
	else if (bDisplayOctree) {
		ofNoFill();
		octree.draw(numLevels, 0);
	}

	ofPopMatrix();
	camPointer->end();

	drawParticles();
}

void ofApp::drawParticles(){

	loadVbo();

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
	case '1':
		toggleLight = !toggleLight;
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
	case 'O':
	case 'o':
		bDisplayOctree = !bDisplayOctree;
		break;
	case ' ':
		cout << "Emitter" << endl;
		emitter.sys->reset();
		emitter.start();
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

	// if moving camera, don't allow mouse interaction
//
	if (cam.getMouseInputEnabled()) return;

	// if rover is loaded, test for selection
	//
}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button) {

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
}

void ofApp::initThreePointLighting() {
	
}

void ofApp::dragEvent(ofDragInfo dragInfo) {

}
