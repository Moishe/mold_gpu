#include "ofApp.h"
#include <algorithm>

float maxage = 100.0;

//--------------------------------------------------------------
void ofApp::setup(){
    bool radial_fill = true;
    float fill_radius = 0.8;
    bool seed_orig_image = true;
    int min_color_vector_length = 64;
    bool actor_for_every_pixel = false;
    int pixel_step = 1;
    float direction_offset = 0;

    img.load("/Volumes/fast-external/new-photos-to-mold/selfie-2.jpg");
    filename = "/Volumes/fast-external/new-photos-to-mold/selfie-2";

    width = int(img.getWidth() / 2) * 2;
    height = int(img.getHeight() / 2) * 2;
    
    if (actor_for_every_pixel) {
        numParticlesSqrt = int(sqrt((width * height) / pixel_step) + 1);
    } else {
        numParticlesSqrt = 4096 ;
    }
    numParticles = numParticlesSqrt * numParticlesSqrt;

    timeStep = 1.0/(max(width, height)) * 0.1;
    
    ofSetWindowShape(768, 768 * (float(height) / float(width)));
    
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
        updateColor.load(shadersFolder+"/passthru.vert", shadersFolder+"/colorUpdate.frag");
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
    vector<float> vel(numParticles * 3);
    if (actor_for_every_pixel) {
        for (int i = 0; i < (width * height) / pixel_step; i++) {
            int x = (i * pixel_step) % width;
            int y = (i * pixel_step) / width;
            pos[i * 3] = x;
            pos[i * 3 + 1] = y;
            pos[i * 3 + 2] = 0;

            ofColor oc = img.getColor(x, y);
            ofVec3f color = ofVec3f(oc.r / 256.0, oc.g / 256.0, oc.b / 256.0);
            colors[i * 3 + 0] = color.x;
            colors[i * 3 + 1] = color.y;
            colors[i * 3 + 2] = color.z;

            vel[i * 3 + 0] = ofRandom(1) * PI * 2;
            vel[i * 3 + 1] = ofRandom(1) * maxage;
            vel[i * 3 + 2] = 0;
        }
    } else {
        float xorig = 0.662;
        float yorig = 0.365;
        for (int i = 0; i < numParticles; i++) {
            if (!radial_fill) {
                if (true) {
                    ofColor oc;
                    do {
                        int xx, yy;
                        do {
                            float x = ofRandom(1);
                            float y = ofRandom(1);

                            pos[i * 3] = x;
                            pos[i * 3 + 1] = y;
                            pos[i * 3 + 2] = 0;
                            
                            xx = min(int(x * float(width)), width - 1);
                            yy = min(int(y * float(height)), height - 1);
                        } while (xx < 0 || yy < 0);
                        int idx = xx + yy * width;
                        oc = img.getColor(xx, yy);
                        ofVec3f color = ofVec3f(oc.r / 256.0, oc.g / 256.0, oc.b / 256.0);
                        colors[i * 3 + 0] = color.x;
                        colors[i * 3 + 1] = color.y;
                        colors[i * 3 + 2] = color.z;
                    } while ((oc.r + oc.g + oc.b) < min_color_vector_length);
                    vel[i * 3 + 1] = ofRandom(maxage);
                } else {
                    pos[i * 3] = -1;
                    pos[i * 3 + 1] = -1;
                    pos[i * 3 + 2] = 0;
                    colors[i * 3] = 1.0;
                    colors[i * 3 + 1] = 1.0;
                    colors[i * 3 + 2] = 1.0;
                    vel[i * 3 + 1] = 0;
                }

                vel[i * 3 + 0] = ofRandom(1) * PI * 2;
                vel[i * 3 + 2] = 100;
            } else {
                if (i == numParticles / 2) {
                    //xorig = 0.5;
                    //yorig = 0.9;
                }
                float t, r, x, y;
                int xx, yy;
                ofColor oc;
                do {
                    do {
                        t = ofRandom(1) * PI * 2;
                        r = ofRandom(1) * fill_radius;
                        x = xorig + cos(t) * r;
                        y = yorig + sin(t) * r * (float(width) / float(height));
                        pos[i * 3] = x;
                        pos[i * 3 + 1] = y;
                        pos[i * 3 + 2] = 0;
                        
                        xx = min(int(x * float(width)), width - 1);
                        yy = min(int(y * float(height)), height - 1);
                    } while (xx < 0 || yy < 0);
                    int idx = xx + yy * width;
                    ofColor oc = img.getColor(xx, yy);
                    ofVec3f color = ofVec3f(oc.r / 256.0, oc.g / 256.0, oc.b / 256.0);
                    colors[i * 3 + 0] = color.x;
                    colors[i * 3 + 1] = color.y;
                    colors[i * 3 + 2] = color.z;
                } while ((oc.r + oc.g + oc.b) < min_color_vector_length);

                vel[i * 3 + 0] = t + direction_offset; //ofRandom(1) * PI * 2; //t + PI;
                vel[i * 3 + 1] = ofRandom(1) * maxage;
                vel[i * 3 + 2] = 0;
            }
        }
    }

    /*
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
        if (sum > (ofRandom(1) * 3.0)) {
            pos[i * 3] = x;
            pos[i * 3 + 1] = y;
            i++;
        }
    } while (i < numParticles);
    */
    posPingPong.allocate(numParticlesSqrt, numParticlesSqrt, GL_RGB32F);
    posPingPong.src->getTexture().loadData(pos.data(), numParticlesSqrt, numParticlesSqrt, GL_RGB);
    posPingPong.dst->getTexture().loadData(pos.data(), numParticlesSqrt, numParticlesSqrt, GL_RGB);

    colorPingPong.allocate(numParticlesSqrt, numParticlesSqrt, GL_RGB32F);
    colorPingPong.src->getTexture().loadData(colors.data(), numParticlesSqrt, numParticlesSqrt, GL_RGB);
    colorPingPong.dst->getTexture().loadData(colors.data(), numParticlesSqrt, numParticlesSqrt, GL_RGB);

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
    if (seed_orig_image) {
        renderFBO.dst->getTexture().loadData(img.getPixels());
        renderFBO.src->getTexture().loadData(img.getPixels());
    }
}

