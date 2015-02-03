//
//  KinectTracker.cpp
//  Relief2
//
//  Created by Sean Follmer on 3/24/13.
//
//

#include "KinectTracker.h"

void KinectTracker::setup(){
    ofSetLogLevel(OF_LOG_VERBOSE);
	// enable depth->video image calibration
	kinect.setRegistration(true);
    
	kinect.init();
	kinect.open();

    colorImg.allocate(kinect.width, kinect.height);
	depthImg.allocate(kinect.width, kinect.height);

    finger_contourFinder.bTrackBlobs = true;
    finger_contourFinder.bTrackFingers = true;
    verdana.loadFont("frabk.ttf", 8, true, true);
    
    // allocate threshold images
	grayThreshFar.allocate(kinect.width, kinect.height);
    depthImgThresCropped.allocate(roiW, roiH);
	
	nearThreshold = 255;
	farThreshold = 234;
	bThreshWithOpenCV = true;
    
    depthImageAlpha.allocate(640, 480, OF_IMAGE_COLOR_ALPHA);
    colorImageAlpha.allocate(640, 480, OF_IMAGE_COLOR_ALPHA);
    handCropMask.allocate(640, 480, GL_RGBA);
    
    replacementHandsColor[0].loadImage("GardenShovel.png");
    replacementHandsDepth[0].loadImage("GardenShovel_depth.png");
    replacementHandsColor[1].loadImage("move_well_open.png");
    replacementHandsDepth[1].loadImage("move_well_open.png");
    replacementHandsColor[2].loadImage("move_well_closed.png");
    replacementHandsDepth[2].loadImage("move_well_closed.png");
    replacementHandsColor[3].loadImage("move_well_wide_open.png");
    replacementHandsDepth[3].loadImage("move_well_wide_open.png");
    replacementHandsColor[4].loadImage("move_well_hook.png");
    replacementHandsDepth[4].loadImage("move_well_hook.png");
    replacementHandsColor[5].loadImage("move_line.png");
    replacementHandsDepth[5].loadImage("move_line.png");
    replacementHandsColor[6].loadImage("move_ramp.png");
    replacementHandsDepth[6].loadImage("move_ramp.png");
}

void KinectTracker::exit() {
    kinect.setCameraTiltAngle(0); // zero the tilt on exit
	kinect.close();
    
}

void KinectTracker::update(){
    kinect.update();
	
	// there is a new frame and we are connected
	if(kinect.isFrameNew()) {
        
        colorImg.setFromPixels(kinect.getPixels(), kinect.width, kinect.height);
        colorImg.mirror(1,1);
        colorImg.flagImageChanged();
        
		depthImg.setFromPixels(kinect.getDepthPixels(), kinect.width, kinect.height);
        depthImg.mirror(1, 1);
        depthImg.flagImageChanged();
        
        if(bThreshWithOpenCV) {
            // copy into threshold image
			grayThreshFar = depthImg;
            // thresholded image
			grayThreshFar.threshold(farThreshold);
            
            // subtract the far threshold value to normalize the depth image
            depthImg.operator -= (farThreshold-1);
            // mask the depth image with the treshold image
			cvAnd(grayThreshFar.getCvImage(), depthImg.getCvImage(), depthImg.getCvImage(), NULL);
            
            unsigned char * depthPixels = depthImg.getPixels();
            unsigned char * depthAlphaPixels = depthImageAlpha.getPixels();
            
            unsigned char * colorPixels = colorImg.getPixels();
            unsigned char * colorAlphaPixels = colorImageAlpha.getPixels();
            
            const int scaleFactor = 25;
            for (int i = 0; i < depthImg.width * depthImg.height; i++) { // go over the array and scale the depth image values
                
                int height = depthPixels[i] * scaleFactor;
                depthPixels[i] = (unsigned char)CLAMP(height, 0, 255);
                
                int indexRGB = i*3; // rgb images store 3 color values
                int indexRGBA = i*4; // rgba images store 4 values
                
                depthAlphaPixels[indexRGBA] = depthPixels[i];
                depthAlphaPixels[indexRGBA+1] = depthPixels[i];
                depthAlphaPixels[indexRGBA+2] = depthPixels[i];
                depthAlphaPixels[indexRGBA+3] = (depthPixels[i] == 0) ? 0 : 255; // define the alpha mask: everything over depth 0 is shown
                
                colorAlphaPixels[indexRGBA] = (unsigned char)(colorPixels[indexRGB] * 1.0);
                colorAlphaPixels[indexRGBA+1] = (unsigned char)(colorPixels[indexRGB+1] * 1.0);
                colorAlphaPixels[indexRGBA+2] = (unsigned char)(colorPixels[indexRGB+2] * 1.0);
                colorAlphaPixels[indexRGBA+3] = (depthPixels[i] == 0) ? 0 : 255; // define the alpha mask: everything over 0 is shown
            }
            depthImg.flagImageChanged();
            depthImageAlpha.update();
            colorImageAlpha.update();
            
            findHands();
		}
    }
}

