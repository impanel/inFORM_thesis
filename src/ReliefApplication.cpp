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

    #ifdef DRAW_SCREEN_HALF_SIZE
        gridOffset0 /= 2;
        gridOffset1 /= 2;
        gridOffset2 /= 2;
        gridSize /= 2;
        simulationZoom /= 2;
    #endif

    ofSetFrameRate(30);
    
	// initialize communication with Relief table
	mIOManager = new ReliefIOManager();
    
    // table simulator
    tableSimulation = new TableSimulator(mIOManager);
    
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
    
    movie.setup("videos");
    setupGuiMenu();
    
    // movie exporter
    movieExporter.setRecordingArea(0, 0, 100, 100);
    movieExporter.setup(100, 100); //set video size to 30x30
    //movieExporter.setPixelSource(tcp.getPixels(), 30 , 30);

    bUseVideo = false;
    
}

//--------------------------------------------------------------

void ReliefApplication::setupGuiMenu()
{
    guiMenu = new ofxUICanvas;
    guiMenu->setPosition(250, 20);
    guiMenu->setName("GUI Menu");

    //Toggle for Table Simulation
    guiMenu->addToggle("show simulation", &drawTableSimulation);
    
    //Dropdown Menu for Videos
    ofxUIDropDownList *ddl = guiMenu->addDropDownList("video list", movie.getLoadedVideoFilenames());
    ddl->setAllowMultiple(false);
    ddl->setAutoClose(true);
    
    ofAddListener(guiMenu->newGUIEvent, this, &ReliefApplication::guiEvent);
    guiMenu->autoSizeToFitWidgets();
}

//--------------------------------------------------------------

void ReliefApplication::update(){
    
    //app timebase, to send to all animatable objets
	float dt = 1.0f / ofGetFrameRate();
    
    if(bUseVideo)
        movie.update(dt);
    else
        tcp.update(dt);

    renderGraphicsFBO();
    
    renderHeightMapFBO();
    
    renderRemoteDisplayFBO();
  
    sendHeightToRelief();
    
    // update the table simulation, which is the rendered table graphic
    tableSimulation->update();
}

//--------------------------------------------------------------

void ReliefApplication::renderGraphicsFBO() {
    
    //render graphics
    pinDisplayImage.begin();
    
    ofClear(255,255,255, 0);
    ofRect(0, 0, RELIEF_PROJECTOR_SIZE_X, RELIEF_PROJECTOR_SIZE_Y);
    
    pinDisplayImage.end();
}

//--------------------------------------------------------------

void ReliefApplication::renderHeightMapFBO() {
    // ---- render large heightmap
    pinHeightMapImage.begin();
    
    ofClear(255,255,255, 0);
    
    if (bUseVideo)
        movie.getPinHeightImage().draw(0, 0, RELIEF_PROJECTOR_SIZE_X, RELIEF_PROJECTOR_SIZE_X);
        //movie.drawHeightMap();
    else
        tcp.getPinHeightImage().draw(0, 0, RELIEF_PROJECTOR_SIZE_X, RELIEF_PROJECTOR_SIZE_X);
    
    pinHeightMapImage.end();
    
    // render small heightmap
    if (bUseVideo)
        pinHeightMapImageSmall = movie.getPinHeightImage();
    else
        pinHeightMapImageSmall = tcp.getPinHeightImage();
}

//--------------------------------------------------------------

void ReliefApplication::renderRemoteDisplayFBO() {
    
}

