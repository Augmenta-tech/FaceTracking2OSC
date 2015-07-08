#include "ofApp.h"

#define CAM_WIDTH  640
#define CAM_HEIGHT 480
#define MOYENNAGE 10

//--------------------------------------------------------------
void ofApp::setup(){
	// Setup face finder
    finder.setup("haarcascade_frontalface_default.xml");
    // Setup camera
    cam.initGrabber(CAM_WIDTH, CAM_HEIGHT);
	// Setup FinalPoint
	finalPoint=ofVec3f(0.0f,0.0f,0.0f);
    
    // Setup GUI with default parameters
    gui.setup();
    gui.add(uiFramerate.setup("FPS", framerate));
    gui.add(uiHost.setup("oscHost", "127.0.0.1"));
    gui.add(uiPort.setup("oscPort", ofToString(12000)));
    gui.add(scaleFactor.setup("scaleFactor", 4, 1, 8));
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
        
		finalPointUpdate();
        
		// Update face position
        if(finder.blobs.size() != 0){
            face = finder.blobs[0].boundingRect;
            face.set(face.getTopLeft().x * scaleFactor,
                     face.getTopLeft().y * scaleFactor,
                     face.getWidth() * scaleFactor,
                     face.getHeight() * scaleFactor);
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

void ofApp::finalPointUpdate()
{
    if(finder.blobs.size()>=1)
    {
		ofVec3f tempPoint(finder.blobs[0].centroid.x*scaleFactor,finder.blobs[0].centroid.y*scaleFactor,
			getZFromOfRect(finder.blobs[0].boundingRect)*scaleFactor);
		if(tempPoint.distance(finalPoint)>MOYENNAGE)
		{	
			finalPoint.x=finder.blobs[0].centroid.x*scaleFactor;
			finalPoint.y=finder.blobs[0].centroid.y*scaleFactor;
			finalPoint.z=getZFromOfRect(finder.blobs[0].boundingRect)*scaleFactor;
		}
	}
	else
	{
		finalPoint=ofVec3f(0.0f,0.0f,0.0f);
	}
}

//--------------------------------------------------------------
void ofApp::draw(){
	ofNoFill();
    
    cam.draw(0, 0, CAM_WIDTH, CAM_HEIGHT);
    
    ofRect(face.getTopLeft().x, face.getTopLeft().y, face.getWidth(), face.getHeight());
    
	// Draw FinalPoint 
	finalPointDraw();

    // Draw GUI
    uiFramerate.setup("FPS", framerate);
    gui.draw();
}


void ofApp::finalPointDraw()
{  
	ofSetColor(255,0,0);
	ofFill();
    std::cout	<< "Point final:   x="<< finalPoint.x << " y= " << finalPoint.y 
				<< " z= " << finalPoint.z << std::endl;
    ofCircle(finalPoint.x,finalPoint.y,10);
	ofNoFill();
	ofSetColor(255,255,255);
}

//--------------------------------------------------------------
void ofApp::exit(){
	gui.saveToFile("autosave.xml");
}

float ofApp::getZFromOfRect(ofRectangle rect)
{
	return (rect.getWidth()+rect.getHeight())/2.0f;
}
