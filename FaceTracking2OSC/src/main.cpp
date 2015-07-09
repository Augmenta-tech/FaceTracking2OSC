#include "ofMain.h"
#include "ofApp.h"

//========================================================================
int main( ){

	ofSetupOpenGL(640,480, OF_WINDOW);	
	ofSetWindowTitle("Face Tracker");
	ofSetWindowShape(ofGetScreenWidth(),ofGetScreenHeight());
	std::cout << "screen size : " <<ofGetWindowHeight() << " " << ofGetWindowWidth() << std::endl; 
	ofSetWindowPosition(0,0);
	ofRunApp( new ofApp());
}
