#include "ofApp.h"
#include <algorithm>

class Config {
public:
    static constexpr bool seed_orig_image = true;
    static constexpr bool seed_orig_image_for_food = false;
    static const int seed_particles = 1024 * 1024;
    
    static constexpr int min_color_vector_length = 16;
    static constexpr int pixel_step = 1;
    static constexpr float direction_offset = PI;
    
    static constexpr int num_particles_sqrt = 2048;
    static constexpr float time_step_multiplier = 0.9;

    static constexpr char *imgName = "/Volumes/fast-external/new-photos-to-mold/aspen-dark-2.jpg";
    
    static constexpr float min_age = 32;
    static constexpr float max_age = 256;
    
    static constexpr float seed_particle_x = 0; //0.459; //0.664; //0.507; //0.493;;
    static constexpr float seed_particle_y = 0; //0.782; //0.365; //0.351; //0.826;;
    
    static constexpr bool radial_fill = false;
    static constexpr float radial_fill_radius = 0.03;
    static constexpr float radial_fill_center_x = 0.65;
    static constexpr float radial_fill_center_y = 0.7;
    
    static constexpr float border = 0.01;
    
    static constexpr float offset_x = 0.5;
    static constexpr float offset_y = 0.5;
    
    static constexpr bool perform_total_refresh = false;
    static const int total_refresh_rate = 2048;

    static constexpr bool save_roll = false;
    static constexpr char *frame_dir = "/Volumes/fast-external/video-frames/lit-trees";
    static constexpr int frame_increment = 10;
    static constexpr int max_frames = 64 * 32;
    static constexpr char *file_prefix = "first";
    
    static constexpr bool save_mask = true;
    static constexpr int mask_increment = 100;
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
    
    showFood = false;
    
    string shadersFolder;
    shadersFolder="shaders_gl3";
    
    // shaders that use combinations of textures to update actor state
    updatePos.load(shadersFolder + "/passthru.vert", shadersFolder + "/posUpdate.frag");
    updateColor.load(shadersFolder + "/passthru.vert", shadersFolder + "/colorUpdate.frag");
    updateVel.load(shadersFolder + "/passthru.vert", shadersFolder + "/velUpdate.frag");
    updateLife.load(shadersFolder + "/passthru.vert", shadersFolder + "/lifeUpdate.frag");
    updateRand.load(shadersFolder + "/passthru.vert", shadersFolder + "/randUpdate.frag");

    // shader that's applied to the render pingpong to blur and fade
    updateBlur.load(shadersFolder + "/passthru.vert", shadersFolder + "/renderBlur.frag");
    
    // shader that's applied to the food pingpong to blur and fade
    updateFoodBlur.load(shadersFolder + "/passthru.vert", shadersFolder + "/renderFoodBlur.frag");
    
    updateRender.setGeometryInputType(GL_POINTS);
	updateRender.setGeometryOutputType(GL_POINTS);
	updateRender.setGeometryOutputCount(1);
    updateRender.load(shadersFolder+"/render.vert", shadersFolder+"/render.frag", shadersFolder+"/render.geom");
    
    updateFood.setGeometryInputType(GL_POINTS);
    updateFood.setGeometryOutputType(GL_POINTS);
    updateFood.setGeometryOutputCount(1);
    updateFood.load(shadersFolder+"/renderFood.vert", shadersFolder+"/renderFood.frag", shadersFolder+"/renderFood.geom");

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
    
    initializeBoard();
    
    // Allocate the final
    boardPingPong.allocate(width, height, GL_RGB32F);
    boardPingPong.dst->begin();
    // Seed the render buffer with the original image.
    if (Config::seed_orig_image) {
        boardPingPong.dst->getTexture().loadData(img.getPixels());
        boardPingPong.src->getTexture().loadData(img.getPixels());
    } else {
        ofClear(0, 0, 0, 255);
    }
    boardPingPong.dst->end();
    
    foodPingPong.allocate(width, height, GL_RGB32F);
    for (int i = 0; i < 2; i++) {
        foodPingPong.dst->begin();
        if (Config::seed_orig_image_for_food) {
            foodPingPong.dst->getTexture().loadData(img.getPixels());
        } else {
            ofClear(255, 255, 255, 255);
        }
        foodPingPong.dst->end();
        if (i == 0) {
            foodPingPong.swap();
        }
    }

