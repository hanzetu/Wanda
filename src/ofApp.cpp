#include "ofApp.h"

//--------------------------------------------------------------


void ofApp::setup(){
    sampleRate = 44100; // SAMPLE RATE
    
    ofEnableAlphaBlending();
    ofSetCircleResolution(100);
  
    ofSetFrameRate(framerate);
    ofSetVerticalSync(true);
    //ofSetLogLevel(OF_LOG_VERBOSE);
    
    
    int width = ofGetWidth();
    int height = ofGetHeight();
    
    
    //************** Parameters Setup ************/
    lastLHandNum = -1; lastRHandNum = -1; lastTotalHands = 0;
    magicMode = 0;
    currentFrameNum = 0;
    
    /********** Fluid Setup ************/
    flameSizeInitialize();
    
    fluid.allocate(width, height, 0.5);
  
    fluid.dissipation = 0.99;
    fluid.velocityDissipation = 0.99;
    
    fluid.setGravity(ofVec2f(0.0,0.0));
      //  fluid.setGravity(ofVec2f(0.0,0.0098));
    
    //  Set obstacle
    fluid.begin();
    ofSetColor(0,0);
    ofSetColor(255);
  //  ofCircle(width*0.1, height*0.35, 40);
    fluid.end();
    fluid.setUseObstacles(true);
    
    
    leap.open();
    
    //cam.setOrientation(ofPoint(-20, 0, 0));
    
    
    /******* Sound Setup ************/
    fireStartSound.openFile(ofToDataPath("rawwaves/igni.wav", false));
    fireCrackleSound.openFile(ofToDataPath("rawwaves/fireCrackleProcessed.wav", false));
    steamSound.openFile(ofToDataPath("rawwaves/steamProcessed.wav", false));
    extinguishSound.openFile(ofToDataPath("rawwaves/puff.wav", false));
    ambient.openFile(ofToDataPath("rawwaves/ambient.wav", false));


    Stk::setSampleRate(sampleRate); // STK must know the sampling rate
    ofSoundStreamSetup(2, 0);
    
    fireStartSoundPlay = false;
    fireCrackleSoundPlay = false;
    extinguishSoundPlay = false;
    isFireSoundEnd = true;
    previouslyIgnition = false;
    
    outputADSR.setAllTimes(0.01, 0.01, 0.9, 0.5);
    crackleADSR.setAllTimes(1.0, 0.01, 1, 1);
    smokeADSR.setAllTimes(0.1, 0.01, 1, 0.8); // ignition
    rHandGentleADSR.setAllTimes(1, 0.01, 1, 0.4);
    
    oneZeroLPF.setCoefficients(1, 0.9);
    
    /******** Timers that control sound playback **********/
    tempLeftId = 0;
    fireTimer.setup(3000);
    extinguishTimer.setup(800);
    ofAddListener( fireTimer.TIMER_COMPLETE , this, &ofApp::fireTimerComplete) ;
    ofAddListener( extinguishTimer.TIMER_COMPLETE , this, &ofApp::fireTimerComplete) ;

}


