#include "ofApp.h"

#define CAM_WIDTH  640
#define CAM_HEIGHT 480

//--------------------------------------------------------------
void ofApp::setup(){
	// Setup face finder
    finder.setup("haarcascade_frontalface_default.xml");
    // Setup camera
    cam.initGrabber(CAM_WIDTH, CAM_HEIGHT);
    // Setup OSC sender
    host = "127.0.0.1";
    port = 12000;
    sender.setup(host, port);
    
    // Setup GUI
    gui.setup();
    gui.add(uiFramerate.setup("FPS", framerate));
    gui.add(scaleFactor.setup("scaleFactor", 4, 1, 8));
    gui.add(finderMinWidth.setup("finderMinWidth", 0, 0, 200));
    gui.add(finderMinHeight.setup("finderMinHeight", 0, 0, 200));
}

//--------------------------------------------------------------
void ofApp::update(){
    // Update framerate for GUI
    framerate = ofToString(ofGetFrameRate());
    
    // Update camera
    cam.update();
	if(cam.isFrameNew()) {
        // Get camera image
        ofImage img;
        img.setFromPixels(cam.getPixelsRef());
        // Use resized image for better performance
        img.resize(CAM_WIDTH/scaleFactor, CAM_HEIGHT/scaleFactor);
        // Find face
        finder.findHaarObjects(img, finderMinWidth/scaleFactor, finderMinHeight/scaleFactor);
        
        // Update face position
        if(finder.blobs.size() != 0){
            face = finder.blobs[0].boundingRect;
        }
	}
    
    // Send face position to osc
    ofxOscMessage m;
    m.setAddress("/head");
    m.addFloatArg(face.getCenter().x / (float)CAM_WIDTH);
    m.addFloatArg(face.getCenter().y / (float)CAM_HEIGHT);
    m.addFloatArg(face.getWidth() / (float)CAM_WIDTH);
    m.addFloatArg(face.getHeight() / (float)CAM_HEIGHT);
    sender.sendMessage(m);
}

//--------------------------------------------------------------
void ofApp::draw(){
	ofNoFill();
    
    cam.draw(0, 0, CAM_WIDTH, CAM_HEIGHT);
    
	for(unsigned int i = 0; i < finder.blobs.size(); i++) {
		ofRectangle cur = finder.blobs[i].boundingRect;
		ofRect(cur.x * scaleFactor, cur.y * scaleFactor, cur.width * scaleFactor, cur.height * scaleFactor);
	}
    
    // Draw GUI
    uiFramerate.setup("FPS", framerate);
    gui.draw();
}
