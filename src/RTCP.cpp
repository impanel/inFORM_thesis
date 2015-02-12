//
//  RTCP.cpp
//  inform
//
//  Class to connect 3dsmax directl to the inFORM
//
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
    TCP.setup(11991);
    TCP.setMessageDelimiter("\n");
    
    //fill the vector<>
    for(int i = 0; i < size; i++)
    {
        pixels[i] = 0;
        rects.push_back(ofRectangle());
        rects.at(i).setHeight(1);
        rects.at(i).setWidth(1);
    }
    
    bStop = true;
    bPause = false;
    sequenceFPS = 30;
    
    pinHeightImage.allocate(RELIEF_SIZE_X, RELIEF_SIZE_Y, GL_RGBA);
}

//--------------------------------------------------------------

void RTCP::update(float dt){
    
    
    //get data from host (3dsmax)
    
    //for each connected client lets get the data being sent and lets print it to the screen
    for(unsigned int i = 0; i < (unsigned int)TCP.getLastID(); i++)
    {
        if(!TCP.isClientConnected(i) )continue;
        
        //we only want to update the text we have recieved there is data
        string str = TCP.receive(i); //incoming data (30 * 30 8-bit values)
        
        if(str.length() > 0)
        {
            //store data in vector for later plaback
            storeText.push_back(str);
            
            //invert values here because we can't do it in maxscript
            for(int i = 0; i < size; i++)
                pixels[i] = 255  - str[i]; 
        }
    }
    
    //if no playback just show the most current data
    if (bStop)
        drawPins(pixels);
    
    // else plaback the animation
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
    //graphics for projector
}

//--------------------------------------------------------------

void RTCP::drawDebug(int _x, int _y)
{
    pinHeightImage.draw(_x, _y, 500, 500);
    
    //draw info about TCP
    if(TCP.isClientConnected(0))
    {
        //ofPushStyle();
        ofSetColor(255, 0, 0);
        
        //where to draw the text
        int xPos = 6;
        int yPos = 520;
        
        //get the ip and port of the client
        string port = ofToString( TCP.getClientPort(0) );
        string ip   = TCP.getClientIP(0);
        string info = "client "+ofToString(0)+" -connected from "+ip+" on port: "+port;
        string storeInfo = "stored frames: " + ofToString(storeText.size()) + " fIdx: " + ofToString(frameIndex);
        
        //draw the info text and the received text bellow it
        ofDrawBitmapString(info, xPos, yPos);
        ofDrawBitmapString(storeInfo, xPos, yPos + 35);
    }
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
                //invert values here because we can't do it in maxscript
                theColors[i] = 255  - _strFrames[_frameIndex][i];
            }
            drawPins(theColors);
        }
    }
}

//--------------------------------------------------------------

void RTCP::drawPins(unsigned char * _theColors)
{
    int rows = RELIEF_SIZE_X;
    int columns = RELIEF_SIZE_Y;
        
    pinHeightImage.begin();
    ofClear(255,255,255, 0);
    
    //sort incoming data
    for(int i = 0; i < columns; i++) // columns
    {
        for(int j = 0; j < rows; j++) // rows
        {
            //2D to 1D array
            int idx = i * columns +j; 
            
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
    //clear frame array
    if(key == 'c')
    {
        bStop = true;
        frameIndex = 0;
        for(int i = 0; i < size; i++)
            pixels[i] = 0;
        storeText.clear();
    }
    
    //toggle pause/play
    if(key == ' ')
    {
        bPause = !bPause;
        bStop = false;
        if(bPause)
        {
            cout<<"pause"<<endl;
            pauseTime = ofGetElapsedTimef() - elapsedTime;
        }
        else
        {
            cout<<"play"<<endl;
            elapsedTime = ofGetElapsedTimef() - pauseTime;
        }
    }
    
    //toggle stop
    if(key == 'z')
    {
        //stop
        bStop = !bStop;
        bPause = false;
        elapsedTime = ofGetElapsedTimef();
    }
    
    //advance single frame
    if(key == OF_KEY_RIGHT)
    {
        if (frameIndex < storeText.size() - 1)
            frameIndex++;
    }
    
    //rewind single frame
    if(key == OF_KEY_LEFT)
    {
        if (frameIndex != 0)
            frameIndex--;
    }
    
}

//--------------------------------------------------------------

ofFbo RTCP::getPinHeightImage()
{
    return pinHeightImage;
}
