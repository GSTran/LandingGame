
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
	initLightingAndMaterials();

	cout << "Moon Test Data: " << endl;
	mars.loadModel("geo/moon-houdini.obj");

	mars.setScaleNormalization(false);

	// keyLight.setup();
	// keyLight.enable();
	// keyLight.setAreaLight(10, 10);
	// keyLight.setAmbientColor(ofFloatColor(0.1, 0.1, 0.1));
	// keyLight.setDiffuseColor(ofFloatColor(0.1, 0.1, 0.1));
	// keyLight.setSpecularColor(ofFloatColor(0.1, 0.1, 0.1));

	// keyLight.setPosition(glm::vec3(0.0, 1000, 0.0));
	// keyLight.tilt(90);


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
		cout << "Particle Texture File: images/smoke.png not found" << endl;
		ofExit();
	}

	if (!ofLoadImage(explosionTex, "images/explosion.png")) {
		cout << "Explosion Texture File: images/explosion.png not found" << endl;
		ofExit();
	}

	initEmitters();

	ship.pos = glm::vec3(0.0, 10, 0.0);

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

void ofApp::loadVbo2() {
	if (explosionEmitter.sys->particles.size() < 1) return;

	vector<ofVec3f> sizes; 
	vector<ofVec3f> points;
	for (int i = 0; i < explosionEmitter.sys->particles.size(); i++) {
		points.push_back(explosionEmitter.sys->particles[i].position);
		sizes.push_back(ofVec3f(100.0));
	}
	// upload the data to the vbo
	//
	int total = (int)points.size();
	vbo2.clear();
	vbo2.setVertexData(&points[0], total, GL_STATIC_DRAW);
	vbo2.setNormalData(&sizes[0], total, GL_STATIC_DRAW);
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
		if (ship.velocity.length() > 5.0f) {
			cout << "CRASH" << endl;
			shipExplode = true;
			explosionEmitter.sys->reset();
			explosionEmitter.start();
		}
		ship.landedLogic();
	}

	colBoxList.clear();
	octree.intersect(ship.getTransformBounds(), octree.root, colBoxList);

	ship.integrate();
	emitter.setPosition(ship.pos - glm::vec3(0.0, 5.0, 0.0));
	emitter.update();

	explosionEmitter.setPosition(ship.pos);
	explosionEmitter.update();

	// cout << ship.calculateAltitude(octree) << endl;

	cam.setTarget(ship.pos);

	topCam.setPosition(ship.pos + glm::vec3(0, 20, 0));
	topCam.lookAt(ship.pos);
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

	// keyLight.draw();
	
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
		explosionEmitter.sys->reset();
		explosionEmitter.start();
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
	radialForce = new ImpulseRadialForce(100);

	explosionEmitter.sys->addForce(turbForce);
	explosionEmitter.sys->addForce(gravityForce);
	explosionEmitter.sys->addForce(radialForce);


}

void ofApp::initThreePointLighting() {
	
}

void ofApp::dragEvent(ofDragInfo dragInfo) {

}
