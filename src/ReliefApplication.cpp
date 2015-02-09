#include "ReliefApplication.h"

//--------------------------------------------------------------
void ReliefApplication::setup(){
    
    ofSetLogLevel(OF_LOG_WARNING);
    
    if(SEAN_SCREEN){
        //For Sean Screen
        ofSetWindowShape(SEAN_SCREEN_RESOLUTION_X+PROJECTOR_RAW_RESOLUTION_X+1600,1200);
        projectorOffsetX=SEAN_SCREEN_RESOLUTION_X+RELIEF_PROJECTOR_OFFSET_X;
        if(CINEMA_DISPLAY){
            ofSetWindowPosition(CINEMA_DISPLAY_OFFSET, 0);
        }
        else{
            ofSetWindowPosition(0, 0);
            
        }
    }
    else{
    //For Daniel Screen
        ofSetWindowShape(DANIEL_SCREEN_RESOLUTION_X+PROJECTOR_RAW_RESOLUTION_X,PROJECTOR_RAW_RESOLUTION_Y);
        projectorOffsetX=DANIEL_SCREEN_RESOLUTION_X+RELIEF_PROJECTOR_OFFSET_X;
        ofSetWindowPosition(0, 0);
    }
    
    ofSetFrameRate(30);
    
	// initialize communication with Relief table
	mIOManager = new ReliefIOManager();
    
    float gain_P = 1.3;
    float gain_I = 0.2;
    int max_I = 60;
    int deadZone = 30;
    int gravityComp = 0;
    int maxSpeed = 220;
    
    mIOManager->sendValueToAllBoards(TERM_ID_GAIN_P, (unsigned char) (gain_P * 25));
    mIOManager->sendValueToAllBoards(TERM_ID_GAIN_I, (unsigned char) (gain_I * 100));
    mIOManager->sendValueToAllBoards(TERM_ID_MAX_I, (unsigned char)max_I);
    mIOManager->sendValueToAllBoards(TERM_ID_DEADZONE, (unsigned char)deadZone);
    mIOManager->sendValueToAllBoards(TERM_ID_GRAVITYCOMP, (unsigned char)gravityComp);
    mIOManager->sendValueToAllBoards(TERM_ID_MAXSPEED, (unsigned char)(maxSpeed/2));
    
    pinDisplayImage.allocate(RELIEF_PROJECTOR_SIZE_X, RELIEF_PROJECTOR_SIZE_X, GL_RGBA);
    pinHeightMapImage.allocate(RELIEF_PROJECTOR_SIZE_X, RELIEF_PROJECTOR_SIZE_X, GL_RGBA);
    pinHeightMapImageSmall.allocate(RELIEF_SIZE_X, RELIEF_SIZE_Y, GL_RGBA);
    remoteDisplayFBO.allocate(CINTIQ_RESOLUTION_X, CINTIQ_RESOLUTION_Y, GL_RGBA);
    
    
    mHeightMapShader.load("shaders/heightMapShader");
   
    
    cam.setDeviceID(2);

    cam.initGrabber(CAMERA_RESOLUTION_X, CAMERA_RESOLUTION_Y);
    
    camImage.allocate(CAMERA_RESOLUTION_X,CAMERA_RESOLUTION_Y, OF_IMAGE_COLOR);
    
    mirrorMode = 1;
    
    tcp.setup();
    
}

//--------------------------------------------------------------
void ReliefApplication::update(){
    
    //app timebase, to send to all animatable objets
	float dt = 1.0f / ofGetFrameRate();
    
    tcp.update(dt);
    
    renderGraphicsFBO();
    
    renderHeightMapFBO();
    
    renderRemoteDisplayFBO();
  
    sendHeightToRelief();
    
    
}

//--------------------------------------------------------------

void ReliefApplication::renderGraphicsFBO() {
    
    //render graphics
    pinDisplayImage.begin();
    
    ofClear(255,255,255, 0);
    ofRect(0, 0, RELIEF_PROJECTOR_SIZE_X, RELIEF_PROJECTOR_SIZE_X);
    
    pinDisplayImage.end();
}

//--------------------------------------------------------------

