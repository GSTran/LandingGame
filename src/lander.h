#pragma once

#include "ofMain.h"
#include  "ofxAssimpModelLoader.h"
#include "box.h"
#include "Octree.h"
#include "ray.h"

// Basic Shape class supporting matrix transformations and drawing.
// 
//
class Lander {
public:
  Lander() {
    pos = glm::vec3(0.0,1.0,0.0);
    velocity.set(0.0, 0.0, 0.0);
		acceleration.set(0.0, 0.0, 0.0);
		radius = 25;
		damping = 0.995;
		mass = 1;
		rotSpeed = 0.0;
		rotAccel = 0.0;
  }

	virtual void draw() {

		// draw a box by defaultd if not overridden
		//
		ofPushMatrix();
		ofMultMatrix(getTransform());
    model.drawFaces();
		ofPopMatrix();
		ofDrawLine(pos, glm::vec3(pos.x, pos.y - 10, pos.z));
		// Octree::drawBox(getTransformBounds());
	}

	glm::mat4 getTransform() {
		glm::mat4 T = glm::translate(glm::mat4(1.0), glm::vec3(pos));
		glm::mat4 R = glm::rotate(glm::mat4(1.0), glm::radians(rot), glm::vec3(0, 1, 0));
		glm::mat4 S = glm::scale(glm::mat4(1.0), scale);      // added this scale if you want to change size of object
		return T*R*S;
	}


	glm::vec3 pos;
	float rot = 0.0;    // degrees 

	glm::vec3 scale = glm::vec3(1, 1, 1);

	glm::vec3 headingY() {
		return glm::normalize(glm::vec3(getTransform() * glm::vec4(0, 1, 0, 0)));
	}

	glm::vec3 headingZ() {
		return glm::normalize(glm::vec3(getTransform() * glm::vec4(0, 0, 1, 0)));
	}

	glm::vec3 headingX() {
		return glm::normalize(glm::vec3(getTransform() * glm::vec4(1, 0, 0, 0)));
	}

  ofxAssimpModelLoader model;
	Box bounds;

  ofVec3f velocity;
	ofVec3f acceleration;
	ofVec3f forces;
	float		damping;
	float   mass;
	float   radius;
	float 	rotSpeed;
	float 	rotAccel;
	float		rotForce;

  void integrate();

  void loadModel();

	Box getTransformBounds();

	void landedLogic();

	float calculateAltitude(Octree ground);
};