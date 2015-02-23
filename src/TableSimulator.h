//
//  TableSimulator.h
//  basicExample
//
//  Created by Tangible Media Group on 3/20/14.
//
//

#ifndef __basicExample__TableSimulator__
#define __basicExample__TableSimulator__

#include "ofMain.h"
#include "ReliefIOManager.h"
#include "Constants.h"

class TableSimulator {

public:

    TableSimulator(ReliefIOManager *reliefManager);
    ~TableSimulator();

    void setup();
    void update();

    // table feedback
    void drawActualPinHeightImageFromTable(int x, int y, int w, int h);

    // cam
    void drawCamViewMessage();
    void drawTableCamView(int px, int py, int w, int h, float zoom);
    void drawInteractionArea(int px, int py, int w, int h);
    void drawPinDisplaySimulation();

    ofEasyCam cam; // add mouse controls for camera movement

    unsigned char pinHeightMap [RELIEF_SIZE_X][RELIEF_SIZE_Y];

    ofLight pointLight;
    ofMaterial material;


private:
    ofMatrix3x3 mat4ToMat3(ofMatrix4x4 mat4);
    ReliefIOManager * mIOManager; // manages communication with the pin display

};

#endif