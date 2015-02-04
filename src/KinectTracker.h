//
//  KinectTracker.h
//  Relief2
//
//  Created by Sean Follmer on 3/24/13.
//
//

#include "KinectHand.h"


#ifndef __Relief2__KinectTracker__
#define __Relief2__KinectTracker__

#include <iostream>

class KinectTracker {
public:
    ofxKinect kinect;
    
    void setup();
    void exit();
    // draws the uncropped depth image (640x480), scaled to width and height at location x, y
    void drawDepth(int x, int y, int width, int height);
    // draws the uncropped color image (640x480), scaled to width and height at location x, y
    void drawGraphics(int x, int y, int width, int height);
    // crops the arms from the depth image
    void cropArms();
    
    // draws the cropped debug image
    void drawDebugImage(float x, float y, float width, float height);
    
    void addSnapshot();
    void removeSnapshot();
    
    void setROI(int x, int y, int width, int height);
    
    void update();
    
    void findHands();
    
    vector<ofPoint>  fingers; //z is relative above height map
    vector<ofPoint>  absFingers; //z is absolute (not vodka)
    
    const int roiX = 96;
    const int roiY = 96;
    const int roiW = 288;
    const int roiH = 288;

    ofxCvColorImage colorImg, rectifiedColorImg;
    ofxCvGrayscaleImage depthImg;
    ofImage depthImageAlpha;
    ofImage colorImageAlpha;
    ofFbo handCropMask; // this mask is used for when we want to crop the hands, to eliminate the arms in the output
    
    vector<ofImage> depthImageSnapshots;
    vector<ofImage> colorImageSnapshots;
    
	ofxCvGrayscaleImage grayThreshFar;
    ofxCvGrayscaleImage depthImgThresCropped;
	
	int nearThreshold;
	int farThreshold;
	bool bThreshWithOpenCV;
    
    
    ofTrueTypeFont verdana;
    
    ContourFinder finger_contourFinder;
    vector<KinectHand> hands;
    
    ofImage replacementHandsColor[7];
    ofImage replacementHandsDepth[7];
    
    bool renderDepth = true;
    int currentHandVisualization = 0;
    
    void switchToNextVisualization() {currentHandVisualization ++; currentHandVisualization = currentHandVisualization % 9;};
    void switchToPreviousVisualization() {currentHandVisualization --; if(currentHandVisualization <= 0) currentHandVisualization =0;};
    void switchVisualization(int visualization) {
        if (currentHandVisualization != 2)
            currentHandVisualization = 2;
        else
            currentHandVisualization = 3;
    };
    void toggleDepthRendering() { renderDepth = !renderDepth;};

};

#endif /* defined(__Relief2__KinectTracker__) */
