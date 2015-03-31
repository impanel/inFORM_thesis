//
//  RMovie.cpp
//  inform
//
//  Created by Philipp Schoessler on 2/11/15.
//
//

#include "RMovie.h"


RMovie::RMovie(){};

//--------------------------------------------------------------

void RMovie::setup(string _path) {
    filePath = _path;
    listAllVideos();
    pinHeightImage.allocate(RELIEF_SIZE_X, RELIEF_SIZE_Y, GL_RGBA);
    bLoop = false;
}

//--------------------------------------------------------------

void RMovie::listAllVideos()
{
    dir.listDir(filePath); // @todo move to constant
    dir.sort();
    
    // if directory is not empty, assign memory in vector
    // @todo check for dot files
    
    int newSize = (int)dir.size();
    
    if(newSize > 0) {
        videos.assign(newSize, ofVideoPlayer());
        
        // load each movie and set to first frame for display
        for(int i = 0; i < newSize; i++) {
            
            videos.resize(newSize);
            loadedVideoPaths.resize(newSize);
            loadedVideoFilenames.resize(newSize);
            
            cout << "== Loading new movie " << dir.getPath(i) << endl;
            videos[i].loadMovie(dir.getPath(i));
            loadedVideoPaths[i] = dir.getPath(i);
            loadedVideoFilenames[i] = dir.getName(i);
        }
    }
}

//--------------------------------------------------------------

void RMovie::update(float dt)
{
    // update all videos
    for(int i = 0; i < (int)videos.size(); i++)
    {
        videos[i].update();
    }
    
    // update our main video, if set
    nowPlaying.update();
    
    drawHeightMap();
}

//--------------------------------------------------------------

void RMovie::playByPath(string _path)
{
    //
    for (vector<ofVideoPlayer>::iterator it = videos.begin() ; it != videos.end(); ++it) {
        if(_path == (*it).getMoviePath()) {
            nowPlaying = *it;
            nowPlaying.play();
            cout << " now playing video by path " << _path;
        }
    }
    
    if(bLoop)
        nowPlaying.setLoopState(OF_LOOP_NORMAL);
    else
        nowPlaying.setLoopState(OF_LOOP_NONE);
}

//--------------------------------------------------------------

void RMovie::playByFilename(string _filename)
{
    _filename = filePath + "/" + _filename; // prefix this folder path
    playByPath(_filename);
    cout<< "play: " << _filename << endl;
    
}

//--------------------------------------------------------------

void RMovie::stop()
{
    nowPlaying.stop(); //
}

//--------------------------------------------------------------

void RMovie::pause()
{
    if(nowPlaying.isPlaying()) nowPlaying.setPaused(true);
}

//--------------------------------------------------------------

void RMovie::resume()
{
    if(!nowPlaying.isPlaying()) nowPlaying.play();
}

//--------------------------------------------------------------

void RMovie::reset()
{
    nowPlaying.setFrame(0);
    pause();
}

//--------------------------------------------------------------

void RMovie::nextVideo(bool _playIt)
{
    nowPlaying.getMoviePath();
    
    for (vector<ofVideoPlayer>::iterator it = videos.begin() ; it != videos.end(); ++it) {
        if(nowPlaying.getMoviePath() == (*it).getMoviePath()) {
            stop();
            reset();
            if(it - videos.begin() >= videos.size() - 1)
                nowPlaying = videos.at(0);
            else
                nowPlaying = *it.operator++();
            
            if (_playIt)
                nowPlaying.play();
            else
                nowPlaying.setPaused(true);
            cout << " now playing video by path " << nowPlaying.getMoviePath();
        }
    }
    
    if(bLoop)
        nowPlaying.setLoopState(OF_LOOP_NORMAL);
    else
        nowPlaying.setLoopState(OF_LOOP_NONE);
}

//--------------------------------------------------------------

void RMovie::previousVideo(bool _playIt)
{
    nowPlaying.getMoviePath();
    
    for (vector<ofVideoPlayer>::iterator it = videos.begin() ; it != videos.end(); ++it) {
        if(nowPlaying.getMoviePath() == (*it).getMoviePath()) {
            stop();
            reset();
            if(it - videos.begin() <= 0)
                nowPlaying = videos.at(videos.size() - 1);
            else
                nowPlaying = *it.operator--();
            
            if (_playIt)
                nowPlaying.play();
            else
                nowPlaying.setPaused(true);
            cout << " now playing video by path " << nowPlaying.getMoviePath();
        }
    }
    
    if(bLoop)
        nowPlaying.setLoopState(OF_LOOP_NORMAL);
    else
        nowPlaying.setLoopState(OF_LOOP_NONE);
}