//--------------------------------------------------------------
void ofApp::update(){
    
    
  //  fluid.addConstantForce(m, ofPoint(0,-2), ofFloatColor(0.5,0.1,0.0), 10.f);
    
    //  Update
    //

    
    fingersFound.clear();
    
    //here is a simple example of getting the hands and drawing each finger and joint
    //the leap data is delivered in a threaded callback - so it can be easier to work with this copied hand data
    
    //if instead you want to get the data as it comes in then you can inherit ofxLeapMotion and implement the onFrame method.
    //there you can work with the frame data directly.
    
    
    
    //Option 1: Use the simple ofxLeapMotionSimpleHand - this gives you quick access to fingers and palms.
    
    

    simpleHands = leap.getSimpleHands();
    ofPoint LThumb, RThumb, LIndex, RIndex, LDis, RDis;
    
    /** The code below enables Leap motion to track and notify if hands
     disappear or reappear on screen. Use this to toggle sound effects **/
    
    if (leap.isFrameNew()) {
        if (simpleHands.size() <= 2) {
            if (!simpleHands.size()) {
                handsAppearDisappear(0);
            }
            for (int i = 0; i < simpleHands.size(); i++) {
                handsAppearDisappear(i);
            }
            lastTotalHands = simpleHands.size();
        }
    }
    /*****************************************************************/
    
    /****** Leap motion tracking data *******/
    if( leap.isFrameNew() && simpleHands.size() ){
        
        leap.setMappingX(-230, 230, -ofGetWidth()/2, ofGetWidth()/2);
        leap.setMappingY(90, 490, -ofGetHeight()/2, ofGetHeight()/2);
        leap.setMappingZ(-150, 150, -200, 200);
        
        fingerType fingerTypes[] = {THUMB, INDEX, MIDDLE, RING, PINKY};
        
        for(int i = 0; i < simpleHands.size(); i++){
            
            // Register Hands
            
            
            for (int f=0; f<5; f++) {
                int id = simpleHands[i].fingers[ fingerTypes[f] ].id;
//                ofPoint mcp = simpleHands[i].fingers[ fingerTypes[f] ].mcp; // metacarpal
//                ofPoint pip = simpleHands[i].fingers[ fingerTypes[f] ].pip; // proximal
//                ofPoint dip = simpleHands[i].fingers[ fingerTypes[f] ].dip; // distal
//                ofPoint tip = simpleHands[i].fingers[ fingerTypes[f] ].tip; // fingertip
                //std::cout<< tip.z << "\n";
                fingersFound.push_back(id);
            }
            
            if (simpleHands[i].isLeft) {
                LThumb = simpleHands[i].fingers[fingerTypes[0]].tip;
                LIndex = simpleHands[i].fingers[fingerTypes[1]].tip;
                LDis = simpleHands[i].fingers[fingerTypes[4]].pip;

            }
            else {
                RThumb = simpleHands[i].fingers[fingerTypes[0]].tip;
                RIndex = simpleHands[i].fingers[fingerTypes[1]].tip;
                RDis = simpleHands[i].fingers[fingerTypes[4]].pip;

            }
            
            // thumb overlapped to change magic
            if (abs(LThumb.x - RThumb.x) <= 25 && abs(LThumb.y - RThumb.y) <= 25 && abs(LThumb.z-RThumb.z) <= 25 ) {
                magicMode = 1;
            } // index fingers overlapped to return to default magic
            else if (abs(LIndex.x - RIndex.x) <= 25 && abs(LIndex.y - RIndex.y) <= 25 && abs(LIndex.z-RIndex.z) <= 25 )
                magicMode = 0;
            else if (abs(LDis.x - RDis.x) <= 40 && abs(LDis.y - RDis.y) <= 40 && abs(LDis.z-RDis.z) <= 40 )
                magicMode = 2;
            
            
            /********** For Projector Demo Purpose **************/
//            ofPoint mleft = ofPoint(simpleHands[i].handPos.x+1100, -(simpleHands[i].handPos.y-400)); // for Projector
//            ofPoint mright = ofPoint(simpleHands[i].handPos.x+700, -(simpleHands[i].handPos.y-300)); // for Projector
//            
            //********** for Native Mac *******************/
            ofPoint mleft = ofPoint(simpleHands[i].handPos.x+900, -(simpleHands[i].handPos.y-300)); // for native Mac
            ofPoint mright = ofPoint(simpleHands[i].fingers[ fingerTypes[1] ].tip.x+700, 400-simpleHands[i].fingers[ fingerTypes[1] ].tip.y);


            // LEFT HAND SMOKE
            if (simpleHands[i].isLeft) {
                
                // Left hand lost then found. Below is the initial phase
                if (simpleHands[i].fingers[fingerTypes[0]].id != tempLeftId) {
                    fireSoundReset();
                    outputADSR.keyOn();
                    crackleADSR.keyOn();
                    
                    fireStartSoundPlay = true; // play fireball sound
                    fireTimer.start(false); // (re)start timer
                    tempLeftId = simpleHands[i].fingers[fingerTypes[0]].id;
                    
                    // Draw big fireball
                    currentFrameNum = 0;
                    
                    
                }
                else { // maintaining the current status (basically do nothing)
                }
                lHandPan = panningBasedOnX(simpleHands[i].handPos.x);
                if (magicMode != 2) {
                
                    
                    fluid.addTemporalForce(mleft, ofPoint(0 ,-5), ofFloatColor(0.5,0.1,0.0),simpleHands[i].handPos.z/flameSizeHelper(currentFrameNum));
                    
                    currentFrameNum++;
                    
                    //crackleVolume controlled by left hand z-axis:
                    crackleVolume  = max(simpleHands[i].handPos.z/350.0, 0.0);
                    
                    //crackleSound lHandPanning controlled by left hand x-axis:
                }
                else { // double burst ignition

                    std::cout << "MAGIC MODE 2" << "\n";
      
                    ofPoint d = (mleft-oldM.x)*130.0;
                    oldM = mleft;
                    fluid.addTemporalForce(mleft, d, ofFloatColor(ofRandom(0.1, 0.9),ofRandom(0.1, 0.9),ofRandom(0.1, 0.9))*sin(ofGetElapsedTimef()*440), max(simpleHands[i].fingers[fingerTypes[1]].tip.z/50, 5.0f));
                    fluid.addTemporalForce(mleft, ofPoint(0,-5), ofFloatColor(0.2,0.1,0.0),20);
                    
                    // use z-axis to control ignition sound volume
                    smokeVolume = max(simpleHands[i].handPos.z/300.0, 0.1);

                    
                    // use resonator to resonate certain noise freq
                    resonatorDouble.setResonance(ofMap(lHandPan, 0, 1, 50, 800), 20.0);
                    resonatorDouble.keyOn();
                    smokeADSR.keyOn();
                    rHandGentleADSR.keyOff();
                    
                    previouslyIgnition = true;
                    
                }

            }
            else { // right Hand smoke
                rHandPan = panningBasedOnX(simpleHands[i].handPos.x);
                
               // ofPoint m = ofPoint(simpleHands[i].fingers[ fingerTypes[1] ].tip.x+700, 400-simpleHands[i].fingers[ fingerTypes[1] ].tip.y);
                
                if (magicMode == 0) { // gentle smoke
                    ambient.setRate(1);
                    ofPoint d = (mright - oldM + ofRandom(-1, 1)) * max(simpleHands[i].handPos.z/70.0, 15.0);

                    oldM = mright;
                    ofPoint c = ofPoint(640*ofRandom(0.1, 0.9), 480*0.5) - mright;
                    c.normalize();
                    fluid.addTemporalForce(mright, d, ofFloatColor(c.x,c.y,ofRandom(0.1, 0.9))*sin(ofGetElapsedTimef()*2), max(simpleHands[i].fingers[fingerTypes[1]].tip.z/50, 0.0f));
                    
                    steamSound.setRate(ofMap(simpleHands[i].handPos.y, -450, 450, 0.6, 0.9));
                    steamVolume  = max(simpleHands[i].handPos.z/3900.0, 0.0);
 
                    // if ignition is played, turn off
                    smokeADSR.keyOff();
                    
                    // play steam gentle sound
                    rHandGentleADSR.keyOn();
                    
                    if (previouslyIgnition == true) {
                        extinguishSoundPlay = true;
                        extinguishTimer.start(false); // (re)start timer
                        previouslyIgnition = false;
                    }
                    
                   
                }
                else if (magicMode == 1) { // burst ignition
                    ambient.setRate(2);
                    ofPoint d = (mright-oldM.x)*130.0;
                    oldM = mright;
                    fluid.addTemporalForce(mright, d, ofFloatColor(255,240,ofRandom(0.1, 0.9))*sin(ofGetElapsedTimef()*3), max(simpleHands[i].fingers[fingerTypes[1]].tip.z/50, 5.0f));
                    
                    // use z-axis to control ignition sound volume
                    smokeVolume = max(simpleHands[i].handPos.z/300.0, 0.1);
                    
                    // start playing ignition sound
                    
                    // set right hand ignition panning
                    
                    // use resonator to resonate certain noise freq
                    resonator.setResonance(ofMap(rHandPan, 0, 1, 50, 800), 20.0);
                    resonator.keyOn();
                    smokeADSR.keyOn();
                    rHandGentleADSR.keyOff();
                    
                    // for future extinguish sound
                    previouslyIgnition = true;

                }
                else if (magicMode == 2) {
                    ambient.setRate( -10*(sin(ofGetElapsedTimef()/2+0.8)) );
                 //   ofPoint m = ofPoint(simpleHands[i].fingers[ fingerTypes[1] ].tip.x+700, 400-simpleHands[i].fingers[ fingerTypes[1] ].tip.y);
                    
                    ofPoint d = (mright-oldM.x)*(70.0+50*sin(ofGetElapsedTimef()) );
                    oldM = mright;
                    fluid.addTemporalForce(mright, d, ofFloatColor(ofRandom(0.1, 0.9),ofRandom(0.1, 0.9),ofRandom(0.1, 0.9))*sin(ofGetElapsedTimef()*440), max(simpleHands[i].fingers[fingerTypes[1]].tip.z/50, 5.0f));
                    fluid.addTemporalForce(mright, ofPoint(0 ,-5), ofFloatColor(ofRandom(0.1,0.9),ofRandom(0.1,0.9),ofRandom(0.1,0.9))*sin(ofGetElapsedTimef()*440),20);
                    
                    // use z-axis to control ignition sound volume
                    smokeVolume = max(simpleHands[i].handPos.z/300.0, 0.1);
                    
                    // use resonator to resonate certain noise freq
                    resonator.setResonance(ofMap(rHandPan, 0, 1, 50, 800), 20.0);
                    resonator.keyOn();
                    smokeADSR.keyOn();
                    rHandGentleADSR.keyOff();
                    
                    previouslyIgnition = true;
                }
                
               
            }
        } // end of leap motion forloop
    }

    //crackle sound ctrl
    if (lastLHandNum < 0)
        crackleADSR.keyOff();
    if (lastRHandNum <0) {
        smokeADSR.keyOff();
        rHandGentleADSR.keyOff();
        //resonator.keyOff();
    }
    
    fluid.update();
 
    
    
    //Option 2: Work with the leap data / sdk directly - gives you access to more properties than the simple approach
    //uncomment code below and comment the code above to use this approach. You can also inhereit ofxLeapMotion and get the data directly via the onFrame callback.
    
//     vector <Hand> hands = leap.getLeapHands();
//     if( leap.isFrameNew() && hands.size() ){
//     
//     //leap returns data in mm - lets set a mapping to our world space.
//     //you can get back a mapped point by using ofxLeapMotion::getMappedofPoint with the Leap::Vector that tipPosition returns
//     leap.setMappingX(-230, 230, -ofGetWidth()/2, ofGetWidth()/2);
//     leap.setMappingY(90, 490, -ofGetHeight()/2, ofGetHeight()/2);
//     leap.setMappingZ(-150, 150, -200, 200);
//     
//     fingerType fingerTypes[] = {THUMB, INDEX, MIDDLE, RING, PINKY};
//     
//         for(int i = 0; i < hands.size(); i++){
//             for(int j = 0; j < 5; j++){
//                 ofPoint pt;
//                 
//                 const Finger & finger = hands[i].fingers()[ fingerTypes[j] ];
//                 
//                 //here we convert the Leap point to an ofPoint - with mapping of coordinates
//                 //if you just want the raw point - use ofxLeapMotion::getofPoint
//                 pt = leap.getMappedofPoint( finger.tipPosition() );
//                 pt = leap.getMappedofPoint( finger.jointPosition(finger.JOINT_DIP) );
//                 cout << pt << "\n";
//                 fingersFound.push_back(finger.id());
//             }
//             if (hands[i].isLeft()) {
//                 fluid.addTemporalForce((hands[i].palmPosition())[0], -hands[i].palmPosition());
//             }
//         }
//     }
    
    
    //IMPORTANT! - tell ofxLeapMotion that the frame is no longer new.
    leap.markFrameAsOld();
    
    fireTimer.update();
    extinguishTimer.update();
}

