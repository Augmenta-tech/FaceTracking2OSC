#include "ofApp.h"
#include "ofAppBaseWindow.h"



//--------------------------------------------------------------
void ofApp::setup(){
	// Setup face finder
    finder.setup("haarcascade_frontalface_default.xml");
	
    // Setup camera
	CAM_WIDTH=  640;
	CAM_HEIGHT= 480;

	vector< ofVideoDevice > devicesList = cam.listDevices();
	if (devicesList.size()!=0){
		vector< ofVideoFormat > formatsList = devicesList[0].formats;
		if(formatsList.size()!=0){	
			CAM_WIDTH=  formatsList[0].width;
			CAM_HEIGHT= formatsList[0].height;
			cam.initGrabber(formatsList[0].width,formatsList[0].height);
		}
		else{
		cam.initGrabber(CAM_WIDTH,CAM_HEIGHT);
		}
	}
	else{
		cam.initGrabber(CAM_WIDTH,CAM_HEIGHT);
	}
    
	// Setup FinalPoint
	finalPoint = ofVec3f(CAM_WIDTH/2,CAM_HEIGHT/2,0);

    // Setup GUI with default parameters
    gui.setup();
    gui.add(uiFramerate.setup("FPS", framerate));
    gui.add(uiHost.setup("oscHost", "127.0.0.1"));
    gui.add(uiPort.setup("oscPort", ofToString(12000)));
    gui.add(scaleFactor.setup("scaleFactor", 4, 1, 8));
    gui.add(smoothFactor.setup("smoothFactor", 0.2, 0, 1));
    gui.add(threshold.setup("threshold", 0.2, 0, 0.1));
    gui.add(finderMinWidth.setup("finderMinWidth", 0, 0, 200));
    gui.add(finderMinHeight.setup("finderMinHeight", 0, 0, 200));
	gui.add(uiFinalPointX.setup("X = ", finalPointX));
	gui.add(uiFinalPointY.setup("Y = ", finalPointY));
	gui.add(uiFinalPointZ.setup("Z = ", finalPointZ));

    // Load autosave (replace default parameters if file exists)
    if(ofFile::doesFileExist("autosave.xml")){
        gui.loadFromFile("autosave.xml");
    }
    
    // Setup OSC sender with parameters defined above in GUI
    host = uiHost.getParameter().toString();
    port = ofToInt(uiPort.getParameter().toString());
    sender.setup(host, port);
    
    // Setup other variables
    pid = 0;
    age = 0;
    face.set(0, 0, 0, 0);
    oldFace.set(0, 0, 0, 0);
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
        // Augmenta-like behavior : get state to send in OSC
        // Person entered
        if(blobsNum == 0 && finder.blobs.size() != 0){
            //std::cout << "Person entered" << std::endl;
            trackingState = PERSON_ENTERED;
            pid++;
            age = 0;
        }
        // Person updated
        else if(blobsNum != 0 && finder.blobs.size() != 0){
            //std::cout << "Person updated" << std::endl;
            trackingState = PERSON_UPDATED;
            age++;
        }
        // Person left
        else if(blobsNum != 0 && finder.blobs.size() == 0){
            //std::cout << "Person left" << std::endl;
            trackingState = PERSON_LEFT;
        }
        // Update number of blobs
        blobsNum = finder.blobs.size();
        
        // Update face position
        if(finder.blobs.size() != 0){
            oldFace = face;
            ofRectangle newFace = finder.blobs[0].boundingRect;
            
            // Face data are set between 0 and 1
            newFace.set((float)newFace.getTopLeft().x / ((float)CAM_WIDTH / (float)scaleFactor),
                        (float)newFace.getTopLeft().y / ((float)CAM_HEIGHT / (float)scaleFactor),
                        (float)newFace.getWidth() / ((float)CAM_WIDTH / (float)scaleFactor),
                        (float)newFace.getHeight() / ((float)CAM_HEIGHT / (float)scaleFactor));
            
            float x = oldFace.getTopLeft().x;
            float y = oldFace.getTopLeft().y;
            float width = oldFace.getWidth();
            float height = oldFace.getHeight();
            
            // Threshold ignore new position if it has not changed enough
			if(abs(width-newFace.getWidth())    > threshold) width = newFace.getWidth();
            if(abs(height-newFace.getHeight())  > threshold) height = newFace.getHeight();
			if(abs(x-newFace.getTopLeft().x)    > threshold) x = newFace.getTopLeft().x;
			if(abs(y-newFace.getTopLeft().y)    > threshold) y = newFace.getTopLeft().y;

            // Exponential smooth on new position
            face.set(smoothFactor * oldFace.getTopLeft().x + (1-smoothFactor) * x,
                     smoothFactor * oldFace.getTopLeft().y + (1-smoothFactor) * y,
                     smoothFactor * oldFace.getWidth() + (1-smoothFactor) * width,
                     smoothFactor * oldFace.getHeight() + (1-smoothFactor) * height);

			//Setting new coordinates of FinalPoint 
			finalPoint.y = face.getCenter().y * CAM_HEIGHT;
			finalPoint.x = face.getCenter().x * CAM_WIDTH;
			finalPoint.z = (face.getHeight() * CAM_HEIGHT + face.getWidth() * CAM_WIDTH)/2;

			//GUI of FinalPoint
			finalPointX = ofToString(finalPoint.x);
			finalPointY = ofToString(finalPoint.y);
			finalPointZ = ofToString(finalPoint.z);

        }
		
	}
    
    // Send face position to osc
    ofxOscMessage m;
    m.setAddress("/head");
    m.addFloatArg(face.getCenter().x);
    m.addFloatArg(face.getCenter().y);
    m.addFloatArg(face.getWidth());
    m.addFloatArg(face.getHeight());
    sender.sendMessage(m);
    
    // Send Augmenta-simulated data
    m.clear();
    switch(trackingState){
        case PERSON_ENTERED:
            m.setAddress("/au/personEntered");
            break;
            
        case PERSON_UPDATED:
            m.setAddress("/au/personUpdated");
            break;
            
        case PERSON_LEFT:
            m.setAddress("/au/personWillLeave");
            break;
            
        default:
            break;
    }
    m.addIntArg(pid);       // pid
    m.addIntArg(0);         // oid
    m.addIntArg(age);       // age
    m.addFloatArg(face.getCenter().x);                              // centroid.x
    m.addFloatArg(1.0 - face.getHeight());                          // centroid.y
    m.addFloatArg(face.getCenter().x - oldFace.getCenter().x);      // velocity.x
    m.addFloatArg(1.0 - (face.getHeight() - oldFace.getHeight()));  // velocity.y
    m.addFloatArg(face.getCenter().y);                              // depth
    m.addFloatArg(face.getTopLeft().x);                             // boundingRect.x
    m.addFloatArg(1.0 - face.getHeight());                          // boundingRect.y
    m.addFloatArg(face.getWidth());                                 // boundingRect.width
    m.addFloatArg(face.getHeight());                                // boundingRect.height
    m.addFloatArg(face.getCenter().x);                              // highest.x
    m.addFloatArg(1.0 - face.getHeight());                          // highest.y
    m.addFloatArg(face.getCenter().y);                              // highest.z
    sender.sendMessage(m);
}

//--------------------------------------------------------------
void ofApp::draw(){
	ofNoFill();
    
    cam.draw(0, 0, CAM_WIDTH, CAM_HEIGHT);
    
    ofRect(face.getTopLeft().x, face.getTopLeft().y, face.getWidth(), face.getHeight());
    
	// Draw FinalPoint 
		finalPointDraw();

    if(finder.blobs.size() != 0){
        ofRect(face.getTopLeft().x * CAM_WIDTH,
               face.getTopLeft().y * CAM_HEIGHT,
               face.getWidth() * CAM_WIDTH,
               face.getHeight() * CAM_HEIGHT);
    }

    // Draw GUI
	uiFinalPointX.setup("X = ", finalPointX);
	uiFinalPointY.setup("Y = ", finalPointY);
	uiFinalPointZ.setup("Z = ", finalPointZ);
    uiFramerate.setup("FPS", framerate);
    gui.draw();
}


void ofApp::finalPointDraw()
{  
	ofSetColor(255,0,0);
	ofFill();
    ofCircle(finalPoint.x,finalPoint.y,10);
	ofNoFill();
	ofSetColor(255,255,255);
}

//--------------------------------------------------------------
void ofApp::exit(){
	gui.saveToFile("autosave.xml");
}

