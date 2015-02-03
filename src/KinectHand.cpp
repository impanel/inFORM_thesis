//
//  KinectHand.cpp
//  Relief2
//
//  Created by Daniel Leithinger on 3/29/14.
//
//

#include "KinectHand.h"


void KinectHand::computeHandTip() {
    ofPoint cornerPoints[4];
    
    // calculate the four corner points from the angleBoundingRect of handBlob
    cornerPoints[0] = ofPoint(-handBlob.angleBoundingRect.width/2, handBlob.angleBoundingRect.height/2);
    cornerPoints[1] = ofPoint(handBlob.angleBoundingRect.width/2, handBlob.angleBoundingRect.height/2);
    cornerPoints[2] = ofPoint(handBlob.angleBoundingRect.width/2, -handBlob.angleBoundingRect.height/2);
    cornerPoints[3] = ofPoint(-handBlob.angleBoundingRect.width/2, -handBlob.angleBoundingRect.height/2);
    for (int i = 0; i< 4; i++) {
        cornerPoints[i].rotate(handBlob.angle + 90, ofVec3f(0,0,1));
        cornerPoints[i] += ofPoint(handBlob.angleBoundingRect.x, handBlob.angleBoundingRect.y);
    }
    
    // decide which two corner points define the locations of the fingertips.
    // they are presumably the farthest away from the edge
    for (int i = 0; i < 4; i++) {
        for (int j = i + 1; j < 4; j++) {
            switch (mTouchingBorder) {
                case BOTTOM:
                    if (cornerPoints[i].y > cornerPoints[j].y) {
                        ofPoint temp = cornerPoints[i];
                        cornerPoints[i] = cornerPoints[j];
                        cornerPoints[j] = temp;
                    }
                    break;
                case TOP:
                    if (cornerPoints[i].y < cornerPoints[j].y) {
                        ofPoint temp = cornerPoints[i];
                        cornerPoints[i] = cornerPoints[j];
                        cornerPoints[j] = temp;
                    }
                    break;
                case LEFT:
                    if (cornerPoints[i].x < cornerPoints[j].x) {
                        ofPoint temp = cornerPoints[i];
                        cornerPoints[i] = cornerPoints[j];
                        cornerPoints[j] = temp;
                    }
                    break;
                case RIGHT:
                    if (cornerPoints[i].x > cornerPoints[j].x) {
                        ofPoint temp = cornerPoints[i];
                        cornerPoints[i] = cornerPoints[j];
                        cornerPoints[j] = temp;
                    }
                    break;
                default:
                    break;
            }
        }
    }
    
    // now that we have the points sorted, find the midPoint, which is the hand tip
    handTip = cornerPoints[0].getMiddle(cornerPoints[1]);
    ofVec2f xAxis(1, 0);
    // compute the angle between the hand tip and the centroid of the hand
    handAngle = xAxis.angle(handTip - handBlob.centroid);
}

// determine which border the hand touches. @todo: this function assumes that the hand is touching only one border
void KinectHand::determineTouchingBorder(int borderWidth, int borderHeight) {
    mTouchingBorder = NONE;
    if (handBlob.boundingRect.getBottomLeft().y == borderHeight - 1) {
        mTouchingBorder = BOTTOM;
    }
    else if (handBlob.boundingRect.getTopLeft().y == 1) {
        mTouchingBorder = TOP;
    }
    else if (handBlob.boundingRect.getBottomRight().x == borderWidth - 1) {
        mTouchingBorder = RIGHT;
    }
    else if (handBlob.boundingRect.getTopLeft().x == 1){
        mTouchingBorder = LEFT;
    }
}

// creates a hand from a blob, determining if the blob touches the image border and calculating the hand tip and the hand angle
bool KinectHand::createHandFromBlob(Blob blob, int borderWidth, int borderHeight) {
    handBlob = blob;
    determineTouchingBorder(borderWidth, borderHeight);
    if (mTouchingBorder == NONE)
        return false;
    
    // if the blob touches the border, we assume it's a hand. we then compute the hand tip and return true
    computeHandTip();
    return true;
}

void KinectHand::drawDebug() {
    ofSetColor(0, 255, 0);
    ofEllipse(handTip, 10, 10);
    
    ofPushMatrix();
    ofTranslate(handTip.x, handTip.y, 0);
    ofRotate(handAngle, 0, 0, 1);
    ofLine(0,0,100, 0);
    //ofLine(fingerTip, handBlob.centroid);
    ofPopMatrix();
}