//--------------------------------------------------------------
void ofApp::draw(){
    //ofBackgroundGradient(ofColor(90, 90, 90), ofColor(30, 30, 30),  OF_GRADIENT_LINEAR);
    
    fluid.draw();
    
    ofSetColor(200);
    ofDrawBitmapString("Wanda v1.1\nPlay with your magic\nLeap Connected: " + ofToString(leap.isConnected()), 20, 20);
    
    
    cam.begin();
    
    ofPushMatrix();
    ofRotate(90, 0, 0, 1);
    ofSetColor(20);
   // ofDrawGridPlane(800, 20, false);
    ofPopMatrix();
    

    fingerType fingerTypes[] = {THUMB, INDEX, MIDDLE, RING, PINKY};
    
    for(int i = 0; i < simpleHands.size(); i++){
        bool isLeft        = simpleHands[i].isLeft;
        ofPoint handPos    = simpleHands[i].handPos;
        ofPoint handNormal = simpleHands[i].handNormal;
        
//        ofSetColor(0, 0, 255);
//        ofDrawSphere(handPos.x, handPos.y, handPos.z, 20);
//        ofSetColor(255, 255, 0);
//        ofDrawArrow(handPos, handPos + 100*handNormal);
        
        for (int f=0; f<5; f++) {
            ofPoint mcp = simpleHands[i].fingers[ fingerTypes[f] ].mcp;  // metacarpal
            ofPoint pip = simpleHands[i].fingers[ fingerTypes[f] ].pip;  // proximal
            ofPoint dip = simpleHands[i].fingers[ fingerTypes[f] ].dip;  // distal
            ofPoint tip = simpleHands[i].fingers[ fingerTypes[f] ].tip;  // fingertip
            
            ofSetColor(0, 255, 0);
//            ofDrawSphere(mcp.x, mcp.y, mcp.z, 12);
//            ofDrawSphere(pip.x, pip.y, pip.z, 12);
//            ofDrawSphere(dip.x, dip.y, dip.z, 12);
//            ofDrawSphere(tip.x, tip.y, tip.z, 12);
            
            ofSetColor(255, 185, 79);
            ofSetLineWidth(10);
            ofDrawLine(mcp.x, mcp.y, mcp.z, pip.x, pip.y, pip.z);
            ofDrawLine(pip.x, pip.y, pip.z, dip.x, dip.y, dip.z);
            ofDrawLine(dip.x, dip.y, dip.z, tip.x, tip.y, tip.z);
            
        }
        

    }
   
    cam.end();
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){
}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){
    
}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){
    
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){
    
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){
    
}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){
    
}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){
    
}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){
    
}