void KinectTracker::cropArms() {
    handCropMask.begin();
    ofClear(0);
    ofClearAlpha();
    ofTranslate(roiX, roiY);
    ofSetColor(255);
    ofFill();
    
    const int handWidth = 100;
    const int handHeight = 120;
    
    for(vector<KinectHand>::iterator itr = hands.begin();itr < hands.end();itr++) {
        ofPushMatrix();
            ofTranslate(itr->handTip);
            ofRotate(itr->handAngle + 90, 0, 0, 1);
            ofRect(-handWidth/2, 0, handWidth, handHeight);
        ofPopMatrix();
    }
    handCropMask.end();
    
    ofPixels handMaskPixels;
    handCropMask.readToPixels(handMaskPixels);
    unsigned char * depthAlphaPixels = depthImageAlpha.getPixels();
    
    for (int i = 0; i < depthImageAlpha.width * depthImageAlpha.height; i++) { // go over the array and scale the depth image values
        int indexRGBA = i*4; // rgba images store 4 values
        depthAlphaPixels[indexRGBA+3] = (handMaskPixels.getPixels()[indexRGBA] == 0) ? 0 : 255;
    }
    depthImageAlpha.update();
}

void KinectTracker::findHands() {
    
    // copy the kinect threshold region of interest into a smaller image
    grayThreshFar.setROI(96, 96, 288, 288);
    depthImgThresCropped.setFromPixels(grayThreshFar.getRoiPixelsRef());
    depthImgThresCropped.dilate_3x3();
    depthImgThresCropped.erode_3x3();
    depthImgThresCropped.erode_3x3();
    depthImgThresCropped.dilate_3x3();
    depthImgThresCropped.flagImageChanged();
    
    const int minContourSize = 75;
    //find the contours in that image
    finger_contourFinder.findContours(depthImgThresCropped, minContourSize, (depthImgThresCropped.width * depthImgThresCropped.height)/2, 20, 20.0, false);

    grayThreshFar.resetROI();
    
    // delete all the contours we found in the last frame
    hands.clear();
    
    // copy over all contours that touch the boundaries of the image to the hands vector
    for(vector<Blob>::iterator itr = finger_contourFinder.blobs.begin();itr < finger_contourFinder.blobs.end();itr++) {
        
        KinectHand newHand;
        bool isHand = newHand.createHandFromBlob(*itr, depthImgThresCropped.width, depthImgThresCropped.height);
        
        if(isHand){
           hands.push_back(newHand);
        }
    }
}


