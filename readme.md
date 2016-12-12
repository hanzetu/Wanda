#Wanda
So you wanna play with magic! This is a Leap Motion based gesture control game that one can cast spells to generate sound and visual effects by changing gestures. Try yourself with different magics with the following instruction:

##Play Magic Instruction  
1. Use your two hands to move up, down, left, right, forward, and backward to control flame size, sound volume, sound spatialization, and sound effects. 
2. Initially, you are at "Gentle Magic" mode. Under this mode, move your left hand out of tracking area then move back in to generate a big fireball. If you are at other magic modes, **touch two index finger** tips to switch back to this mode.
3. **Touch two thumb tips** to switch to "Burst Ignition" mode. Under this mode, the right hand casts fire sparks. One can use that to burn the gentle flame-smoke mixture casted by left hand.  
4. Touch the two heads of metacarpals of pinky fingers (i.e. **touch the second joints of two pinky fingers**) to switch to "Double Burst Ignition" mode. It also generates a weird ambient sound effect

Try not to overlap hands as leap motion cannot detect blocked hands.


##Run Instructions:   
1. Connect Leap Motion to your computer and install drivers needed.
2. For Mac, run WandaDebug.app under Wanda/bin folder.

##Compile Instructions:  
1. Install Leap Motion V2 Desktop SDK at https://developer.leapmotion.com  
2. In Xcode, run Wanda.xcodeproj and compile
3. OS X 10.11.6 tested, but it should work on all Windows/mac OS/Linux platforms
4. The project uses 5 addons: ofxSimpleTimer, ofxStk, ofxFluid, ofxFX, and ofxLeapMotion2. If compiler error says package missing, please install these addons to openFrameworks.
