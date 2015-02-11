//
//  RMovie.h
//  inform
//
//  Created by Philipp Schoessler on 2/11/15.
//
//

#ifndef __inform__RMovie__
#define __inform__RMovie__

#include <stdio.h>
#include "RenderableObject.h"
#include "ofxNetwork.h"
#include <iostream>

class RMovie : public RenderableObject {
public:
    RMovie();
    void drawHeightMap();
    void drawGraphics();
    void drawDebug(int _x, int _y);
    void update(float dt);
    void setup(string _path);
    void keyPressed(int key);
    
    void playByPath(string _path);
    void playByFilename(string _filename);
    
    void play();
    void pause();
    void resume();
    void stop();
    void reset();
    void setLooping(bool _set);
    void clearFrames();
    ofFbo getPinHeightImage();
    
    vector <string>     getLoadedVideoPaths();
    vector <string>     getLoadedVideoFilenames();
    unsigned char*      getColorPixels();
    unsigned char*      getPixels();
    bool                isPlaying();
    
private:
    void listAllVideos();
    
    ofVideoPlayer nowPlaying;
    ofDirectory dir;
    vector<ofVideoPlayer>   videos;
    vector<string>          loadedVideoPaths;
    vector<string>          loadedVideoFilenames;
    string filePath;
    
    int size;
    int width;
    int height;
    
    bool bPause;
    bool bStop;
    
    int frameIndex;
    unsigned char * pixels;
    
    ofFbo pinHeightImage;
};

#endif /* defined(__inform__RMovie__) */
