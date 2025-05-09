
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
	cam.setDistance(10);
	cam.setNearClip(.1);
	cam.setFov(65.5);   // approx equivalent to 28mm in 35mm format
	ofSetVerticalSync(true);
	cam.disableMouseInput();
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
	// keyLight.setAreaLight(1, 1);
	// keyLight.setAmbientColor(ofFloatColor(0.1, 0.1, 0.1));
	// keyLight.setDiffuseColor(ofFloatColor(0.1, 0.1, 0.1));
	// keyLight.setSpecularColor(ofFloatColor(0.1, 0.1, 0.1));

	// keyLight.setPosition(glm::vec3(0.0, 20, 0.0));
	// keyLight.tilt(60); // Rotate the light around the X-axis


	// create sliders for testing
	//
	gui.setup();
	gui.add(numLevels.setup("Number of Octree Levels", 1, 1, 10));
	gui.add(bTimingInfo.setup("Timing Info", true));
	bHide = false;

	//  Create Octree for testing.
	//
	
	int t1 = ofGetElapsedTimeMillis();
	octree.create(mars.getMesh(0), 20);
	int t2 = ofGetElapsedTimeMillis();
	cout << "Time to build the tree: " << t2 - t1 << endl;
	
	cout << "Number of Verts: " << mars.getMesh(0).getNumVertices() << endl;

	testBox = Box(Vector3(3, 3, 0), Vector3(5, 5, 2));

	ship.loadModel();

	ofDisableArbTex();     // disable rectangular textures

	// load textures
	//
	if (!ofLoadImage(particleTex, "images/smoke.png")) {
		cout << "Particle Texture File: images/dot.png not found" << endl;
		ofExit();
	}
	emitter.setPosition(ofVec3f(0, 0, 0));
	emitter.setVelocity(ofVec3f(0, 10, 0));
	emitter.setOneShot(true);
	emitter.setEmitterType(DirectionalEmitter);
	emitter.setParticleRadius(100);
	emitter.setLifespanRange(ofVec2f(3.0, 5.0));
	emitter.setMass(1);
	emitter.setDamping(0.99);
	emitter.setGroupSize(1);

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
	if (collisionResolution) {
		glm::vec3 pos = lander.getPosition();
		pos += glm::vec3(0.0, 0.1, 0.0);
		lander.setPosition(pos.x, pos.y, pos.z);

		colBoxList.clear();
		ofVec3f min = lander.getSceneMin() + lander.getPosition();
		ofVec3f max = lander.getSceneMax() + lander.getPosition();
		Box bounds = Box(Vector3(min.x, min.y, min.z), Vector3(max.x, max.y, max.z));
		octree.intersect(bounds, octree.root, colBoxList);

		if (colBoxList.size() <= 10)
			collisionResolution = false;
	}	

	if (keymap[OF_KEY_UP])  {

		ship.forces += 2 * ship.headingY();
	}
	if (keymap['a'] || keymap['A']) ship.forces += -10 * ship.headingX();
	if (keymap['d'] || keymap['D']) ship.forces += 10 * ship.headingX();
	if (keymap['s'] || keymap['S']) ship.forces += 10 * ship.headingZ();
	if (keymap['w'] || keymap['W']) ship.forces += -10 * ship.headingZ();	
	if (keymap['e'] || keymap['E']) ship.rotForce += -30.0;
	if (keymap['q'] || keymap['Q']) ship.rotForce += 30.0;


	if (colBoxList.size() < 10) {
		ship.forces += glm::vec3(0.0, -1.0, 0.0); // Gravity Force
	} else if (!keymap[OF_KEY_UP]){
		if (ship.velocity.length() > 5.0f) cout << "CRASH" << endl;
		ship.landedLogic();
	}

	colBoxList.clear();
	octree.intersect(ship.getTransformBounds(), octree.root, colBoxList);

	ship.integrate();
	emitter.setPosition(ship.pos);
	emitter.update();
	cout << "Particle count: " << emitter.sys->particles.size() << endl;
	// cout << ship.calculateAltitude(octree) << endl;
}
//--------------------------------------------------------------
void ofApp::draw() {

	ofBackground(ofColor::black);

	glDepthMask(false);
	if (!bHide) gui.draw();
	glDepthMask(true);
	
	
	cam.begin();

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
	cam.end();

	drawParticles();


}

void ofApp::drawParticles(){

	loadVbo();

	// draw a grid
	//
	cam.begin();
	ofPushMatrix();
	ofRotateDeg(90, 0, 0, 1);
	ofSetLineWidth(1);
	ofSetColor(ofColor::dimGrey);
	ofDrawGridPlane();
	ofPopMatrix();
	cam.end();

	glDepthMask(GL_FALSE);

	// ofSetColor(255, 100, 90);

	// this makes everything look glowy :)
	//
	ofEnableBlendMode(OF_BLENDMODE_ADD);
	ofEnablePointSprites();
	ofEnableLighting();


	// begin drawing in the camera
	//
	shader.begin();
	cam.begin();

	// draw particle emitter here..
	//
	// emitter.draw();
	particleTex.bind();
	vbo.draw(GL_POINTS, 0, (int)emitter.sys->particles.size());
	particleTex.unbind();

	//  end drawing in the camera
	// 
	cam.end();
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
		if (cam.getMouseInputEnabled()) cam.disableMouseInput();
		else cam.enableMouseInput();
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

void ofApp::dragEvent(ofDragInfo dragInfo) {

}
