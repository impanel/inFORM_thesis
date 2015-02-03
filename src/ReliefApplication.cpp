#include "ReliefApplication.h"

//--------------------------------------------------------------
void ReliefApplication::setup(){
    if (USE_KINECT) {
        kinectTracker.setup();
    }
    
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
    
    
    myRipple = new RRipple();
    myRipple->setPosition(ofPoint(450,450));
    myRipple->setSize(ofPoint(650,650));
    myRipple->render = true;
    
    
    bool loadPlane = false;
    
    if (loadPlane) {
        model.loadModel("plane/plane.3ds", 18);
        model.setRotation(0, -5, 1, 0, 0); // plane
        model.setRotation(1, 29, 0, 0, 1); // plane
        model.setPosition(435, 420, -400); // plane
    }
    else {
        model.loadModel("VWBeetle/VWBeetle.3ds", 0.099);
        model.setScale(1,1,1);
        model.setPosition(470, 520, -250);
        model.setRotation(1, 230, 0, 0, 1);
    }
    
    //model.loadModel("plane/plane.3ds", 18); // plant
    
    mHeightMapShader.load("shaders/heightMapShader");
    
    show3DModel = false;
    
    
    cam.setDeviceID(2);

    cam.initGrabber(CAMERA_RESOLUTION_X, CAMERA_RESOLUTION_Y);
    
    camImage.allocate(CAMERA_RESOLUTION_X,CAMERA_RESOLUTION_Y, OF_IMAGE_COLOR);
    
    mirrorMode = 1;
    
    
}

//--------------------------------------------------------------
void ReliefApplication::update(){

    
    if (USE_KINECT) {
        kinectTracker.update();
    }
    
    cam.update();
    if(cam.isFrameNew()) {
        camImage.setFromPixels(cam.getPixelsRef());
    }
    
    //app timebase, to send to all animatable objets
	float dt = 1.0f / ofGetFrameRate();
    
    // update rendered objects
    // myCurrentRenderedObject->update(dt);
    //for(int i=0; i < renderableObjects.size(); i++){
    //    renderableObjects[i]->update(dt);
    //}
    
    renderGraphicsFBO();
    
    renderHeightMapFBO();
    
    renderRemoteDisplayFBO();
  
    sendHeightToRelief();
}


void ReliefApplication::renderGraphicsFBO() {
    
    // render graphics
    pinDisplayImage.begin();
    // set up projection
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glViewport(0, 0, 900, 900);
    glOrtho(0.0, 900, 0, 900, -500, 500);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glPushMatrix();
    glTranslated(0, 0, -500);
    glPopMatrix();
    
    ofBackground(0);
    
    // draw 3d model
    glEnable(GL_DEPTH_TEST);
    if(show3DModel) model.draw();
    glDisable(GL_DEPTH_TEST);
    
    ///kinectTracker.drawColorImage(-300, -300, 2000, 1500);
    //ofPushMatrix();         // push the current coordinate position
    //ofRotateZ(20);
    //kinectTracker.drawColorImage(-300, -300, 2000, 1500);
    //ofPopMatrix();
    
    // draw the kinect image
    ofPushMatrix();
    ofTranslate(450, 450);
    ofScale(mirrorMode*inputCanvasScale, inputCanvasScale);
    ofRotateZ(inputCanvasRotation);
    kinectTracker.drawGraphics(-750, -750, 2000, 1500);
    ofPopMatrix();
    
    
    pinDisplayImage.end();
}

