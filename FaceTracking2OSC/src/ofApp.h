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
	string framerate;
	string finalPointX;
	string finalPointY;
	string finalPointZ;
    int port;
    
    ofxPanel gui;
    ofxLabel uiFramerate;
    ofxLabel uiHost;
    ofxLabel uiPort;
	ofxLabel uiFinalPointX;
	ofxLabel uiFinalPointY;
	ofxLabel uiFinalPointZ;
    ofxIntSlider scaleFactor;
    ofxIntSlider finderMinWidth;
    ofxIntSlider finderMinHeight;
	ofxIntSlider finderAntiShacking;
	
};