void KinectTracker::drawDebugImage(float x, float y, float width, float height) {
    ofPushMatrix();
        ofTranslate(x, y);
        ofScale(width/roiW, height/roiH);
    
        // draw the cropped threshold image
        depthImgThresCropped.draw(0, 0);

        // draw all the contours
        for(vector<Blob>::iterator itr = finger_contourFinder.blobs.begin();itr < finger_contourFinder.blobs.end();itr++) {
            itr->drawContours();
        }
    
        // draw all the detected hands
        for(vector<KinectHand>::iterator itr = hands.begin();itr < hands.end();itr++) {
            itr->drawDebug();
        }
    ofPopMatrix();
}

//--------------------------------------------------------------
// draws the uncropped depth image (640x480), scaled to width and height at location x, y
//--------------------------------------------------------------
void KinectTracker::drawDepth(int x, int y, int width, int height) {
    
    if(!renderDepth) return false;
    
    ofEnableAlphaBlending();
    
    if(currentHandVisualization > 1) {
        ofPushMatrix();
            ofTranslate(x, y);
            ofScale((float)width/640, (float)height/480);
            ofTranslate(roiX, roiY);
            ofSetColor(255);
            ofFill();
            
            for(vector<KinectHand>::iterator itr = hands.begin();itr < hands.end();itr++) {
                ofPushMatrix();
                ofTranslate(itr->handTip);
                ofRotate(itr->handAngle + 90, 0, 0, 1);
                replacementHandsDepth[currentHandVisualization - 2].draw(-replacementHandsDepth[currentHandVisualization - 2].width/2,0);
                ofPopMatrix();
            }
        ofPopMatrix();
    }
    else {
        
        if(currentHandVisualization == 1) {
            cropArms();
        }
        
        ofPushMatrix();
            ofTranslate(x, y);
        
            ofSetColor(255, 255, 255);
            // draw snapshots, in case we took any
            for(vector<ofImage>::iterator itr = depthImageSnapshots.begin();itr < depthImageSnapshots.end();itr++) {
                itr->draw(0,0,width, height);
            }
            // draw current depth image
            depthImageAlpha.draw(0,0,width,height);
            //handCropMask.draw(0,0,width,height);
        ofPopMatrix();
    }
    
    ofDisableAlphaBlending();
}

//--------------------------------------------------------------
// draws the uncropped color image (640x480), scaled to width and height at location x, y
//--------------------------------------------------------------
void KinectTracker::drawGraphics(int x, int y, int width, int height) {

    ofEnableAlphaBlending();
    
    
    if(currentHandVisualization > 1) {
        ofPushMatrix();
            ofTranslate(x, y);
            ofScale((float)width/640, (float)height/480);
            ofTranslate(roiX, roiY);
            ofSetColor(255);
            ofFill();
            
            
            for(vector<KinectHand>::iterator itr = hands.begin();itr < hands.end();itr++) {
                ofPushMatrix();
                ofTranslate(itr->handTip);
                ofRotate(itr->handAngle + 90, 0, 0, 1);
                replacementHandsColor[currentHandVisualization - 2].draw(-replacementHandsColor[currentHandVisualization - 2].width/2,0);
                ofPopMatrix();
            }
        ofPopMatrix();
    }
    else {
        ofPushMatrix();
            ofTranslate(x, y);
            
            ofSetColor(255, 255, 255);
            // draw snapshots, in case we took any
            for(vector<ofImage>::iterator itr = colorImageSnapshots.begin();itr < colorImageSnapshots.end();itr++) {
                itr->draw(0,0,width, height);
            }
            // draw current color image
            colorImageAlpha.draw(0,0,width,height);
        
        ofPopMatrix();
    }
    
    ofDisableAlphaBlending();
    
}


void KinectTracker::addSnapshot() {
    depthImageSnapshots.push_back(depthImageAlpha);
    colorImageSnapshots.push_back(colorImageAlpha);
}

void KinectTracker::removeSnapshot() {
    if(depthImageSnapshots.size() > 0) depthImageSnapshots.pop_back();
    if(colorImageSnapshots.size() > 0) colorImageSnapshots.pop_back();
}