void ReliefApplication::renderHeightMapFBO() {
    // ---- render large heightmap
    pinHeightMapImage.begin();
    
    ofClear(255,255,255, 0);
    tcp.getPinHeightImage().draw(0, 0, RELIEF_PROJECTOR_SIZE_X, RELIEF_PROJECTOR_SIZE_X);
    
    pinHeightMapImage.end();
    
    // render small heightmap
    pinHeightMapImageSmall = tcp.getPinHeightImage();
}

//--------------------------------------------------------------

void ReliefApplication::renderRemoteDisplayFBO() {
    
}

//--------------------------------------------------------------
void ReliefApplication::draw(){
    
    // draw debug graphics
    ofBackground(128,128,128);
    ofSetColor(255);
    ofRect(1, 1, 502, 502);
    pinDisplayImage.draw(2, 2, 500, 500);
    
    ofSetColor(255);
    ofRect(505, 1, 502, 502);
    pinHeightMapImage.draw(506, 2, 500, 500);
    
    ofSetColor(255);
    ofRect(1009, 1, 502, 502);
    pinHeightMapImageSmall.draw(1010, 2, 500, 500);
    
    //Draw Graphics onto projector
    ofSetColor(255);
    pinDisplayImage.draw(projectorOffsetX, RELIEF_PROJECTOR_OFFSET_Y, RELIEF_PROJECTOR_SCALED_SIZE_X, RELIEF_PROJECTOR_SCALED_SIZE_Y);
    
    tcp.drawDebug(2, 506);
}

//--------------------------------------------------------------
void ReliefApplication::keyPressed(int key){
    //myCurrentRenderedObject->keyPressed(key);
    
    if(key == '0') {
        inputCanvasRotation += 3;
    }
    
    if(key == '-') {
        inputCanvasScale -= 0.05;
    }
    if(key == '=') {
        inputCanvasScale += 0.05;
    }
    
    if(key == 'r') {
        inputCanvasScale = 1;
        inputCanvasRotation = 0;
    }
    
    tcp.keyPressed(key);
}

//--------------------------------------------------------------
void ReliefApplication::keyReleased(int key){
    
}

//--------------------------------------------------------------
void ReliefApplication::mouseMoved(int x, int y){
    
}

//--------------------------------------------------------------
void ReliefApplication::mouseDragged(int x, int y, int button){
    
    
}

//--------------------------------------------------------------
void ReliefApplication::mousePressed(int x, int y, int button){
    //test->position.animateTo(ofPoint((x - 2)*3, (y - 2)*3, 50));
    //printf("%f, %f, %f \n",renderableObjects[0].position.getCurrentPosition().x, renderableObjects[0].position.getCurrentPosition().y, renderableObjects[0].position.getCurrentPosition().z);

    //renderableObjects[i].drawGraphics();
}

//--------------------------------------------------------------
void ReliefApplication::mouseReleased(int x, int y, int button){
    cout<< x <<" " << y << endl;
}

//--------------------------------------------------------------
void ReliefApplication::windowResized(int w, int h){
    
}

//--------------------------------------------------------------
void ReliefApplication::gotMessage(ofMessage msg){
    
}

//-----------------------------------------------------------
void ReliefApplication::sendHeightToRelief(){
    
    unsigned char* pixels;
    ofPixels tempPixels;
    pinHeightMapImageSmall.readToPixels(tempPixels);
    //Rotate to align all coordinate systems
    tempPixels.rotate90(1);
    tempPixels.mirror(true, false);
    pixels = tempPixels.getPixels();
    
    for (int j = 0; j < RELIEF_SIZE_Y; j ++) {
        for (int i = 0; i < RELIEF_SIZE_X; i ++) {
            int y = j;
            int x = RELIEF_SIZE_X - 1 - i;
            int heightMapValue = pixels[((x + (y * RELIEF_SIZE_X)) * 4)];
            mPinHeightToRelief[i][j] = heightMapValue;
        }
    }
    
    // send the height map to the hardware interface
	mIOManager->sendPinHeightToRelief(mPinHeightToRelief);
}

//------------------------------------------------------------
void ReliefApplication::exit(){
    mIOManager->sendValueToAllBoards(TERM_ID_MAXSPEED, (unsigned char) 0);
    ofSleepMillis(1000);
}


