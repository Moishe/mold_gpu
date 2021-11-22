#include "ofApp.h"
#include <algorithm>

class Config {
public:
    static constexpr bool seed_orig_image = false;
    static const int seed_particles = 1024;
    static constexpr int min_color_vector_length = 16;
    static constexpr int pixel_step = 1;
    static constexpr float direction_offset = PI;
    
    static constexpr int num_particles_sqrt = 2048;
    static constexpr float time_step_multiplier = 0.3;

    static constexpr char *imgName = "/Users/moishe/Desktop/flower-2.jpg";
    
    static constexpr float min_age = 100;
    static constexpr float max_age = 100;
};

//--------------------------------------------------------------
void ofApp::setup(){
    img.load(Config::imgName);

    width = int(img.getWidth() / 2) * 2;
    height = int(img.getHeight() / 2) * 2;
    
    numParticlesSqrt = Config::num_particles_sqrt;
    numParticles = numParticlesSqrt * numParticlesSqrt;

    timeStep = 1.0/(max(width, height)) * Config::time_step_multiplier;
    
    ofSetWindowShape(768, 768 * (float(height) / float(width)));
    
    string shadersFolder;
    shadersFolder="shaders_gl3";
    
    // shaders that use combinations of textures to update actor state
    updatePos.load(shadersFolder + "/passthru.vert", shadersFolder + "/posUpdate.frag");
    updateColor.load(shadersFolder + "/passthru.vert", shadersFolder + "/colorUpdate.frag");
    updateVel.load(shadersFolder + "/passthru.vert", shadersFolder + "/velUpdate.frag");
    updateLife.load(shadersFolder + "/passthru.vert", shadersFolder + "/lifeUpdate.frag");
    updateRand.load(shadersFolder + "/passthru.vert", shadersFolder + "/randUpdate.frag");

    // shader that's applied to the render FBO to blur and fade
    updateBlur.load(shadersFolder + "/passthru.vert", shadersFolder + "/renderBlur.frag");
    
    updateRender.setGeometryInputType(GL_POINTS);
	updateRender.setGeometryOutputType(GL_POINTS);
	updateRender.setGeometryOutputCount(1);
    updateRender.load(shadersFolder+"/render.vert", shadersFolder+"/render.frag", shadersFolder+"/render.geom");

    // We have four textures that we pass to our shaders, that are all indexed to the same actor by x/y coordinates
    //   Position:   actor positions
    //   Colors:     the color each actor is seeking
    //   Velocities: the direction and speed each actor is traveling. This corresponds to the direction the actor is looking.
    //   Life:       each actor's lifecycle: how long they will live, their current age, and whether or not they're currently active.
    
    // We also pass a "random" texture, to facilitate random movement and behavior. This texture is updated each generation.
    
    // The pipeline for updating (in the update method) is as follows:
    //   Life:     if active, age (and maybe become inactive); if inactive, maybe become active
    //   Position: update position based on velocity; reset position if newborn (active and age == 0)
    //   Colors:   if newborn (active and age == 0), reset goal
    //   Velocity: update velocity based on goal
    
    // Also:
    //   Random:   updates random number texture with an LCG
    
    // Then, we render our actors onto our render FBO, setting position and color based on the respective textures
    
    vector<float> pos(numParticles * 3);
    vector<float> colors(numParticles * 3);
    vector<float> vel(numParticles * 3);
    vector<float> life(numParticles * 3);
    vector<float> randtex(numParticles * 3);
        
    for (int i = 0; i < numParticles; i++) {
        float x = ofRandom(width);
        float y = ofRandom(height);
        
        pos[i * 3 + 0] = 0.1 + (x / float(width)) * 0.8;// pos.x
        pos[i * 3 + 1] = 0.1 + (y / float(height)) * 0.8;// pos.y
        pos[i * 3 + 2] = 0;                             // pos.z (unused)
        
        ofColor color = img.getColor(x, y) / 256.0;
        colors[i * 3 + 0] = color.r;                    // self-evident
        colors[i * 3 + 1] = color.g;
        colors[i * 3 + 2] = color.b;
        
        vel[i * 3 + 0] = ofRandom(PI * 2);              // vel.x -> direction
        vel[i * 3 + 1] = timeStep;                      // vel.y -> speed
        vel[i * 3 + 2] = 0;                             // vel.z (unused)

        if (i < Config::seed_particles) {
            float lifespan = ofRandom(Config::max_age) + Config::min_age;
            life[i * 3 + 0] = lifespan;                     // life.x -> lifespan
            life[i * 3 + 1] = 1.0;                          // life.y -> age
            life[i * 3 + 2] = 1.0;                          // life.z -> active (0 == false, 1 == true)
        } else {
            // inactive actor
            life[i * 3 + 0] = 0;
            life[i * 3 + 1] = 0.0;
            life[i * 3 + 1] = 0.0;
        }
        
        randtex[i * 3 + 0] = ofRandom(1.0);             // pseudorandom numbers to seed the LCG
        randtex[i * 3 + 1] = ofRandom(1.0);             // note that if we want reproducible results,
        randtex[i * 3 + 2] = ofRandom(1.0);             // we could seed these with predictable numbers
    }
    
    allocateAndLoad(posPingPong, pos);
    allocateAndLoad(colorPingPong, colors);
    allocateAndLoad(velPingPong, vel);
    allocateAndLoad(lifePingPong, life);
    allocateAndLoad(randPingPong, randtex);

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
    if (Config::seed_orig_image) {
        renderFBO.dst->getTexture().loadData(img.getPixels());
        renderFBO.src->getTexture().loadData(img.getPixels());
    }
}