void ReliefApplication::renderHeightMapFBO() {
    // ---- render large heightmap
    pinHeightMapImage.begin();
    
        // set up the projection
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glViewport(0, 0, 900, 900);
        glOrtho(0.0, 900, 0, 900, -500, 500);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        glPushMatrix();
        glTranslated(0, 0, -500);
        glPopMatrix();
        
        ofBackground(0);
        
        // draw the 3D model
        mHeightMapShader.begin();
        glEnable(GL_DEPTH_TEST);
        if(show3DModel) model.draw();
        glDisable(GL_DEPTH_TEST);
        mHeightMapShader.end();
        
        // draw the kinect image
        ofPushMatrix();
            ofTranslate(450, 450);
            ofScale(mirrorMode*inputCanvasScale, inputCanvasScale);
            ofRotateZ(inputCanvasRotation);
            kinectTracker.drawDepth(-750, -750, 2000, 1500);
        ofPopMatrix();
    
    pinHeightMapImage.end();
    
    // render small heightmap
    pinHeightMapImageSmall.begin();
        ofBackground(0);
        ofSetColor(255);
        pinHeightMapImage.draw(0, 0, RELIEF_SIZE_X, RELIEF_SIZE_Y);
    pinHeightMapImageSmall.end();
}

void ReliefApplication::renderRemoteDisplayFBO() {
    
    remoteDisplayFBO.begin();
    // set up the projection
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glViewport(0, 0, CINTIQ_RESOLUTION_X, CINTIQ_RESOLUTION_Y);
    glOrtho(0.0, CINTIQ_RESOLUTION_X, 0, CINTIQ_RESOLUTION_Y, -500, 500);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    //glPushMatrix();
    //glTranslated(0, 0, -500);
    //glPopMatrix();
    
    ofBackground(0,0,0);
    
    ofPushMatrix();
    const float subX = 502;
    const float subY = 166;
    const float subWidth = 833-subX;
    const float subHeight = 507-subY;

        ofTranslate(CINTIQ_RESOLUTION_X/2, CINTIQ_RESOLUTION_Y/2);
        ofRotateZ(-inputCanvasRotation);
        ofScale(1.0f/inputCanvasScale, 1.0f/inputCanvasScale);
        ofScale((CINTIQ_RESOLUTION_X/subWidth) * (15.0f/17.0f), (CINTIQ_RESOLUTION_Y/subHeight) * (15.0f/13.0f));
        ofTranslate(-subWidth/2, -subHeight/2);
        camImage.drawSubsection(0, 0, subWidth, subHeight, subX, subY, subWidth, subHeight);

    ofPopMatrix();
    
    remoteDisplayFBO.end();
}

//--------------------------------------------------------------
void ReliefApplication::draw(){
    
    // draw debug graphics
    ofBackground(0,0,0);
    ofSetColor(255);
    ofRect(1, 1, 502, 502);
    pinDisplayImage.draw(2, 2, 500, 500);
    
    ofSetColor(255);
    ofRect(505, 1, 502, 502);
    pinHeightMapImage.draw(506, 2, 500, 500);
    
    
    ofSetColor(255);
    ofRect(1009, 1, 502, 502);
    pinHeightMapImageSmall.draw(1010, 2, 500, 500);

    ofSetColor(255);
    ofRect(1, 506, 502, 502);
    kinectTracker.drawDebugImage(2, 507, 500, 500);
    
    //Draw Graphics onto projector
    ofSetColor(255);
    pinDisplayImage.draw(projectorOffsetX, RELIEF_PROJECTOR_OFFSET_Y, RELIEF_PROJECTOR_SCALED_SIZE_X, RELIEF_PROJECTOR_SCALED_SIZE_Y);
    

    remoteDisplayFBO.draw(SEAN_SCREEN_RESOLUTION_X + PROJECTOR_RAW_RESOLUTION_X, 0);
    
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
    if (key == 'a') {
        kinectTracker.addSnapshot();
    }
    if (key == 's') {
        kinectTracker.removeSnapshot();
    }
    if (key == '1') {
        kinectTracker.switchToNextVisualization();
    }
    if (key == '2') {
        kinectTracker.switchToPreviousVisualization();
    }
    if (key == '3') {
        kinectTracker.toggleDepthRendering();
    }
    if (key == 'm'){
        mirrorMode= -1* mirrorMode;
    }
    if (key == 'b'){
        show3DModel= !show3DModel;
    }
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
    if (USE_KINECT) {
        kinectTracker.exit();
    }
    mIOManager->sendValueToAllBoards(TERM_ID_MAXSPEED, (unsigned char) 0);
    ofSleepMillis(1000);
}


