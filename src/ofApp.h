#pragma once

#include "ofMain.h"
#include "ofxAssimpModelLoader.h"
#include "ofxGui.h"

struct block {
	ofVec3f position;
	int type;
};

struct deathEffect {
	float x, z;
	float radius = 0.4f;
	float speed = 0.2f;
	float opacity = 255; // full opacity (will be reduced over time of effect)
};

class GameObject {
public:
	GameObject();
	~GameObject();
	float x, y, z;
	float x_vel, y_vel, z_vel;
	float drag = 1.0;

	void updateObject(float minX, float maxX, float minZ, float maxZ);
};

class player :public GameObject {
public:
	player();
	~player();
	float size = 1.0f;
	float pAngle = 0.0f;
	float health = 10.0f;
	int score = 0;
	float lastAttack = 0.0f;  
	float coolDown = 1.0f;
	bool isAlive = true;
};


class Enemy : public GameObject {
public:
	Enemy();
	~Enemy();
	float size = 0.5f;
	float enemyRotY = 0.0f;
	int health = 2;
	float attRad = 10.0f;
	float lastAttack = 0.0f;
	float coolDown = 2.0f;
	float inRadius = 0.0f;

};

class ofApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();

		void keyPressed(int key);
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

		// create a function to spawn in enemies
		void spawnEnemy();

		// create a function to be called upon to attack an enemy as the player character
		void attack();

		// enemy function for attempting to attack
		void attempt(Enemy& enemy);

		// enemy attack function
		void enemyAttack();

		// game over function
		void gameOver();

		// new game function
		void newGameFunc();

		// models
		ofxAssimpModelLoader character;
		ofxAssimpModelLoader enemy;
		ofxAssimpModelLoader grassBlock;
		ofxAssimpModelLoader treeBlock;
		ofxAssimpModelLoader roadBlock;

		// enemies
		vector<Enemy> enemies;
		int enemyCount = 30; // there will always be 30 enemies in the game

		// death effect vector
		vector<deathEffect> effect;

		// tilemap
		vector<block> tiles;
		float treeOffset = 1.0f;
		static const int tileGridSize = 18;

		float minX;
		float maxX;
		float minZ;
		float maxZ;

		int tileMap[tileGridSize][tileGridSize] = {
			{2,0,0,0,2,0,2,0,0,1,1,1,1,1,1,1,1,1},
			{1,1,2,0,0,0,0,0,2,1,0,0,0,0,0,2,0,1},
			{0,1,1,1,1,2,0,0,0,1,0,2,0,0,0,0,0,1},
			{0,2,0,0,1,0,2,0,1,1,0,0,0,0,1,1,1,1},
			{0,0,0,0,1,0,0,0,1,0,2,0,0,2,1,0,0,1},
			{2,0,2,0,1,1,1,1,1,0,1,1,1,1,1,0,2,1},
			{0,0,0,0,1,0,0,2,0,2,1,0,0,0,0,0,0,1},
			{1,1,0,2,1,0,0,0,0,0,1,0,2,0,0,0,0,1},
			{2,1,0,0,1,0,2,0,0,2,1,1,0,0,0,2,0,1},
			{0,1,1,1,1,1,1,1,1,0,0,1,0,0,0,0,0,1},
			{0,1,2,0,2,0,0,0,1,0,0,1,2,0,2,0,0,1},
			{2,1,0,0,0,0,2,0,1,0,2,1,1,1,1,1,1,1},
			{0,1,0,2,0,0,0,0,1,0,0,1,0,0,0,2,0,0},
			{0,1,0,0,1,1,1,1,1,0,0,1,0,0,0,0,0,0},
			{2,1,0,0,1,0,0,0,0,2,0,1,0,2,0,0,0,2},
			{1,1,0,0,1,0,2,0,0,1,1,1,0,0,0,1,1,1},
			{0,0,0,2,1,0,0,0,2,1,0,0,2,0,0,1,2,0},
			{2,0,0,0,1,0,2,0,0,1,1,1,1,1,1,1,0,0},
		};

		// lighting and camera
		ofEasyCam cam;
		ofVec3f camTarget;
		ofLight light;
		ofLight light2;

		// character movement
		bool moveUp = false;
		bool moveDown = false;
		bool moveLeft = false;
		bool moveRight = false;

		// create an instance of the player class
		player player;

		// other variables
		float playerSpeed = 0.25f;

		float attackRadius = 10.0f;

		bool newGame = false;
		bool terminate = false;

		int flash = 0;

		ofTrueTypeFont scoreFont;
		ofTrueTypeFont enemyHealthFont;
		ofTrueTypeFont coolDownFont;
		ofTrueTypeFont restartFont;



};

