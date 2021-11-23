#pragma once

#include "ofMain.h"

// Struct for doing PingPong quickly and easy
//
// Because on GPU you can't write over the texture that you are reading we are
// using to pair of ofFbo attached together on what we call pingPongBuffer
// Learn more about Ping-Pong at:
//
// http://www.seas.upenn.edu/~cis565/fbo.htm#setupgl4
//
struct pingPongBuffer {
public:
	void allocate( int _width, int _height, int _internalformat = GL_RGBA){
		// Allocate
		for(int i = 0; i < 2; i++){
			FBOs[i].allocate(_width, _height, _internalformat );
			FBOs[i].getTexture().setTextureMinMagFilter(GL_NEAREST, GL_NEAREST);
		}

		//Assign
		src = &FBOs[0];
		dst = &FBOs[1];

		// Clean
		clear();
	}
    
	void swap(){
		std::swap(src,dst);
	}

	void clear(){
		for(int i = 0; i < 2; i++){
			FBOs[i].begin();
			ofClear(0,255);
			FBOs[i].end();
		}
	}

	ofFbo& operator[]( int n ){ return FBOs[n];}
	ofFbo   *src;       // Source       ->  Ping
	ofFbo   *dst;       // Destination  ->  Pong

private:
	ofFbo   FBOs[2];    // Real addresses of ping/pong FBO«s
};


class ofApp : public ofBaseApp{
public:
	void setup();
	void update();
	void draw();

	void keyPressed  (int key);
	void keyReleased(int key);
	void mouseMoved(int x, int y );
	void mouseDragged(int x, int y, int button);
	void mousePressed(int x, int y, int button);
	void mouseReleased(int x, int y, int button);
	void mouseEntered(int x, int y);
	void mouseExited(int x, int y);
	void windowResized(int w, int h);
	void dragEvent(ofDragInfo dragInfo);
	void gotMessage(ofMessage msg);

    ofShader    updateLife;
	ofShader    updatePos;
    ofShader    updateColor;
	ofShader    updateVel;
    ofShader    updateRand;

    pingPongBuffer lifePingPong;
	pingPongBuffer posPingPong;
    pingPongBuffer colorPingPong;
	pingPongBuffer velPingPong;
    pingPongBuffer randPingPong;

    ofShader    updateBlur;
    ofShader    updateRender;

	pingPongBuffer renderFBO;
    
    ofImage img;
    
    string random_id;
    string filename;

	float   timeStep;

	int     width, height;
    int     numParticlesSqrt;
	int     numParticles;

	ofVboMesh mesh;
    
private:
    void allocateAndLoad(pingPongBuffer &buf, vector<float> &data);
};
