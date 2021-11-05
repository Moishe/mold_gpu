#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    timeStep = 0.0015f;
    numParticles = 1024 * 1024;
    
    width = ofGetWindowWidth();
    height = ofGetWindowHeight();
    
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
    }else{
        updatePos.load("",shadersFolder+"/posUpdate.frag");
        updateVel.load("",shadersFolder+"/velUpdate.frag");
        updateBlur.load("", shadersFolder+"/renderBlur.frag");
    }
    
    // Frag, Vert and Geo shaders for the rendering process of the spark image
    updateRender.setGeometryInputType(GL_POINTS);
	updateRender.setGeometryOutputType(GL_POINTS);
	updateRender.setGeometryOutputCount(1);
    updateRender.load(shadersFolder+"/render.vert", shadersFolder+"/render.frag", shadersFolder+"/render.geom");
    
    // Seting the textures where the information ( position and velocity ) will be
    textureRes = (int)sqrt((float)numParticles);
    numParticles = textureRes * textureRes;
    
    // 1. Making arrays of float pixels with position information
    vector<float> pos(numParticles*3);
    for (int x = 0; x < textureRes; x++){
        for (int y = 0; y < textureRes; y++){
            int i = textureRes * y + x;

            /*
            pos[i*3 + 0] = 0.5 + cos(float(i) / float(numParticles) * PI * 2) / 3;
            pos[i*3 + 1] = 0.5 + sin(float(i) / float(numParticles) * PI * 2) / 3;

            pos[i*3 + 0] = 0.40 + ofRandom(1) * 0.2;
            pos[i*3 + 1] = 0.40 + ofRandom(1) * 0.2;

            pos[i*3 + 0] = float(x) / float(textureRes);
            pos[i*3 + 1] = float(y) / float(textureRes);

             pos[i*3 + 0] = 0.5;
            pos[i*3 + 1] = 0.5;
*/
            
            float r = ofRandom(1) * 0.1;
            float t = ofRandom(1) * PI * 2;
            pos[i * 3 + 0] = 0.5 + cos(t) * r;
            pos[i * 3 + 1] = 0.5 + sin(t) * r;
            
            pos[i*3 + 2] = 0.0;
        }
    }
    // Load this information in to the FBO's texture
    posPingPong.allocate(textureRes, textureRes, GL_RGB32F);
    posPingPong.src->getTexture().loadData(pos.data(), textureRes, textureRes, GL_RGB);
    posPingPong.dst->getTexture().loadData(pos.data(), textureRes, textureRes, GL_RGB);
        
    vector<float> vel(numParticles*3);
    for (int i = 0; i < numParticles; i++){
        vel[i*3 + 0] = ofRandom(1) * 6.28; //float(i) / float(numParticles) * 6.28 + 3.14;
        vel[i*3 + 1] = 0;
        vel[i*3 + 2] = 0;
    }
    velPingPong.allocate(textureRes, textureRes, GL_RGB32F);
    velPingPong.src->getTexture().loadData(vel.data(), textureRes, textureRes, GL_RGB);
    velPingPong.dst->getTexture().loadData(vel.data(), textureRes, textureRes, GL_RGB);
    
    vector<float> colors(numParticles * 3);
    for (int i = 0; i < numParticles; i++) {
        colors[i * 3 + 0] = ofRandom(1);
        colors[i * 3 + 1] = ofRandom(1);
        colors[i * 3 + 2] = ofRandom(1);
    }
    colorPingPong.allocate(textureRes, textureRes, GL_RGB32F);
    colorPingPong.src->getTexture().loadData(colors.data(), textureRes, textureRes, GL_RGB);
    colorPingPong.dst->getTexture().loadData(colors.data(), textureRes, textureRes, GL_RGB);

    // Allocate the final
    renderFBO.allocate(width, height, GL_RGB32F);
    renderFBO.dst->begin();
    ofClear(0, 0, 0, 255);
    renderFBO.dst->end();

    mesh.setMode(OF_PRIMITIVE_POINTS);
    for(int x = 0; x < textureRes; x++){
        for(int y = 0; y < textureRes; y++){
            mesh.addVertex({x,y,0});
            mesh.addTexCoord({x, y});
        }
    }

    // do one round of updates to the renderFBO to make it consistent
    for (int i = 0; i < 2; i++) {
        renderFBO.dst->begin();
        ofClear(0,0,0,0);
        updateRender.begin();
        updateRender.setUniformTexture("posTex", posPingPong.dst->getTexture(), 0);
        updateRender.setUniformTexture("prevTex", renderFBO.src->getTexture(), 1);
        updateRender.setUniform1i("resolution", (float)textureRes);
        updateRender.setUniform2f("screen", (float)width, (float)height);
        ofPushStyle();
        ofEnableBlendMode( OF_BLENDMODE_DISABLED );
        ofSetColor(255);

        mesh.draw();
        
        ofDisableBlendMode();
        glEnd();
        
        updateRender.end();
        renderFBO.dst->end();
        renderFBO.swap();
        ofPopStyle();
    }
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
    velPingPong.dst->begin();
    ofClear(0);
    updateVel.begin();
    updateVel.setUniformTexture("backbuffer", velPingPong.src->getTexture(), 0);
    updateVel.setUniformTexture("posData", posPingPong.src->getTexture(), 1);
    updateVel.setUniformTexture("trailData", renderFBO.src->getTexture(), 2);
    updateVel.setUniform1i("resolution", (int)textureRes); 
    updateVel.setUniform2f("screen", (float)width, (float)height);
    updateVel.setUniform1f("timestep", (float)timeStep);

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
    posPingPong.src->draw(0, 0);
    updatePos.end();
    posPingPong.dst->end();
    posPingPong.swap();
    
    // Blur it
    for (int i = 0; i < 2; i++) {
        renderFBO.dst->begin();
        updateBlur.begin();
        updateBlur.setUniformTexture("image", renderFBO.src->getTexture(), 0);
        updateBlur.setUniform1i("horizontal", i);
        updateBlur.setUniform2f("screen", (float)width, (float)height);
        renderFBO.src->draw(0,0);
        updateBlur.end();
        renderFBO.dst->end();
    }

    renderFBO.dst->begin();
    //ofClear(0,0,0,0);
    updateRender.begin();
    updateRender.setUniformTexture("posTex", posPingPong.dst->getTexture(), 0);
    updateRender.setUniformTexture("prevTex", renderFBO.src->getTexture(), 2);
    updateRender.setUniform1i("resolution", (float)textureRes);
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
    renderFBO.dst->draw(0,0);
    
    ofSetColor(255);
    ofDrawBitmapString("Fps: " + ofToString( ofGetFrameRate()), 15,15);
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