//--------------------------------------------------------------
void ofApp::update() {
    bool save_roll = true;
    int frame_increment = 1;
    int max_frames = 60 * 60;
    static int step = 0;
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
    updatePos.setUniform1f("locx", 0.5); //(float)ofRandom(1));
    updatePos.setUniform1f("locy", 0.5); //(float)ofRandom(1));
    updatePos.setUniform1f("maxage", maxage);
    posPingPong.src->draw(0, 0);
    updatePos.end();
    posPingPong.dst->end();
    posPingPong.swap();
    
    colorPingPong.dst->begin();
    updateColor.begin();
    updateColor.setUniformTexture("prevColorData", colorPingPong.src->getTexture(), 0);
    updateColor.setUniformTexture("posData", posPingPong.src->getTexture(), 1);
    updateColor.setUniformTexture("velData", velPingPong.src->getTexture(), 2);
    updateColor.setUniformTexture("origImageData", img.getTexture(), 3);
    updateColor.setUniform2f("screen", (float)width, (float)height);
    colorPingPong.src->draw(0, 0);
    updateColor.end();
    colorPingPong.dst->end();
    colorPingPong.swap();

    velPingPong.dst->begin();
    ofClear(0);
    updateVel.begin();
    updateVel.setUniformTexture("backbuffer", velPingPong.src->getTexture(), 0);
    updateVel.setUniformTexture("posData", posPingPong.src->getTexture(), 1);
    updateVel.setUniformTexture("colorData", colorPingPong.src->getTexture(), 2);
    updateVel.setUniformTexture("trailData", renderFBO.src->getTexture(), 3);
    updateVel.setUniform2f("screen", (float)width, (float)height);
    updateVel.setUniform1f("timestep", (float)timeStep);
    updatePos.setUniform1f("maxage", maxage);
    velPingPong.src->draw(0, 0);
    updateVel.end();
    velPingPong.dst->end();
    velPingPong.swap();
        
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
    
    if (save_roll && step % frame_increment == 0 && step < max_frames) {
        ofPixels pixels;
        renderFBO.src->getTexture().readToPixels(pixels);
        ofImage img(pixels);
        std::stringstream ss;
        ss << filename << "/" << "zzy" << "-";
        ss << std::setw(10) << std::setfill('0') << std::to_string(int(step / frame_increment));
        ss << ".jpg";
        
        string fullname = ss.str();
        img.save(fullname);
    } else if (step >= max_frames) {
        ofExit();
    }
    step++;
}

//--------------------------------------------------------------
void ofApp::draw(){
    ofBackground(0);
    
    ofSetColor(255,255,255);
    renderFBO.dst->draw(0,0, ofGetWindowWidth(), ofGetWindowHeight());

    ofSetColor(255);
    ofDrawBitmapString("Fps: " + ofToString( ofGetFrameRate()) + ": " + ofToString(float(mouseX) / float(768)) + ", " + ofToString(float(mouseY) / float(768 * (float(height) / float(width)))), 15, 15);
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
    std::string filename = "/Users/moishelettvin/gen-images/saved-image-foobar";
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
