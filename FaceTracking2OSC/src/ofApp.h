#pragma once

#include "ofMain.h"
#include "ofxCvHaarFinder.h"
#include "ofxGui.h"
#include "ofxOsc.h"

class ofApp : public ofBaseApp{
public:
    void setup();
    void update();
    void draw();
	void exit();

private:
    ofxCvHaarFinder finder;
    ofVideoGrabber cam;
    
    ofRectangle face;
    ofRectangle oldFace;
    
    ofxOscSender sender;
    string host;
    int port;
    
    ofxPanel gui;
    string framerate;
    ofxLabel uiFramerate;
    ofxLabel uiHost;
    ofxLabel uiPort;
    ofxIntSlider scaleFactor;
    ofxFloatSlider smoothFactor;
    ofxFloatSlider threshold;
    ofxIntSlider finderMinWidth;
    ofxIntSlider finderMinHeight;
};
