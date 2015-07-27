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
    
    void sendDataToOSC();
	void drawCentroid();
	void drawOldsCentroids();
	void SmoothFaceFromAVerage();

private:
    int pid;
    int age;
    int lastUpdated;

	int CAM_HEIGHT;
	int CAM_WIDTH;

	ofxCvGrayscaleImage imgTransform;
    ofxCvHaarFinder finder;
    ofVideoGrabber cam;
    ofRectangle face;
	vector<ofRectangle> faces;
    ofRectangle oldFace;
    
    int blobsNum;
    enum trackingState{ PERSON_ENTERED, PERSON_UPDATED, PERSON_LEFT, EMPTY};
    trackingState trackingState;
    
    ofxOscSender sender;
    string host;
    int port;
    
    ofxPanel gui;
    string framerate;
    ofxLabel uiFramerate;
    ofxLabel uiHost;
    ofxLabel uiPort;
    ofxIntSlider scaleFactor;
    ofxFloatSlider smoothPos;
    ofxFloatSlider thresholdPos;
    ofxFloatSlider smoothBound;
    ofxFloatSlider thresholdBound;
    ofxIntSlider finderMinWidth;
    ofxIntSlider finderMinHeight;
    ofxIntSlider timeout;
	ofxIntSlider contrast;
	ofxIntSlider brightness;
	ofxIntSlider smoothAverage;

};
