//
//  RTCP.h
//  inform
//
//  Created by Philipp Schoessler on 2/5/15.
//
//

#ifndef __inform__RTCP__
#define __inform__RTCP__

#include "RenderableObject.h"
#include "ofxNetwork.h"
#include <iostream>

class RTCP : public RenderableObject {
public:
    RTCP();
    void drawHeightMap();
    void drawGraphics();
    void update(float dt);
    void setup();
    void keyPressed(int key);
    void exit();
    
    void drawPinsDebug(unsigned char * _theColors);
    void drawPins(unsigned char * _theColors);
    
    void drawDebug(int _x, int _y, int _w, int _h);
    
    void togglePlay();
    void togglePause();
    void clearFrames();
    ofFbo getPinHeightImage();
    
    unsigned char* getPixels();
    void setMovieExportMode(bool _isMovieExport);
    
    ofxTCPServer TCP;
    
private:
    void playBack(vector <string> & _strFrames, int & _frameIndex, int _sequenceFPS, bool _pause, bool _play);
    bool checkForErrors();
    vector <string> storeText;
    vector <ofRectangle> rects;
    
    int size;
    int width;
    int height;
    int sequenceFPS;
    bool bFrameIndependent;
    bool bPause;
    bool bStop;
    bool bErrorDetected;
    bool bMentionError;
    bool bEnableMovieRecording;
    int frameIndex;
    int oldCheckSum;
    float elapsedTime;
    float pauseTime;
    
    unsigned char * pixels;
    string receivedText;
    
    ofFbo pinHeightImage;
};

#endif /* defined(__inform__RTCP__) */
