#include "ofMain.h"
#include "ofApp.h"

//========================================================================
int main( ){
	ofSetupOpenGL(640,480, OF_WINDOW);	
	ofSetWindowTitle("FaceTracking2OSC");
	ofRunApp( new ofApp());
}