    mesh.setMode(OF_PRIMITIVE_POINTS);
    for(int x = 0; x < numParticlesSqrt; x++){
        for(int y = 0; y < numParticlesSqrt; y++){
            mesh.addVertex({x, y, 0});
            mesh.addTexCoord({x, y});
        }
    }

    for (int i = 0; i < 2; i++) {
        boardPingPong.dst->begin();
        updateBlur.begin();
        updateBlur.setUniformTexture("image", boardPingPong.src->getTexture(), 0);
        updateBlur.setUniform1i("horizontal", i == 0);
        updateBlur.setUniform2f("screen", (float)width, (float)height);
        boardPingPong.src->draw(0,0);
        updateBlur.end();
        boardPingPong.dst->end();
        boardPingPong.swap();

        foodPingPong.dst->begin();
        updateFoodBlur.begin();
        updateFoodBlur.setUniformTexture("image", boardPingPong.src->getTexture(), 0);
        updateFoodBlur.setUniform1i("horizontal", i == 0);
        updateFoodBlur.setUniform2f("screen", (float)width, (float)height);
        foodPingPong.src->draw(0,0);
        updateFoodBlur.end();
        foodPingPong.dst->end();
        foodPingPong.swap();
    }
}

void ofApp::initializeBoard(int x, int y) {
    float offset_x;
    float offset_y;

    if (x != -1 && y != -1) {
        offset_x = float(x) / float(window_width);
        offset_y = float(y) / float(window_height);
    } else if (Config::radial_fill) {
        offset_x = Config::radial_fill_center_x;
        offset_y = Config::radial_fill_center_y;
    } else {
        offset_x = Config::offset_x;
        offset_y = Config::offset_y;
    }

    
    vector<float> pos(numParticles * 3);
    vector<float> colors(numParticles * 3);
    vector<float> vel(numParticles * 3);
    vector<float> life(numParticles * 3);
    vector<float> randtex(numParticles * 3);
        
    for (int i = 0; i < numParticles; i++) {
        float x, y;

        float dir;
        if (Config::radial_fill) {
            float t = ofRandom(PI * 2);
            float r = ofRandom(Config::radial_fill_radius);
            x = offset_x + cos(t) * r;
            y = offset_y + sin(t) * r / (float(height) / float(width));
            dir = ofRandom(PI * 2);
        } else {
            if (Config::seed_particle_x != 0 && Config::seed_particle_y != 0) {
                x = Config::seed_particle_x * width;
                y = Config::seed_particle_y * height;
            } else {
                x = ofRandom(width);
                y = ofRandom(height);
            }
            
            x = Config::border + (x / float(width)) * (1 - Config::border * 2) + (offset_x - 0.5);
            y = Config::border + (y / float(height)) * (1 - Config::border * 2) + (offset_y - 0.5);
            
            dir = ofRandom(PI * 2);
        }

        pos[i * 3 + 0] = x;
        pos[i * 3 + 1] = y;
        pos[i * 3 + 2] = 0;                             // pos.z (unused)

        vel[i * 3 + 0] = dir;                           // vel.x -> direction
        vel[i * 3 + 1] = timeStep;                      // vel.y -> speed
        vel[i * 3 + 2] = 0;                             // vel.z (unused)

        ofFloatColor color = img.getColor(max(0, min(int(x * width), width - 1)), max(0, min(int(y * height), height - 1)));
        colors[i * 3 + 0] = color.r;                    // self-evident
        colors[i * 3 + 1] = color.g;
        colors[i * 3 + 2] = color.b;

        if (i < Config::seed_particles) {
            float lifespan = ofRandom(Config::max_age) + Config::min_age;
            if (i == 0) {
                life[i * 3 + 0] = 4; //this is for debugging    // life.x -> lifespan
            } else {
                life[i * 3 + 0] = lifespan;                     // life.x -> lifespan
            }
            life[i * 3 + 1] = 2.0;                          // life.y -> age
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
}

void ofApp::allocateAndLoad(pingPongBuffer &buf, vector<float> &data) {
    buf.allocate(numParticlesSqrt, numParticlesSqrt, GL_RGB32F);
    loadData(buf, data);
}

void ofApp::loadData(pingPongBuffer &buf, vector<float> &data) {
    buf.src->getTexture().loadData(data.data(), numParticlesSqrt, numParticlesSqrt, GL_RGB);
    buf.dst->getTexture().loadData(data.data(), numParticlesSqrt, numParticlesSqrt, GL_RGB);
}

std::string gen_random(const int len) {
    static const char alphanum[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";
    std::string tmp_s;
    tmp_s.reserve(len);

    for (int i = 0; i < len; ++i) {
        tmp_s += alphanum[rand() % (sizeof(alphanum) - 1)];
    }
    
    return tmp_s;
}

//--------------------------------------------------------------
void ofApp::update() {
    lifePingPong.dst->begin();
    ofClear(0);
    updateLife.begin();
    updateLife.setUniformTexture("prevLifeData", lifePingPong.src->getTexture(), 0);
    updateLife.setUniformTexture("prevPosData", posPingPong.src->getTexture(), 1);
    updateLife.setUniformTexture("randomData", randPingPong.src->getTexture(), 2);
    updateLife.setUniformTexture("colorData", colorPingPong.src->getTexture(), 3);
    updateLife.setUniformTexture("foodData", foodPingPong.src->getTexture(), 4);
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

    colorPingPong.dst->begin();
    updateColor.begin();
    updateColor.setUniformTexture("prevColorData", colorPingPong.src->getTexture(), 0);
    updateColor.setUniformTexture("posData", posPingPong.src->getTexture(), 1);
    updateColor.setUniformTexture("origImageData", img.getTexture(), 2);
    updateColor.setUniformTexture("lifeData", lifePingPong.src->getTexture(), 3);
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
    updateVel.setUniformTexture("foodData", foodPingPong.src->getTexture(), 3);
    updateVel.setUniformTexture("lifeData", lifePingPong.src->getTexture(), 4);
    updateVel.setUniformTexture("trailData", boardPingPong.src->getTexture(), 5);
    updateVel.setUniformTexture("randomData", randPingPong.src->getTexture(), 6);
    updateVel.setUniform2f("screen", (float)width, (float)height);
    updateVel.setUniform1f("timestep", (float)timeStep);
    updatePos.setUniform1f("numParticlesSqrt", numParticlesSqrt);
    velPingPong.src->draw(0, 0);
    updateVel.end();
    velPingPong.dst->end();
    velPingPong.swap();
        
    randPingPong.dst->begin();
    ofClear(0);
    updateRand.begin();
    updateRand.setUniformTexture("prevRandData", randPingPong.src->getTexture(), 0);
    randPingPong.src->draw(0, 0);
    updateRand.end();
    randPingPong.dst->end();
    randPingPong.swap();
    
    // Blur it
    for (int i = 0; i < 2; i++) {
        boardPingPong.dst->begin();
        updateBlur.begin();
        updateBlur.setUniformTexture("image", boardPingPong.src->getTexture(), 0);
        updateBlur.setUniform1i("horizontal", i == 0);
        updateBlur.setUniform2f("screen", (float)width, (float)height);
        boardPingPong.src->draw(0,0);
        updateBlur.end();
        boardPingPong.dst->end();
        if (i == 0) {
            boardPingPong.swap();
        }
        
        foodPingPong.dst->begin();
        updateFoodBlur.begin();
        updateFoodBlur.setUniformTexture("image", boardPingPong.src->getTexture(), 0);
        updateFoodBlur.setUniform1i("horizontal", i == 0);
        updateFoodBlur.setUniform2f("screen", (float)width, (float)height);
        ofPushStyle();
        ofEnableBlendMode(OF_BLENDMODE_ALPHA);
        foodPingPong.src->draw(0,0);
        updateFoodBlur.end();
        foodPingPong.dst->end();
        if (i == 0) {
            foodPingPong.swap();
        }
        ofPopStyle();
    }

    // Update the food texture; this informs where actors go
    foodPingPong.dst->begin();
    updateFood.begin();
    updateFood.setUniformTexture("posTex", posPingPong.dst->getTexture(), 0);
    updateFood.setUniform2f("screen", (float)width, (float)height);

    ofPushStyle();
    ofEnableBlendMode(OF_BLENDMODE_ALPHA);
    ofSetColor(255);

    mesh.draw();
    
    ofDisableBlendMode();
    glEnd();
    
    updateFood.end();
    foodPingPong.dst->end();
    foodPingPong.swap();
    ofPopStyle();

    // Render the actual scene
    boardPingPong.dst->begin();
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
    boardPingPong.dst->end();
    boardPingPong.swap();
    ofPopStyle();
    
    static int step = 0;
    if (Config::save_roll) {
        if (step % Config::frame_increment == 0) {
            ofPixels pixels;
            boardPingPong.src->getTexture().readToPixels(pixels);
            ofImage img(pixels);
            std::stringstream ss;
            ss << Config::frame_dir << "/" << Config::file_prefix << "-";
            ss << std::setw(10) << std::setfill('0') << std::to_string(int(step / Config::frame_increment));
            ss << ".jpg";
            
            string fullname = ss.str();
            img.save(fullname);
        }
        
        if (Config::save_mask && step % Config::mask_increment == 0) {
            ofPixels pixels;
            foodPingPong.src->getTexture().readToPixels(pixels);
            ofImage img(pixels);
            std::stringstream ss;
            ss << Config::frame_dir << "/" << Config::file_prefix << "-mask-";
            ss << std::setw(10) << std::setfill('0') << std::to_string(int(step / Config::mask_increment));
            ss << ".jpg";
            
            string fullname = ss.str();
            img.save(fullname);
        }

        if (step > Config::max_frames * Config::frame_increment) {
            ofExit();
        }
    }
    step++;
    
    if (Config::perform_total_refresh && step % Config::total_refresh_rate == 0) {
        initializeBoard();
    }
}

//--------------------------------------------------------------
void ofApp::draw(){
    static int step = 0;
    if (step++ % 1 == 0) {
        ofBackground(0);
        
        ofSetColor(255,255,255);
        if (showFood) {
            foodPingPong.dst->draw(0,0, ofGetWindowWidth(), ofGetWindowHeight());
        } else {
            boardPingPong.dst->draw(0,0, ofGetWindowWidth(), ofGetWindowHeight());
        }
    }
    int active_cells = 0;
    int cells_in_bounds = 0;
    /*
    ofFloatPixels pixels;
    lifePingPong.src->getTexture().readToPixels(pixels);
    
    ofFloatPixels positionPixels;
    posPingPong.src->getTexture().readToPixels(positionPixels);
    
    for (int i = 0; i < numParticlesSqrt * numParticlesSqrt; i++) {
        int idx = i * 3;
        if (pixels[idx + 2] == 1.0) {
            active_cells++;
        }
        
        float x = positionPixels[idx];
        float y = positionPixels[idx + 1];
        if (x >= 0 && x < 1 && y > 0 && y <= 1) {
            cells_in_bounds++;
        }
    }
    */
    ofSetColor(255);
    ofDrawBitmapString("Fps: " + ofToString( ofGetFrameRate()) + ": " + ofToString(active_cells) + ", " + ofToString(cells_in_bounds) + " (" + ofToString(step) + ")", 15, 15);
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    if (key == ' ') {
        initializeBoard();
    } else if (key == 's') {
        string unique_id = gen_random(5);

        ofPixels pix;
        boardPingPong.dst->getTexture().readToPixels(pix);
        ofImage img(pix);
        std::string filename = "/Users/moishe/gen-images/s-";
        filename.append(unique_id);
        filename.append(".jpg");
        img.save(filename);
        
        foodPingPong.dst->getTexture().readToPixels(pix);
        ofImage imgFood(pix);
        filename = "/Users/moishe/gen-images/s-";
        filename.append(unique_id);
        filename.append("-mask.jpg");
        imgFood.save(filename);
    } else if (key == 'f') {
        showFood = !showFood;
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
    initializeBoard(x, y);
}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){
    window_width = w;
    window_height = h;
}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
