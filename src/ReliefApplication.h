#pragma once

#include "ofMain.h"
#include "ofxSimpleGuiToo.h"
#include "ReliefIOManager.h"
#include "RenderableObject.h"
#include "RWell.h"
#include "RRectangle.h"
#include "RBitmap.h"
#include "KinectTracker.h"
#include "RRipple.h"
#include <memory>
#include "R3DModel.h"
#include "ofVideoGrabber.h"

//#define RECTDRAW_SIZE 10
//#define DRAW_OFFSET 300
//#define MID_PIN_HEIGHT 127

class ReliefApplication : public ofBaseApp {
public:
    void setup();
    void update();
    void draw();
    void exit();
    
    void keyPressed(int key);
    void keyReleased(int key);
    void mouseMoved(int x, int y);
    void mouseDragged(int x, int y, int button);
    void mousePressed(int x, int y, int button);
    void mouseReleased(int x, int y, int button);
    void windowResized(int w, int h);
    //void dragEvent(ofDragInfo dragInfo);
    void gotMessage(ofMessage msg);
    
    void renderGraphicsFBO();
    void renderHeightMapFBO();
    void renderRemoteDisplayFBO();
    void sendHeightToRelief();
    
    ReliefIOManager * mIOManager;
	unsigned char mPinHeightToRelief [RELIEF_SIZE_X][RELIEF_SIZE_Y];
    
    ofFbo pinDisplayImage; //FBO where we render graphics for pins
    ofFbo pinHeightMapImage; //FBO where we render height map
    ofFbo pinHeightMapImageSmall; //FBO where we render height map
    
    ofFbo remoteDisplayFBO; //FBO where we render the image for the wacom cintiq display
    
    int projectorOffsetX;
    
    vector< RenderableObject *> renderableObjects;
    
    RRipple * myRipple;
    
    RenderableObject * myCurrentRenderedObject;
    
    KinectTracker kinectTracker;
        
    ofx3DModelLoader model;
    ofShader mHeightMapShader;
    
    ofVideoGrabber cam;
    ofImage camImage;
    
    bool show3DModel;
    
    float inputCanvasRotation = 0;
    float inputCanvasScale = 1;
    int mirrorMode = 1;
};