void ofApp::allocateAndLoad(pingPongBuffer &buf, vector<float> &data) {
    buf.allocate(numParticlesSqrt, numParticlesSqrt, GL_RGB32F);
    buf.src->getTexture().loadData(data.data(), numParticlesSqrt, numParticlesSqrt, GL_RGB);
    buf.dst->getTexture().loadData(data.data(), numParticlesSqrt, numParticlesSqrt, GL_RGB);
}

//--------------------------------------------------------------
void ofApp::update() {
    lifePingPong.dst->begin();
    ofClear(0);
    updateLife.begin();
    updateLife.setUniformTexture("prevLifeData", lifePingPong.src->getTexture(), 0);
    updateLife.setUniformTexture("prevPosData", posPingPong.src->getTexture(), 1);
    updateLife.setUniformTexture("randomData", randPingPong.src->getTexture(), 2);
    lifePingPong.src->draw(0, 0);
    updateLife.end();
    lifePingPong.dst->end();
    lifePingPong.swap();
    
    posPingPong.dst->begin();
    ofClear(0);
    updatePos.begin();
    updatePos.setUniformTexture("prevPosData", posPingPong.src->getTexture(), 0);
    updatePos.setUniformTexture("velData", velPingPong.src->getTexture(), 1);
    updatePos.setUniformTexture("lifeData", lifePingPong.src->getTexture(), 2);
    updatePos.setUniformTexture("randomData", randPingPong.src->getTexture(), 3);
    updatePos.setUniform1f("timestep", timeStep);
    updatePos.setUniform1f("numParticlesSqrt", numParticlesSqrt);
    posPingPong.src->draw(0, 0);
    updatePos.end();
    posPingPong.dst->end();
    posPingPong.swap();
    
    ofFloatPixels pixels;
    lifePingPong.src->getTexture().readToPixels(pixels);
    
    ofFloatPixels positionPixels;
    posPingPong.src->getTexture().readToPixels(positionPixels);

    colorPingPong.dst->begin();
    updateColor.begin();
    updateColor.setUniformTexture("prevColorData", colorPingPong.src->getTexture(), 0);
    updateColor.setUniformTexture("posData", posPingPong.src->getTexture(), 1);
    updateColor.setUniformTexture("lifeData", lifePingPong.src->getTexture(), 2);
    updateColor.setUniformTexture("origImageData", img.getTexture(), 3);
    updateColor.setUniform2f("screen", (float)width, (float)height);
    colorPingPong.src->draw(0, 0);
    updateColor.end();
    colorPingPong.dst->end();
    colorPingPong.swap();

    velPingPong.dst->begin();
    ofClear(0);
    updateVel.begin();
    updateVel.setUniformTexture("velData", velPingPong.src->getTexture(), 0);
    updateVel.setUniformTexture("posData", posPingPong.src->getTexture(), 1);
    updateVel.setUniformTexture("colorData", colorPingPong.src->getTexture(), 2);
    updateVel.setUniformTexture("lifeData", lifePingPong.src->getTexture(), 3);
    updateVel.setUniformTexture("trailData", renderFBO.src->getTexture(), 4);
    updateVel.setUniformTexture("randomData", randPingPong.src->getTexture(), 5);
    updateVel.setUniform2f("screen", (float)width, (float)height);
    updateVel.setUniform1f("timestep", (float)timeStep);
    velPingPong.src->draw(0, 0);
    updateVel.end();
    velPingPong.dst->end();
    velPingPong.swap();
        
    ofFloatPixels randPixels;
    randPingPong.src->getTexture().readToPixels(randPixels);

    randPingPong.dst->begin();
    ofClear(0);
    updateRand.begin();
    updateRand.setUniformTexture("prevRandData", randPingPong.src->getTexture(), 0);
    randPingPong.src->draw(0, 0);
    updateRand.end();
    randPingPong.dst->end();
    randPingPong.swap();

    randPingPong.src->getTexture().readToPixels(randPixels);

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
    
    /*
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
     */
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
