//
//  RTCP.cpp
//  inform
//
//  Created by Philipp Schoessler on 2/5/15.
//
//

#include "RTCP.h"

RTCP::RTCP(){};

//--------------------------------------------------------------

void RTCP::setup(){
    
    size = RELIEF_SIZE_X * RELIEF_SIZE_Y;
    pixels = new unsigned char[size];
    
    //setup the server to listen on 11999
    TCP.setup(11996);
    TCP.setMessageDelimiter("\n");
    
    //fill the vector<>
    for(int i = 0; i < size; i++)
    {
        rects.push_back(ofRectangle());
        rects.at(i).setHeight(1);
        rects.at(i).setWidth(1);
    }
    
    bStop = true;
    bPause = false;
    sequenceFPS = 30;
    
    pinHeightImage.allocate(RELIEF_SIZE_X, RELIEF_SIZE_Y, GL_RGB);
}

//--------------------------------------------------------------

void RTCP::update(float dt){
    
    
    //get data from host (3dsmax)
    
    //for each connected client lets get the data being sent and lets print it to the screen
    for(unsigned int i = 0; i < (unsigned int)TCP.getLastID(); i++)
    {
        if(!TCP.isClientConnected(i) )continue;
        
        //we only want to update the text we have recieved there is data
        string str = TCP.receive(i);
        
        if(str.length() > 0)
        {
            storeText.push_back(str);
            unsigned char * receivedChars = new unsigned char[size];
            
            for(int i = 0; i < size; i++)
                receivedChars[i] = 255  - str[i]; //invert values here because it doesn't work in maxscript
            pixels = receivedChars;
        }
    }
    
    
    //draw info about TCP
    if(TCP.isClientConnected(0))
    {
        //calculate where to draw the text
        int xPos = 506;
        int yPos = 506;
        
        //get the ip and port of the client
        string port = ofToString( TCP.getClientPort(0) );
        string ip   = TCP.getClientIP(0);
        string info = "client "+ofToString(0)+" -connected from "+ip+" on port: "+port;
        string storeInfo = "stored frames: " + ofToString(storeText.size()) + " fIdx: " + ofToString(frameIndex);
        
        //draw the info text and the received text bellow it
        ofDrawBitmapString(info, xPos, yPos);
        ofDrawBitmapString(storeInfo, xPos + 500, yPos);
    }
    
    if (bStop)
        drawPins(pixels);
    else
        playBack(storeText, frameIndex, sequenceFPS, bPause, bStop);
}

//--------------------------------------------------------------

void RTCP::drawHeightMap()
{
    pinHeightImage.draw(0, 0, RELIEF_SIZE_X, RELIEF_SIZE_Y);
}

//--------------------------------------------------------------

void RTCP::drawGraphics()
{
    //pinHeightImage.draw(506, 506, 500, 500);
}

//--------------------------------------------------------------

void RTCP::drawDebug(int _x, int _y)
{
    pinHeightImage.draw(_x, _y, 500, 500);
    
    //ofPushStyle();
    ofSetColor(255, 0, 0);
    string message = "TCP SERVER EasyPin 006 TRANSFORM \n\nconnect on port: "+ofToString(TCP.getPort());
    ofDrawBitmapString(message, _x + 10, _y + 10);
    //ofPopStyle();
}

//--------------------------------------------------------------

void RTCP::playBack(vector <string> & _strFrames, int & _frameIndex, int _sequenceFPS, bool _pause, bool _stop)
{
    if(_strFrames.size() != 0)
    {
        if(!bPause)
        {
            _frameIndex = (int)((ofGetElapsedTimef() - elapsedTime) * _sequenceFPS) % _strFrames.size();
        }
        
        if (!bStop) {
            unsigned char * theColors = new unsigned char[size];
            for(int i = 0; i < size; i++)
            {
                theColors[i] = 255  - _strFrames[_frameIndex][i]; //invert values here because it doesn't work in maxscript
            }
            drawPins(theColors);
        }
    }
}

//--------------------------------------------------------------

void RTCP::drawPins(unsigned char * _theColors)
{
    
//    cout<<"called"<<endl;
//    //debug
    for (int i = 0; i < 30 * 30; i++) {
        _theColors[i] = (int)ofMap(i, 0, 899, 0, 255);
    }

    int rows = 30;
    int columns = 30;
        
    pinHeightImage.begin();
    ofClear(255,255,255, 0);
    
    
    //sort incoming data
    for(int i = 0; i < columns; i++) // columns
    {
        for(int j = 0; j < rows; j++) // rows
        {
            int idx = i * columns +j; //2D to 1D array
            rects.at(idx).setWidth(1);
            rects.at(idx).setHeight(1);
            
            rects.at(idx).x = i*1;
            rects.at(idx).y = j*1;
            
            ofSetColor(_theColors[idx]);
            //draw values in sorted order
            ofRect(rects.at(idx));
            //cout<<idx << " : " <<(int)_theColors[idx]<<endl;
        }
    }
    
    pinHeightImage.end();
}

//--------------------------------------------------------------

void RTCP::togglePlay()
{
    bStop = !bStop;
    bPause = false;
    elapsedTime = ofGetElapsedTimef();
}

//--------------------------------------------------------------

void RTCP::togglePause()
{
    bPause = !bPause;
    bStop = false;
    if(bPause)
        pauseTime = ofGetElapsedTimef() - elapsedTime;
    else
        elapsedTime = ofGetElapsedTimef() - pauseTime;
}
//--------------------------------------------------------------

void RTCP::clearFrames()
{
    storeText.clear();
}
//--------------------------------------------------------------

void RTCP::keyPressed(int key)
{
    if (key == 'c')
        storeText.clear();
    if (key == 'x') //toggle pause
    {
        cout<<"pause"<<endl;
        bPause = !bPause;
        bStop = false;
        if(bPause)
            pauseTime = ofGetElapsedTimef() - elapsedTime;
        else
            elapsedTime = ofGetElapsedTimef() - pauseTime;
    }
    if (key == 'z') //toggle stop
    {
        //stop
        bStop = !bStop;
        bPause = false;
        elapsedTime = ofGetElapsedTimef();
    }
}

//--------------------------------------------------------------

ofFbo RTCP::getPinHeightImage()
{
    return pinHeightImage;
}