//--------------------------------------------------------------

void RMovie::drawHeightMap()
{
    pinHeightImage.begin();
    ofClear(255,255,255, 0);
    
    if(nowPlaying.isPlaying() || nowPlaying.isPaused())
    {
        //MAKE SURE VIDEO IS RENDERED AS RGB + ALPHA
        nowPlaying.draw(0, 0, RELIEF_SIZE_X, RELIEF_SIZE_Y);
    }
    else
    {
        ofSetColor(0, 0, 0);
        ofRect(0, 0, RELIEF_SIZE_X, RELIEF_SIZE_Y);
    }
    
    pinHeightImage.end();
}

//--------------------------------------------------------------

void RMovie::drawGraphics()
{
    //draw graphics for projector
    ofSetColor(0, 0, 0);
    ofRect(0, 0, RELIEF_SIZE_X, RELIEF_SIZE_Y);
}

//--------------------------------------------------------------

void RMovie::drawDebug(int _x, int _y)
{
    nowPlaying.draw(_x, _y, RELIEF_SIZE_X, RELIEF_SIZE_Y);
}

//--------------------------------------------------------------

unsigned char * RMovie::getPixels()
{
    unsigned char * returnArr = new unsigned char[RELIEF_SIZE_X * RELIEF_SIZE_Y];
    unsigned char * temp = new unsigned char[RELIEF_SIZE_X * RELIEF_SIZE_Y * 3];
    temp = nowPlaying.getPixels();
    
    //convert 3 channel to 1 channel;
    if (temp != 0) { //check if there are pixels in the array (if the video is not playing there won't be any)
        for (int i = 1; i <= 102 * 24; i++)
        {
            char val = (temp[i * 3 - 1] + temp[i * 3 - 2] + temp[i * 3 - 3]) / 3;
            returnArr[i -1] = val;
        }
    }
    else for (int i = 1; i <= 102 * 24; i++)returnArr[i] = 0;
    
    //return nowPlaying.getPixels();
    return returnArr;
}

//--------------------------------------------------------------

unsigned char * RMovie::getColorPixels()
{
    return nowPlaying.getPixels();
}

//--------------------------------------------------------------

bool RMovie::isPlaying()
{
    return nowPlaying.isPlaying();
}

//--------------------------------------------------------------

void RMovie::setLooping(bool _set)
{
    if(_set) nowPlaying.setLoopState(OF_LOOP_NORMAL);
    else nowPlaying.setLoopState(OF_LOOP_NONE);
}

//--------------------------------------------------------------

ofFbo RMovie::getPinHeightImage()
{
    return pinHeightImage;
}

//--------------------------------------------------------------

vector <string> RMovie::getLoadedVideoFilenames(){
    return loadedVideoFilenames;
}

//--------------------------------------------------------------

vector <string> RMovie::getLoadedVideoPaths() {
    return loadedVideoPaths;
}

//--------------------------------------------------------------

bool RMovie::getLoopState()
{
    return bLoop;
    //return nowPlaying.getLoopState();
}

//--------------------------------------------------------------

void RMovie::keyPressed(int key)
{
    if(key == ' ')
    {
        if(nowPlaying.isPlaying())
            pause();
        else
            resume();
    }
    
    //advance single frame
    if(key == OF_KEY_RIGHT)
    {
        nowPlaying.nextFrame();
    }
    
    //previous frame
    if(key == OF_KEY_LEFT)
    {
        nowPlaying.previousFrame();
    }
    
    //advance single frame
    if(key == OF_KEY_DOWN)
    {
        nextVideo(true);
    }
    
    //previous frame
    if(key == OF_KEY_UP)
    {
        previousVideo(true);
    }

    if(key == 'l')
    {
        bLoop = !bLoop;
        if(bLoop)
           nowPlaying.setLoopState(OF_LOOP_NORMAL);
        else
           nowPlaying.setLoopState(OF_LOOP_NONE);
    }

}



