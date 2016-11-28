#include "ofApp.h"

//--------------------------------------------------------------


void ofApp::setup(){
    ofEnableAlphaBlending();
    ofSetCircleResolution(100);
  
    ofSetFrameRate(60);
    ofSetVerticalSync(true);
    //ofSetLogLevel(OF_LOG_VERBOSE);
    
    
    int width = ofGetWidth();
    int height = ofGetHeight();
    
    /********** Fluid Setup ************/
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
    
//    glEnable(GL_DEPTH_TEST);
//    glEnable(GL_NORMALIZE);
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
    
    if( leap.isFrameNew() && simpleHands.size() ){
        
        leap.setMappingX(-230, 230, -ofGetWidth()/2, ofGetWidth()/2);
        leap.setMappingY(90, 490, -ofGetHeight()/2, ofGetHeight()/2);
        leap.setMappingZ(-150, 150, -200, 200);
        
        fingerType fingerTypes[] = {THUMB, INDEX, MIDDLE, RING, PINKY};
        
        for(int i = 0; i < simpleHands.size(); i++){
            for (int f=0; f<5; f++) {
                int id = simpleHands[i].fingers[ fingerTypes[f] ].id;
                ofPoint mcp = simpleHands[i].fingers[ fingerTypes[f] ].mcp; // metacarpal
                ofPoint pip = simpleHands[i].fingers[ fingerTypes[f] ].pip; // proximal
                ofPoint dip = simpleHands[i].fingers[ fingerTypes[f] ].dip; // distal
                ofPoint tip = simpleHands[i].fingers[ fingerTypes[f] ].tip; // fingertip
                //std::cout<< tip.z << "\n";
                fingersFound.push_back(id);
            }
            
            // LEFT HAND SMOKE
            if (simpleHands[i].isLeft) {
            //    std::cout<< "LEFT!\n";
            //    std::cout<<simpleHands[i].handPos<<"\n";
                fluid.addTemporalForce(ofPoint(simpleHands[i].handPos.x+900, -(simpleHands[i].handPos.y-300) ), ofPoint(0,-2), ofFloatColor(0.5,0.1,0.0), max(simpleHands[i].handPos.z/7, 5.0f));

            }
            else {
                ofPoint m = ofPoint(simpleHands[i].fingers[ fingerTypes[1] ].tip.x+700, 400-simpleHands[i].fingers[ fingerTypes[1] ].tip.y);
                ofPoint d = (m - oldM)*10.0;
                oldM = m;
                ofPoint c = ofPoint(640*ofRandom(0.1, 0.9), 480*0.5) - m;
                c.normalize();
                fluid.addTemporalForce(m, d, ofFloatColor(c.x,c.y,ofRandom(0.1, 0.9))*sin(ofGetElapsedTimef()*2), max(simpleHands[i].fingers[fingerTypes[1]].tip.z/50, 3.0f));
            }
        }

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
//                 
//                 fingersFound.push_back(finger.id());
//             }
//             if (hands[i].isLeft()) {
//                 fluid.addTemporalForce((hands[i].palmPosition())[0], -hands[i].palmPosition());
//             }
//         }
//     }
    
    
    //IMPORTANT! - tell ofxLeapMotion that the frame is no longer new.
    leap.markFrameAsOld();
}

//--------------------------------------------------------------
void ofApp::draw(){
    //ofBackgroundGradient(ofColor(90, 90, 90), ofColor(30, 30, 30),  OF_GRADIENT_LINEAR);
    
    fluid.draw();
    
    ofSetColor(200);
    ofDrawBitmapString("ofxLeapMotion - Example App\nLeap Connected? " + ofToString(leap.isConnected()), 20, 20);
    
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
        
        ofSetColor(0, 0, 255);
        ofDrawSphere(handPos.x, handPos.y, handPos.z, 20);
        ofSetColor(255, 255, 0);
        ofDrawArrow(handPos, handPos + 100*handNormal);
        
        for (int f=0; f<5; f++) {
            ofPoint mcp = simpleHands[i].fingers[ fingerTypes[f] ].mcp;  // metacarpal
            ofPoint pip = simpleHands[i].fingers[ fingerTypes[f] ].pip;  // proximal
            ofPoint dip = simpleHands[i].fingers[ fingerTypes[f] ].dip;  // distal
            ofPoint tip = simpleHands[i].fingers[ fingerTypes[f] ].tip;  // fingertip
            
            ofSetColor(0, 255, 0);
            ofDrawSphere(mcp.x, mcp.y, mcp.z, 12);
            ofDrawSphere(pip.x, pip.y, pip.z, 12);
            ofDrawSphere(dip.x, dip.y, dip.z, 12);
            ofDrawSphere(tip.x, tip.y, tip.z, 12);
            
            ofSetColor(255, 0, 0);
            ofSetLineWidth(20);
            ofLine(mcp.x, mcp.y, mcp.z, pip.x, pip.y, pip.z);
            ofLine(pip.x, pip.y, pip.z, dip.x, dip.y, dip.z);
            ofLine(dip.x, dip.y, dip.z, tip.x, tip.y, tip.z);
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

//--------------------------------------------------------------
void ofApp::exit(){
    // let's close down Leap and kill the controller
    leap.close();
}