//--------------------- Audio Out ------------------------------
void ofApp::audioOut(float *output, int bufferSize, int nChannels) {
    if (fireStartSoundPlay) {
        StkFrames frames(bufferSize, 2), ADSRFrames(bufferSize, 1);
        fireStartSound.tick(frames);
        outputADSR.tick(ADSRFrames);
        
        for (int i = 0; i < bufferSize ; i++) {
            output[2*i] = frames(i,0)*ADSRFrames(i,0)*equalPowerPanning(lHandPan, true);
            output[2*i+1] = frames(i,1)*ADSRFrames(i,0)*equalPowerPanning(lHandPan, false);
            //std::cout << ADSRFrames(i,0) << "\n";
        }

    }
    
    // fire crackle sound
    StkFrames crackleFrames(bufferSize, 2);
    fireCrackleSound.tick(crackleFrames);
    
    // right hand gentle mode sound
    StkFrames rHandGentleFrames(bufferSize, 2);
    steamSound.tick(rHandGentleFrames);
    
    // ambient
    StkFrames ambientFrames(bufferSize, 2);
    ambient.tick(ambientFrames);
    
    for (int i = 0; i < bufferSize ; i++) {
        // fire crackle
        output[2*i] += crackleFrames(i,0)*crackleADSR.tick()*equalPowerPanning(lHandPan, true)*crackleVolume;
        output[2*i+1] += crackleFrames(i,1)*crackleADSR.tick()*equalPowerPanning(lHandPan, false)*crackleVolume;
        
        // right hand gentle smoke
        output[2*i] += rHandGentleFrames(i,0)*rHandGentleADSR.tick()*equalPowerPanning(rHandPan, true)*steamVolume;
        output[2*i+1] += rHandGentleFrames(i,1)*rHandGentleADSR.tick()*equalPowerPanning(rHandPan, false)*steamVolume;
        
        // right hand burst ignition
        output[2*i] += oneZeroLPF.tick( ofRandom(-1, 1)*smokeADSR.tick()*resonator.tick()*smokeVolume ) *equalPowerPanning(rHandPan, true);
        output[2*i+1] += oneZeroLPF.tick( ofRandom(-1, 1)*smokeADSR.tick()*resonator.tick()*smokeVolume ) *equalPowerPanning(rHandPan, false);
        
        // Double hand burst ignition
        output[2*i] += oneZeroLPF.tick( ofRandom(-1, 1)*smokeADSR.tick()*resonatorDouble.tick()*smokeVolume ) *equalPowerPanning(lHandPan, true);
        output[2*i+1] += oneZeroLPF.tick( ofRandom(-1, 1)*smokeADSR.tick()*resonatorDouble.tick()*smokeVolume ) *equalPowerPanning(lHandPan, false);
        

        // ambient background sound
        output[2*i] += ambientFrames(i,0)*0.2;
        output[2*i+1] += ambientFrames(i,1)*0.2;

    }
    
    // extinguish sound play "puff"
    if (extinguishSoundPlay) {
        StkFrames extinguishframes(bufferSize, 2);
        extinguishSound.tick(extinguishframes);
        
        for (int i = 0; i < bufferSize ; i++) {
            output[2*i] += extinguishframes(i,0)*equalPowerPanning(lHandPan, true)*0.3;
            output[2*i+1] += extinguishframes(i,1)*equalPowerPanning(lHandPan, false)*0.3;
        }
    }
    
}

