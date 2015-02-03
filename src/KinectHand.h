//
//  KinectHand.h
//  Relief2
//
//  Created by Daniel Leithinger on 3/29/14.
//
//

#ifndef __Relief2__KinectHand__
#define __Relief2__KinectHand__

#include <iostream>

#include "ofxOpenCv.h"
#include "ofxKinect.h"
#include "ofxKCore.h"

enum touchingImageBorder {NONE, TOP, BOTTOM, LEFT, RIGHT};

class KinectHand {
private:
    touchingImageBorder mTouchingBorder = NONE;    // computes the hand tip and the angle of the hand from handBlob
    
    // compute handTip and handAngle
    void computeHandTip();
    
    // determine which border the hand touches. @todo: this function assumes that the hand is touching only one border
    void determineTouchingBorder(int borderWidth, int borderHeight);
    
public:
    Blob handBlob; // contains the tracked contour.
    ofPoint handTip; // the fingertip, calculated from angleBoundingRect of handBlob
    float handAngle = 0; // the angle between the fingertip and the centroid of handBlob
    
    // creates a hand from a blob, determining if the blob touches the image border and calculating the hand tip and the hand angle
    bool createHandFromBlob(Blob blob, int borderWidth, int borderHeight);
    
    // draw debug graphics
    void drawDebug();
};

#endif /* defined(__Relief2__KinectHand__) */