//--------------------------------------------------------------
void ReliefApplication::draw(){
    ofBackground(128, 128, 128);
    ofSetColor(255);
    ofRect(gridOffset0 - 1, gridOffset0 - 1, gridSize + 2, gridSize + 2);
    pinDisplayImage.draw(gridOffset0, gridOffset0, gridSize, gridSize);
    
    ofSetColor(255);
    ofRect(gridOffset1 - 1, gridOffset0 - 1, gridSize + 2, gridSize + 2);
    pinHeightMapImage.draw(gridOffset1, gridOffset0, gridSize, gridSize);
    
    ofSetColor(255);
    ofRect(gridOffset2 - 1, gridOffset0 - 1, gridSize + 2, gridSize + 2);
    pinHeightMapImageSmall.draw(gridOffset2, gridOffset0, gridSize, gridSize);
    
    //Draw Graphics onto projector
    ofSetColor(255);
    pinDisplayImage.draw(projectorOffsetX, RELIEF_PROJECTOR_OFFSET_Y, RELIEF_PROJECTOR_SCALED_SIZE_X, RELIEF_PROJECTOR_SCALED_SIZE_Y);
    
    // draw simulation in all views if we want
    // be careful as this slows performance
    if(drawTableSimulation) {
        tableSimulation->drawTableCamView(gridOffset2 - 1, gridOffset1, gridSize + 2, gridSize + 2, simulationZoom);
        tableSimulation->drawInteractionArea(gridOffset2, gridOffset1, gridSize, gridSize);
    }
    
    if(bEnableMovieRecorder)
    {
        tcp.drawDebug(0, 0, 100, 100);
    }
    
    // draw debug graphics
    if(bUseVideo)
    {
        movie.drawDebug(gridOffset0, gridOffset1);
        string videoInfo = "loop video [l]: ";
        ofDrawBitmapString(videoInfo + ofToString(movie.getLoopState()), 10, ofGetHeight() - 50);
        
        ofPushStyle();
        ofSetColor(255);
        string msg = "[2] video playback/assembly mode";
        msg += "\n[l] toggle looping video";
        msg += "\n[b] set the playhead to the beginning of the video";
        msg += "\n[space] toggle pause video";
        msg += "\n[DOWN] next video";
        msg += "\n[UP] previous video";
        msg += "\n[RIGHT] next frame";
        msg += "\n[LEFT] previous frame";
        ofDrawBitmapStringHighlight(msg, 510, ofGetWindowHeight() - 200);
        ofPopStyle();
    }
    else
    {
        tcp.drawDebug(gridOffset0, gridOffset1, gridSize, gridSize);
        
        string videoExportInfo = "video export [e]: ";
        ofDrawBitmapString(videoExportInfo + ofToString(bEnableMovieRecorder), 10, ofGetHeight() - 50);
        
        string recordingInfo = "is video being recorded: ";
        ofDrawBitmapString(recordingInfo + ofToString(movieExporter.isRecording()), 10, ofGetHeight() - 70);
        
        ofPushStyle();
        ofSetColor(255);
        string msg = "[1] TCP mode";
        msg += "\n[e] toggle export video flag";
        msg += "\n[l] toggle looping video";
        msg += "\n[s] stop and reset video";
        msg += "\n[c] clear frame buffer array";
        msg += "\n[d] delete current frame from frame buffer array";
        msg += "\n[RIGHT] next frame";
        msg += "\n[LEFT] previous frame";
        ofDrawBitmapStringHighlight(msg, 510, ofGetWindowHeight() - 200);
        ofPopStyle();
    }
    
    ofPushStyle();
    ofSetColor(255);
    string msg = "Press [1] for TCP mode or [2] for Video/Assembly Mode";
    ofDrawBitmapStringHighlight(msg, 510, ofGetWindowHeight() - 300);
    ofPopStyle();
    
}

//--------------------------------------------------------------
void ReliefApplication::keyPressed(int key){
    //myCurrentRenderedObject->keyPressed(key);
    
    //Activate TCP
    if(key == '1') {
        bUseVideo = false;
    }
    
    //Activate Video
    if(key == '2') {
        bUseVideo = true;
    }
    
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
    
    if(!bUseVideo)
        if (key == 'e') {
            bEnableMovieRecorder = !bEnableMovieRecorder;
        }
    
    if (bUseVideo)
    {
        movie.keyPressed(key);
    }
    else
    {
        tcp.keyPressed(key);
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

//------------------------------------------------------------
void ReliefApplication::guiEvent(ofxUIEventArgs &e)
{
    //cout << "Event fired: " << e.getName() << endl;
    if(e.getName() == "video list")
    {
        ofxUIDropDownList *ddlist = (ofxUIDropDownList *) e.widget;
        vector<ofxUIWidget *> &selected = ddlist->getSelected();
        //vector<string> playList;
        for(int i = 0; i < selected.size(); i++)
        {
            cout << "SELECTED VIDEO: " << selected[i]->getName() << endl;
            //playList.push_back(selected[i]->getName());
            movie.playByFilename(selected[i]->getName());
            movie.reset();
            movie.setLooping(true);
            movie.resume();
            cout<<"play"<<endl;
        }
        //movie.setPlayList(playList);
    }
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
    tcp.exit();
    mIOManager->sendValueToAllBoards(TERM_ID_MAXSPEED, (unsigned char) 0);
    ofSleepMillis(1000);
}


