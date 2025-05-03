#include "lander.h"

void Lander::integrate() {

	// (1) update position from velocity and time interval
	// (2) update velocity (based on acceleration
	// (3) multiply final result by the damping factor to sim drag
	//

	if (ofGetFrameRate() != 0) {
		float dt = 1.0/ofGetFrameRate();
		pos = pos + velocity * dt;

		ofVec3f accel = acceleration;
		accel += (forces * (1.0 / mass));

		velocity = velocity + accel * dt;
		velocity = velocity * damping;

		rot = rot + rotSpeed * dt;

		glm::vec3 rAccel = rotAccel;
		rAccel += (rotForce * (1.0 / mass));

		rotSpeed = rotSpeed + rAccel * dt;
		rotSpeed = rotSpeed * damping;

		forces.set(0, 0, 0);
		rotForce = glm::vec3(0.0);
	}	
}

void Lander::loadModel() {
  if (model.loadModel("geo/lander.obj")) {
		model.setScaleNormalization(false);
		ofVec3f bboxCenter = model.getSceneCenter();
    ofVec3f offset = -bboxCenter;

    model.setPosition(pos.x + offset.x, pos.y + offset.y, pos.z + offset.z);
		// model.setScale(0.05, 0.05, 0.05);
	}
}