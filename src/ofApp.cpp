#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    img.load("/Volumes/fast-external/photos-to-mold/taxi-small.jpg");

    numParticlesSqrt = 512;
    numParticles = numParticlesSqrt * numParticlesSqrt;

    timeStep = 0.0006f;
    
    width = img.getWidth(); //ofGetWindowWidth();
    height = img.getHeight(); //ofGetWindowHeight();
    
    ofSetWindowShape(1024, 1024 * (float(height) / float(width)));
    
    string shadersFolder;
    if(ofIsGLProgrammableRenderer()){
    	shadersFolder="shaders_gl3";
    }else{
    	shadersFolder="shaders";
    }

    // Loading the Shaders
    if(ofIsGLProgrammableRenderer()){
        updatePos.load(shadersFolder+"/passthru.vert", shadersFolder+"/posUpdate.frag");
        updateVel.load(shadersFolder+"/passthru.vert", shadersFolder+"/velUpdate.frag");
        updateBlur.load(shadersFolder+"/passthru.vert", shadersFolder+"/renderBlur.frag");
    }else{
        updatePos.load("",shadersFolder+"/posUpdate.frag");
        updateVel.load("",shadersFolder+"/velUpdate.frag");
        updateBlur.load("", shadersFolder+"/renderBlur.frag");
        updateColor.load(shadersFolder+"/renderColor.vert", shadersFolder+"/renderColor.frag");
    }
    
    // Frag, Vert and Geo shaders for the rendering process of the spark image
    updateRender.setGeometryInputType(GL_POINTS);
	updateRender.setGeometryOutputType(GL_POINTS);
	updateRender.setGeometryOutputCount(1);
    updateRender.load(shadersFolder+"/render.vert", shadersFolder+"/render.frag", shadersFolder+"/render.geom");
    
    // 1. Making arrays of float pixels with position & color information
    vector<float> pos(numParticles*3);
    vector<float> colors(numParticles * 3);
    int i = 0;
    do {
        float x = ofRandom(1);
        float y = ofRandom(1);
        int xx = int(x * float(width));
        int yy = int(y * float(height));
        int idx = xx + yy * width;
        ofColor oc = img.getColor(xx, yy);
        colors[i * 3 + 0] = oc.r / 256.0;
        colors[i * 3 + 1] = oc.g / 256.0;
        colors[i * 3 + 2] = oc.b / 256.0;
        float sum = colors[i * 3] + colors[i * 3 + 1] + colors[i * 3 + 2];
        int m = max(max(oc.r, oc.g), oc.b);
        if (/*m > 168 && sum > 1.0 && */sum > (ofRandom(1) * 3.0)) {
            pos[i * 3] = x;
            pos[i * 3 + 1] = y;
            i++;
        }
    } while (i < numParticles);
    
    posPingPong.allocate(numParticlesSqrt, numParticlesSqrt, GL_RGB32F);
    posPingPong.src->getTexture().loadData(pos.data(), numParticlesSqrt, numParticlesSqrt, GL_RGB);
    posPingPong.dst->getTexture().loadData(pos.data(), numParticlesSqrt, numParticlesSqrt, GL_RGB);

    colorPingPong.allocate(numParticlesSqrt, numParticlesSqrt, GL_RGB32F);
    colorPingPong.src->getTexture().loadData(colors.data(), numParticlesSqrt, numParticlesSqrt, GL_RGB);
    colorPingPong.dst->getTexture().loadData(colors.data(), numParticlesSqrt, numParticlesSqrt, GL_RGB);

    vector<float> vel(numParticles*3);
    for (int i = 0; i < numParticles; i++){
        vel[i*3 + 0] = ofRandom(1) * PI * 2.0 - PI;
        vel[i*3 + 1] = 0;
        vel[i*3 + 2] = 0;
    }
    velPingPong.allocate(numParticlesSqrt, numParticlesSqrt, GL_RGB32F);
    velPingPong.src->getTexture().loadData(vel.data(), numParticlesSqrt, numParticlesSqrt, GL_RGB);
    velPingPong.dst->getTexture().loadData(vel.data(), numParticlesSqrt, numParticlesSqrt, GL_RGB);
    
    // Allocate the final
    renderFBO.allocate(width, height, GL_RGB32F);
    renderFBO.dst->begin();
    ofClear(0, 0, 0, 255);
    renderFBO.dst->end();

    mesh.setMode(OF_PRIMITIVE_POINTS);
    for(int x = 0; x < numParticlesSqrt; x++){
        for(int y = 0; y < numParticlesSqrt; y++){
            mesh.addVertex({x, y, 0});
            mesh.addTexCoord({x, y});
        }
    }

    // Seed the render buffer with the original image.
    renderFBO.dst->getTexture().loadData(img.getPixels());
    renderFBO.src->getTexture().loadData(img.getPixels());
    /*
    for (int i = 0; i < 2; i++) {
        renderFBO.dst->begin();
        ofClear(0,0,0,0);
        updateRender.begin();
        updateRender.setUniformTexture("posTex", posPingPong.dst->getTexture(), 0);
        updateRender.setUniformTexture("colorTex", colorPingPong.dst->getTexture(), 1);
        updateRender.setUniform2f("screen", (float)width, (float)height);
        ofPushStyle();
        ofEnableBlendMode( OF_BLENDMODE_ALPHA );
        ofSetColor(255);

        mesh.draw();
        
        ofDisableBlendMode();
        glEnd();
        
        updateRender.end();
        renderFBO.dst->end();
        renderFBO.swap();
        ofPopStyle();
    }
    */
}