//--------------------------------------------------------------
void ofApp::exit(){
    // let's close down Leap and kill the controller

    leap.close();
    ambient.closeFile();
    fireCrackleSound.closeFile();
    fireCrackleSound.closeFile();
    steamSound.closeFile();
    extinguishSound.closeFile();

}

// ********************** Private helpers **********************
void ofApp::flameSizeInitialize() {
    flameStartReachedMax = false;
    
    for (int i = 0; i < framerate; i++) {
        flameStartSize[i] = 6 - 0.0012*i*i;
    }
    for (int i = framerate; i < 2*framerate; i++) {
        flameStartSize[i] = 6 - framerate*framerate*0.0012 + 0.05*(i-framerate);
    }
}

float ofApp::flameSizeHelper(int currentFrame) {
    if (currentFrame < 2*framerate)
        return flameStartSize[currentFrame];
    else
        return flameStartSize[2*framerate-1];
}

void ofApp::fireTimerComplete(int &args) {
    outputADSR.keyOff();
    fireStartSoundPlay = false;
    fireStartSound.reset();
    fireTimer.stop();
    //std::cout << "Running? "<< fireTimer.bIsRunning << "\n";
    fireTimer.reset();
    
    extinguishSoundPlay = false;
    extinguishSound.reset();
    extinguishTimer.reset();
    
    
}

