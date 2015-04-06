//
//  TableSimulator.cpp
//  basicExample
//
//  Created by Tangible Media Group on 3/20/14.
//
//

#include "TableSimulator.h"


TableSimulator::TableSimulator(ReliefIOManager *reliefManager) {
    mIOManager = reliefManager;
    setup();
}

void TableSimulator::setup(){
    // turn on smooth lighting //
    ofSetSmoothLighting(true);

    pointLight.setDiffuseColor(ofColor(255, 255, 255));
    pointLight.setSpecularColor(ofColor(0, 0, 0));
    pointLight.setPosition(50, -400, 500);

    material.setSpecularColor(ofColor(255, 255, 255));
	material.setShininess(64);
    
}

//--------------------------------------------------------------
//
// Setup interaction area for easyCam
// Uses the viewport as container. To replace with smaller area
// change value of vp
//
//--------------------------------------------------------------
void TableSimulator::drawInteractionArea(int px, int py, int w, int h){
	//ofRectangle vp = ofGetCurrentViewport();
    ofRectangle vp(px, py, w, h);
	float r = MIN(vp.width, vp.height) * 0.5f;
	float x = vp.width * 0.5f;
	float y = vp.height * 0.5f;
    
	ofPushStyle();
	ofSetLineWidth(2);
	ofSetColor(255, 255, 0);
	ofNoFill();
	glDepthMask(false);
	ofSetCircleResolution(100);
	ofEnableSmoothing();
	ofCircle(px+x, py+y, r);
	glDepthMask(true);
	ofPopStyle();
}

//--------------------------------------------------------------
//
// Program to setup and draw the pins on map.
// The pin simulation itself is drawn using drawPinDisplaySimulation.
// This function mainly handles openGL setup
//
// @todo matt, refactor setup out of this
//
//--------------------------------------------------------------
void TableSimulator::drawTableCamView(int px, int py, int w, int h, float zoom) {
    
    ofRectangle vp(px, py, w, h);
    
    // easyCam area is rotated around by mouse movement
    // in combination with keyboard commands
    cam.begin(vp);

    // set scale here. We dont need this inside the draw simulation method
    // anymore. This gives us more consistancy in scaling based on other factors
    // @note removing this in favor of cam.setDistance(100); causes the shading on the
    // table to dissappear!
    ofScale(zoom,zoom,zoom);
    
    // set initial rotation to be sudo ortho view
    ofRotateX(-70); // simulate height looking down on table, -90 is shorter and -20 is taller
    ofRotateZ(25); // simulate walk around table
    
    ofEnableLighting();

    // the position of the light must be updated every frame, 
    // call enable() so that it can update itself //
    pointLight.enable();
    material.begin();
    ofFill();
    ofPushMatrix();

    // draw the simulation
    drawPinDisplaySimulation();

    ofPopMatrix();
	material.end();
    ofDisableLighting();
    cam.end();
    ofSetColor(255);
}

//--------------------------------------------------------------
//
// After setup, run this to draw the table on pins.
//
// gets pin height
//
//--------------------------------------------------------------
void TableSimulator::drawPinDisplaySimulation() {
    // define 3d table and pin boxes.
    ofBoxPrimitive pin = ofBoxPrimitive(0.95, 0.95, 5);
    ofBoxPrimitive table = ofBoxPrimitive(1.2 * RELIEF_SIZE_X, 1.2 * RELIEF_SIZE_Y, 20);

    // enable more GL stuff
    // @todo this is added in the setup, right?
    glEnable(GL_DEPTH_TEST);
    
    // save current matrix
    ofPushMatrix();

    // draw the table
    ofSetColor(100);
    table.draw();
    
    // account for pin width, depth, height
    // @todo move pin width definition to constant
    // black is all the way down, white is all the up
    // gray is in the middle
    ofTranslate(0.95/2, 0.95/2, -5/2);
    
    // move to area where we want to draw the pins.
    ofTranslate(-RELIEF_SIZE_X/2, -RELIEF_SIZE_Y/2, 10);
    ofSetColor(200);
    
    // draw the pins mapped to height.
    for (int y = 0; y < RELIEF_SIZE_Y; y++) {
        for (int x = 0; x < RELIEF_SIZE_X; x++) {
            int height = pinHeightMap[x][y];
            //ofSetColor(height);
            ofPushMatrix();
            ofTranslate(x, y, height/51.f);
            pin.draw();
            ofPopMatrix();
        }
    }
    ofPopMatrix();
    glDisable(GL_DEPTH_TEST);

    ofPopStyle();
}


//--------------------------------------------------------------
//
// Updates local pin array with shape manager pin array
//
// @todo Matt replace with memcpy or something more effecient!
//
//--------------------------------------------------------------
void TableSimulator::update() {
    memcpy(pinHeightMap, mIOManager->pinHeightToRelief, sizeof(unsigned char) * RELIEF_SIZE_X * RELIEF_SIZE_Y);
}

//--------------------------------------------------------------

ofMatrix3x3 TableSimulator::mat4ToMat3(ofMatrix4x4 mat4)
{
    return ofMatrix3x3(mat4._mat[0][0],
                       mat4._mat[0][1],
                       mat4._mat[0][2],
                       mat4._mat[1][0],
                       mat4._mat[1][1],
                       mat4._mat[1][2],
                       mat4._mat[2][0],
                       mat4._mat[2][1],
                       mat4._mat[2][2]);
}