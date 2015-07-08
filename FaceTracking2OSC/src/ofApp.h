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
	float getZFromOfRect(ofRectangle rect);
	void finalPointDraw();
	void finalPointUpdate();



private:
    ofxCvHaarFinder finder;
    ofVideoGrabber cam;
    ofVec3f finalPoint;
    ofRectangle face;
    
    ofxOscSender sender;
    string host;
    int port;
    
    ofxPanel gui;
    string framerate;
    ofxLabel uiFramerate;
    ofxLabel uiHost;
    ofxLabel uiPort;
    ofxIntSlider scaleFactor;
    ofxIntSlider finderMinWidth;
    ofxIntSlider finderMinHeight;
	
};