//--------------------------------------------------------------
void ofApp::update(){
    // In each cycle it's going to update the velocity first and the the position
    // Each one one with a different shader and FBO.
    // Because on GPU you can't write over the texture that you are reading we are
    // using to pair of ofFbo attached together on what we call pingPongBuffer 
    // Learn more about Ping-Pong at:
    //
    // http://www.comp.nus.edu/~ashwinna/docs/PingPong_FBO.pdf
    // http://www.seas.upenn.edu/~cis565/fbo.htm#setupgl4
    
    // Velocities PingPong
    //
    // It calculates the next frame and see if it's there any collition
    // then updates the velocity with that information
    //
    static int step = 0;
    velPingPong.dst->begin();
    ofClear(0);
    updateVel.begin();
    updateVel.setUniformTexture("backbuffer", velPingPong.src->getTexture(), 0);
    updateVel.setUniformTexture("posData", posPingPong.src->getTexture(), 1);
    updateVel.setUniformTexture("colorData", colorPingPong.src->getTexture(), 2);
    updateVel.setUniformTexture("trailData", renderFBO.src->getTexture(), 3);
    updateVel.setUniform2f("screen", (float)width, (float)height);
    updateVel.setUniform1f("timestep", (float)timeStep);
    updateVel.setUniform1i("dir_delta", step++ % 2);

    velPingPong.src->draw(0, 0);
    
    updateVel.end();
    velPingPong.dst->end();
    
    velPingPong.swap();
    
    
    // Positions PingPong
    //
    // With the velocity calculated updates the position
    //
    posPingPong.dst->begin();
    ofClear(0);
    updatePos.begin();
    updatePos.setUniformTexture("prevPosData", posPingPong.src->getTexture(), 0); // Previus position
    updatePos.setUniformTexture("velData", velPingPong.src->getTexture(), 1);  // Velocity
    updatePos.setUniform1f("timestep",(float) timeStep );
    updatePos.setUniform1f("locx", (float)ofRandom(1));
    updatePos.setUniform1f("locy", (float)ofRandom(1));
    posPingPong.src->draw(0, 0);
    updatePos.end();
    posPingPong.dst->end();
    posPingPong.swap();

    // Blur it
    for (int i = 0; i < 2; i++) {
        renderFBO.dst->begin();
        updateBlur.begin();
        updateBlur.setUniformTexture("image", renderFBO.src->getTexture(), 0);
        updateBlur.setUniform1i("horizontal", i == 0);
        updateBlur.setUniform2f("screen", (float)width, (float)height);
        renderFBO.src->draw(0,0);
        updateBlur.end();
        renderFBO.dst->end();
        if (i == 0) {
            renderFBO.swap();
        }
    }

    renderFBO.dst->begin();
    //ofClear(0,0,0,0);
    updateRender.begin();
    updateRender.setUniformTexture("posTex", posPingPong.dst->getTexture(), 0);
    updateRender.setUniformTexture("colorTex", colorPingPong.dst->getTexture(), 1);
    updateRender.setUniform2f("screen", (float)width, (float)height);
    
    ofPushStyle();
    ofEnableBlendMode( OF_BLENDMODE_ALPHA );
    ofSetColor(255);

    mesh.draw();
    
    ofDisableBlendMode();
    glEnd();
    
    updateRender.end();
    renderFBO.dst->end();
    renderFBO.swap();
    ofPopStyle();
}

//--------------------------------------------------------------
void ofApp::draw(){
    ofBackground(0);
    
    ofSetColor(255,255,255);
    renderFBO.dst->draw(0,0, ofGetWindowWidth(), ofGetWindowHeight());

    ofSetColor(255);
    ofDrawBitmapString("Fps: " + ofToString( ofGetFrameRate()), 15, 15);
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

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
    ofPixels pix;
    renderFBO.dst->getTexture().readToPixels(pix);
    ofImage img(pix);
    std::string filename = "/Users/moishe/gen-images/saved-image-foobar";
    //filename.append(gen_random(5));
    filename.append(".jpg");
    img.save(filename);

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
