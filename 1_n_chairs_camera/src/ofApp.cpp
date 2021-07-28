#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setupArduino(const int & version) {
	
	// remove listener because we don't need it anymore
	ofRemoveListener(ard.EInitialized, this, &ofApp::setupArduino);
    
    // it is now safe to send commands to the Arduino
    bSetupArduino = true;
    
    // print firmware name and version to the console
    ofLogNotice() << ard.getFirmwareName(); 
    ofLogNotice() << "firmata v" << ard.getMajorFirmwareVersion() << "." << ard.getMinorFirmwareVersion();
    
    // attach a servo to pin D9
    // servo motors can only be attached to pin D3, D5, D6, D9, D10, or D11
    ard.sendServoAttach(9);
}

//--------------------------------------------------------------
void ofApp::updateArduino(){

	// update the arduino, get any data or messages.
    // the call to ard.update() is required
	ard.update();
	
	// do not send anything until the arduino has been set up
	if (bSetupArduino) {
        // fade the led connected to pin D11
		// ard.sendPwm(11, (int)(128 + 128 * sin(ofGetElapsedTimef())));   // pwm...
	}
}

//--------------------------------------------------------------
string ofApp::getCaption(const char* cmd){
    cout<< "Requesting data..."<< endl;
    char buffer[128];
    std::string result = "";
    FILE* pipe = popen(cmd, "r");
    if (!pipe) throw std::runtime_error("popen() failed!");
    try {
        while (!feof(pipe)) {
            if (fgets(buffer, 128, pipe) != NULL)
                result += buffer;
        }
    } catch (...) {
        pclose(pipe);
        throw;
    }
    pclose(pipe);
	return result;
}

//--------------------------------------------------------------
void ofApp::setup(){
    ofSetWindowTitle("1_&_n_chairs / fito_segrera");
	ofSetFrameRate(60); // run at 60 fps
	ofSetVerticalSync(true);

	isFullScreen = false;
	isMlRunning = false;

	///////////////////
	// SETUP ARDUINO //
	///////////////////

	// open an outgoing connection to HOST:PORT
	sender.setup(HOST, PORT);

	//Connect to Arduino
	ard.connect("/dev/ttyACM0", 57600);

	// listen for EInitialized notification. this indicates that
	// the arduino is ready to receive commands and it is safe to
	// call setupArduino()
	ofAddListener(ard.EInitialized, this, &ofApp::setupArduino);
	bSetupArduino	= false;	// flag so we setup arduino when its ready, you don't need to touch this :)

	//////////////////
	// CAMERA SETUP //
	//////////////////

	camWidth = 1280;  // try to grab at this size.
    camHeight = 720;

    //get back a list of devices.
    vector<ofVideoDevice> devices = cam.listDevices();

    for(size_t i = 0; i < devices.size(); i++){
        if(devices[i].bAvailable){
            //log the device
            ofLogNotice() << devices[i].id << ": " << devices[i].deviceName;
        }else{
            //log the device and note it as unavailable
            ofLogNotice() << devices[i].id << ": " << devices[i].deviceName << " - unavailable ";
        }
    }

	cam.setDeviceID(0);
    cam.setDesiredFrameRate(30);
    cam.initGrabber(camWidth, camHeight);
}

//--------------------------------------------------------------
void ofApp::update(){
	ofxOscMessage m;
	m.setAddress("/mouse/position");
	m.addFloatArg(ofMap(ofGetMouseX(), 0, ofGetWidth(), 0.f, 1.f, true));
	m.addFloatArg(ofMap(ofGetMouseY(), 0, ofGetHeight(), 0.f, 1.f, true));
	sender.sendMessage(m, false);

	updateArduino();

	cam.update();
	if(cam.isFrameNew()){
        ofPixels & pixels = cam.getPixels();
        //
    }

	////////////////////////////////
	// RUN MACHINE LEARNING MODEL //
	////////////////////////////////
	if(isMlRunning){
		string result = getCaption("sudo python3 data/chainer-caption/sample_code_beam.py --rnn-model data/chainer-caption/data/caption_en_model40.model --cnn-model data/chainer-caption/data/ResNet50.model --vocab data/chainer-caption/data/MSCOCO/mscoco_caption_train2014_processed_dic.json --gpu -1 --img data/frame.png");
		vector <string> splittedResult = ofSplitString( result, "\n");
		vector <string> captions;
		for(int i=0; i<splittedResult.size(); i++){
			if(i%2 != 0 && splittedResult[i] != ""){
				ofLog(OF_LOG_NOTICE, splittedResult[i]);
				captions.push_back(splittedResult[i]);
			}
		}
		ofSystem("sudo python3 data/image-downloader/app.py " + captions[0]);
		isMlRunning = false;
		splittedResult.clear();
		captions.clear();
	}
}

//--------------------------------------------------------------
void ofApp::draw(){
	cam.draw(0, 0, camWidth, camHeight);
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
	// send a test message
	if(key == 'a' || key == 'A'){
		ofxOscMessage m;
		m.setAddress("/test");
		m.addIntArg(1);
		m.addFloatArg(3.5f);
		m.addStringArg("hello");
		m.addFloatArg(ofGetElapsedTimef());
		sender.sendMessage(m, false);
	}

	if(key == 'f' || key == 'F'){
		isFullScreen = !isFullScreen;
		ofSetFullscreen(isFullScreen);
		if(isFullScreen){
			camWidth = 1920;
			camHeight = 1080;
		}else{
			camWidth = 1280;
			camHeight = 720;
		}
	}

	if(key == 's' || key == 'S'){
		frame.grabScreen(0, 0 , 640, 360);
        frame.save("frame.png");
		isMlRunning = true;
	}

	switch (key) {
        case OF_KEY_RIGHT:
            // rotate servo head to 180 degrees
            ard.sendServo(9, 180, false);
            break;
        case OF_KEY_LEFT:
            // rotate servo head to 0 degrees
            ard.sendServo(9, 0, false);
            break;
        default:
            break;
    }
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
