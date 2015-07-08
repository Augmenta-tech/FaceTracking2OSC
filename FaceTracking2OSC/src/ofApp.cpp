#include "ofApp.h"

#define CAM_WIDTH  640
#define CAM_HEIGHT 480

//--------------------------------------------------------------
void ofApp::setup(){
	// Setup face finder
    finder.setup("haarcascade_frontalface_default.xml");
    // Setup camera
    cam.initGrabber(CAM_WIDTH, CAM_HEIGHT);
    
    // Setup GUI with default parameters
    gui.setup();
    gui.add(uiFramerate.setup("FPS", framerate));
    gui.add(uiHost.setup("oscHost", "127.0.0.1"));
    gui.add(uiPort.setup("oscPort", ofToString(12000)));
    gui.add(scaleFactor.setup("scaleFactor", 4, 1, 8));
    gui.add(smoothFactor.setup("smoothFactor", 0.2, 0, 1));
    gui.add(finderMinWidth.setup("finderMinWidth", 0, 0, 200));
    gui.add(finderMinHeight.setup("finderMinHeight", 0, 0, 200));
    
    // Load autosave (replace default parameters if file exists)
    if(ofFile::doesFileExist("autosave.xml")){
        gui.loadFromFile("autosave.xml");
    }
    
    // Setup OSC sender with parameters defined above in GUI
    host = uiHost.getParameter().toString();
    port = ofToInt(uiPort.getParameter().toString());
    sender.setup(host, port);
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
            ofRectangle oldFace = face;
            face = finder.blobs[0].boundingRect;
            face.set(smoothFactor * oldFace.getTopLeft().x + (1-smoothFactor) * (face.getTopLeft().x * scaleFactor),
                     smoothFactor * oldFace.getTopLeft().y + (1-smoothFactor) * (face.getTopLeft().y * scaleFactor),
                     smoothFactor * oldFace.getWidth() + (1-smoothFactor) * (face.getWidth() * scaleFactor),
                     smoothFactor * oldFace.getHeight() + (1-smoothFactor) * (face.getHeight() * scaleFactor));
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
    
    ofRect(face.getTopLeft().x, face.getTopLeft().y, face.getWidth(), face.getHeight());
    
    // Draw GUI
    uiFramerate.setup("FPS", framerate);
    gui.draw();
}

//--------------------------------------------------------------
void ofApp::exit(){
	gui.saveToFile("autosave.xml");
}