void ofApp::fireSoundReset() {
    outputADSR.keyOff();
    fireStartSoundPlay = false;
    fireStartSound.reset();
    fireTimer.stop();
    fireTimer.reset();
}

//********* Equal Power Panning Function ***********/
float ofApp::equalPowerPanning(float lHandPan, bool forLeftChannel) {
    
    // 0 <= lHandPan <= 1
    
    if( lHandPan < 0 )
        lHandPan = 0;
    else if( lHandPan > 1 )
        lHandPan = 1;
    
    // return gains
    if (forLeftChannel)
        return cos(lHandPan * pi/2) ;
    else
        return sin(lHandPan * pi/2);
}

//************* Calculating panning based on x-axis coordinate *****************/
float ofApp::panningBasedOnX(float xCoordin){
    if (xCoordin < -700)
        xCoordin = -700;
    else if (xCoordin > 700)
        xCoordin > 700;
    return ofMap(xCoordin, -700, 700, 0, 1);
}

/******* Detect if hand appear on screen or lost. Then report hand status ******/
void ofApp::handsAppearDisappear(int i) {
    
    // case 1: one hand added / appeared
    if (simpleHands.size() > lastTotalHands) {
        
        if (simpleHands[i].isLeft) { // it's the left hand that is added
            lastLHandNum = i; // then update lastLHandNum, making the new LHand #i
            std::cout << "L hand Added with ID# " << lastLHandNum << "\n";
        }
        else {// it's the right hand that is added
            lastRHandNum = i; // then update lastRHandNum, making the new RHand #i
            std::cout << "R hand Added with ID# " << lastRHandNum << "\n";

        }
        std::cout << "LH: "<<lastLHandNum<< " RH: "<< lastRHandNum << "\n\n";
    }
    // case 2: one hand disappeared
    else if (simpleHands.size() < lastTotalHands) {

        // subcase: 2hands -> 1hand
        if (simpleHands.size()) {
            if (simpleHands[i].isLeft) { //Left Hand still there. Right hand disappeared
                lastRHandNum = -1; // if disappear, assign ID# -1.
                std::cout << "R hand minused" << "\n";
            }
            else {
                lastLHandNum = -1; // if disappear, assign ID# -1.
                std::cout << "L hand minused" << "\n";
            }
        }
        
        // subcase: 1hand -> 0hand
        else { // an ID of a hand must already be -1
            if (lastLHandNum > lastRHandNum) {
                std::cout << "L hand minused. No hands now" << "\n";
                            }
            else
                std::cout << "R hand minused. No hands now" << "\n";
            lastLHandNum = -1;
            lastRHandNum = -1;

        }
        std::cout << "LH: "<<lastLHandNum<< " RH: "<< lastRHandNum << "\n\n";

    }
    else {} // no adding or deleting hands. Maintain current status
}
