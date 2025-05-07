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

		float rAccel = rotAccel;
		rAccel += (rotForce * (1.0 / mass));

		rotSpeed = rotSpeed + rAccel * dt;
		rotSpeed = rotSpeed * damping;

		forces.set(0, 0, 0);
		rotForce = 0.0;
	}	
}

void Lander::loadModel() {
  if (model.loadModel("geo/lander.obj")) {
		model.setScaleNormalization(false);
		glm::vec3 bboxCenter = model.getSceneCenter();
    glm::vec3 offset = -bboxCenter;

    model.setPosition(pos.x + offset.x, pos.y + offset.y, pos.z + offset.z);
	}
}

Box Lander::getTransformBounds() {
	ofVec3f bboxCenter = model.getSceneCenter();
	ofVec3f offset = -bboxCenter / 1.6;

	ofVec3f min = model.getSceneMin() + pos + offset;
	ofVec3f max = model.getSceneMax() + pos + offset;

	return Box(Vector3(min.x, min.y, min.z), Vector3(max.x, max.y, max.z));
}

void Lander::landedLogic() {
	velocity = glm::vec3(0.0);
	acceleration = glm::vec3(0.0);
	forces = glm::vec3(0.0);

	rotSpeed = 0.0;
	rotAccel = 0.0;
	rotForce = 0.0;
}

float Lander::calculateAltitude(Octree ground) {
	Ray altitudeSensor = Ray(Vector3(pos.x, pos.y, pos.z), Vector3(pos.x, pos.y - 100000, pos.z));
	TreeNode selectedNode;
	if(ground.intersect(altitudeSensor, ground.root, selectedNode))
		return glm::distance(ground.mesh.getVertex(selectedNode.points[0]), pos);
	return 0.0;
}