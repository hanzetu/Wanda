#pragma once

#include "ofMain.h"
#include "ofxLeapMotion2.h"
#include "ofxFX.h"
#include "ofxFluid.h"
#include "ofxStk.h"
#include "ofxSimpleTimer.h"


#define framerate 60

using namespace stk;

class ofApp : public ofBaseApp{
    
public:
    void setup();
    void update();
    void draw();
    
    void keyPressed  (int key);
    void keyReleased(int key);
    void mouseMoved(int x, int y );
    void mouseDragged(int x, int y, int button);
    void mousePressed(int x, int y, int button);
    void mouseReleased(int x, int y, int button);
    void windowResized(int w, int h);
    void dragEvent(ofDragInfo dragInfo);
    void gotMessage(ofMessage msg);
    void audioOut(float *output, int bufferSize, int nChannels);
    void exit();
    
    ofxLeapMotion leap;
    vector <ofxLeapMotionSimpleHand> simpleHands;
    vector <int> fingersFound;
    
    /**** Sound *****/
    FileLoop fireStartSound, fireCrackleSound, steamSound, extinguishSound, ambient;
    ADSR outputADSR, crackleADSR, smokeADSR, rHandGentleADSR;
    bool fireStartSoundPlay, fireCrackleSoundPlay, extinguishSoundPlay;
    bool isFireSoundEnd, previouslyIgnition;
    float crackleVolume, smokeVolume, steamVolume;
    float lHandPan, rHandPan;
    Resonate resonator, resonatorDouble;
    OneZero oneZeroLPF;

    /** Flame / smoke **/
    ofxFluid fluid;
    ofVec2f oldM;
    
    /** Camera for Leap Motion **/
    ofEasyCam cam;
    
    
    
private:
    int sampleRate;
    int currentFrameNum; // for the initial fireball effect

    int magicMode; // switch between magic modes
    
    int tempLeftId, lastLHandNum, lastRHandNum, lastTotalHands; // used to detect if hand is lost or reappear
    
    float flameStartSize[2*framerate];
    bool flameStartReachedMax;
    
    // ****** functions *******/
    void flameSizeInitialize();
    float flameSizeHelper(int currentFrame);
    void fireTimerComplete(int &args);
    void fireSoundReset();
    void handsAppearDisappear(int i);
    float equalPowerPanning(float pan, bool forLeftChannel);
    float panningBasedOnX(float xCoordin);
    ofxSimpleTimer fireTimer, extinguishTimer;
};
