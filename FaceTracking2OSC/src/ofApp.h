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
    
	int pid;
    int age;

    ofxCvHaarFinder finder;
    ofVideoGrabber cam;
    ofVec3f finalPoint;
    ofRectangle face;
    ofRectangle oldFace;
    
    int blobsNum;
    enum trackingState{ PERSON_ENTERED, PERSON_UPDATED, PERSON_LEFT};
    trackingState trackingState;
    
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
    ofxFloatSlider smoothFactor;
    ofxFloatSlider threshold;
    ofxIntSlider finderMinWidth;
    ofxIntSlider finderMinHeight;
	
	
};
