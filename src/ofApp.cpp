#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
	
	ofSetFrameRate(60);
	ofBackground(0, 0, 0);

	// load in the font for the UI
	scoreFont.load("cinzel.ttf", 32);
	enemyHealthFont.load("cinzel.ttf", 32);
	coolDownFont.load("cinzel.ttf", 32);
	restartFont.load("cinzel.ttf", 16);




	std::cout << "SETUP STARTED" << std::endl;
	//ofLogNotice() << "SETUP STARTED";

	// load the models
	bool loaded = character.loadModel("character.fbx");
	std::cout << "Loaded? " << loaded << std::endl;
	std::cout << "Has meshes? " << character.hasMeshes() << std::endl;
	enemy.loadModel("enemy.fbx");
	grassBlock.loadModel("grassBlock.fbx");
	treeBlock.loadModel("tree.fbx");
	roadBlock.loadModel("road.fbx");

	// terminal messages
	std::cout << "character max: " << character.getSceneMax() << std::endl;
	std::cout << "character min: " << character.getSceneMin() << std::endl;

	// scale the models to a reasonable size
	character.setScaleNormalization(false);
	enemy.setScaleNormalization(false);
	grassBlock.setScaleNormalization(false);
	treeBlock.setScaleNormalization(false);
	roadBlock.setScaleNormalization(false);
	character.setScale(0.025, 0.025, 0.025);
	enemy.setScale(0.025, 0.025, 0.025);
	grassBlock.setScale(0.1, 0.1, 0.1);
	treeBlock.setScale(0.1, 0.1, 0.1);
	roadBlock.setScale(0.1, 0.1, 0.1);


	// set positions of the models
	//character.setPosition(0,0,0);

	// enemy setup
	for (int l = 0; l < enemyCount; l++) {
		spawnEnemy();
	}
		

	// tilemap setup
	float tileMapSize = 20;
	for (int i = 0; i < tileGridSize; i++) {
		for (int j = 0; j < tileGridSize; j++) {
			block b;
			b.position = ofVec3f((i - tileGridSize / 2) * tileMapSize, 0, (j - tileGridSize / 2) * tileMapSize);
			b.type = tileMap[i][j];
			tiles.push_back(b);
		}
	}

	// calculate the boundaries of the tilemap
	minX = -(tileGridSize / 2.0f) * 20;
	maxX = (tileGridSize / 2.0f) * 20;
	minZ = -(tileGridSize / 2.0f) * 20;
	maxZ = (tileGridSize / 2.0f) * 20;


	// print diagnostics
	character.setRotation(0, 180, 1, 0, 0);


	// setup the camera at an angle
	cam.disableMouseInput();
	cam.setPosition(25, 30, 25);
	cam.lookAt(ofVec3f(0, 0, 0));
	camTarget = cam.getPosition();
	cam.setNearClip(0.1);       // allow very close objects
	cam.setFarClip(50000);      // allow very distant objects

	// set the light settings
	ofSetGlobalAmbientColor(ofColor(120, 120, 120));
	light.setPosition(500, 800, -800); // above + in front
	light2.setPosition(-500, 500, 500);
	light.setDiffuseColor(ofColor(255, 255, 255));
	light2.setDiffuseColor(ofColor(180, 180, 180));
	light.setSpecularColor(ofColor(0, 0, 0));
	light.enable();
	light2.enable();
	ofEnableLighting();

	// count meshes (for diagnostic check)
	std::cout << "Mesh count: " << character.getMeshCount() << std::endl;
	for (int i = 0; i < character.getMeshCount(); i++) {
		std::cout << "Mesh " << i << " vertex count: " << character.getMesh(i).getNumVertices() << std::endl;
	}
}

//--------------------------------------------------------------
void ofApp::update(){

	// allow to restart or escape if character is dead
	if (player.isAlive != true) {
		if (newGame == true) {
			newGameFunc();
			newGame = false;
		}
		else if (terminate == true) ofExit();
		return;
	}



	// update characher player
	float x_vel = 0.0f;
	float z_vel = 0.0f;

	// rotate character based on 4 cardinal movement directions
	if (moveUp == true) z_vel -= playerSpeed, player.pAngle = 180;
	if (moveDown == true) z_vel += playerSpeed, player.pAngle = 0;
	if (moveLeft == true) x_vel -= playerSpeed, player.pAngle = -90;
	if (moveRight == true) x_vel += playerSpeed, player.pAngle = 90;

	// rotate character based off diagonal movement directions
	if (moveUp == true && moveLeft == true) player.pAngle = -135;
	if (moveUp == true && moveRight == true) player.pAngle = 135;
	if (moveDown == true && moveLeft == true) player.pAngle = -45;
	if (moveDown == true && moveRight == true) player.pAngle = 45;

	// update position based on velocity
	player.x += x_vel;
	player.z += z_vel;

	// update enemies
	for (auto& e : enemies) {
		e.updateObject(minX, maxX, minZ, maxZ);
		float dist = ofDist(player.x, player.z, e.x, e.z);
		bool inRange = (dist < e.attRad);

		// track how long the player is within the enemy's attack radius
		if (inRange == true) {
			e.inRadius += ofGetLastFrameTime();
		}
		// if the player leave the attack redius then reset the timer to 0
		else {
			e.inRadius = 0.0f;
		}
		
		// enemy attacks player
		float current = ofGetElapsedTimef();
		// guaranteed hit
		if (inRange == true && e.inRadius >= 3.0f) {
			enemyAttack();
			e.inRadius = 0.0f;
			e.lastAttack = current;
			continue;
		}
		// non-guaranteed hit
		if (current - e.lastAttack >= e.coolDown) {
			e.lastAttack = current;
			if (inRange == true) attempt(e);
		}
	}

	// update death effect
	for (int p = effect.size() - 1; p >= 0; p--) {
		effect[p].radius += effect[p].speed;
		effect[p].opacity -= 4;

		if (effect[p].opacity <= 0) {
			effect.erase(effect.begin() + p);
		}
	}


	// allow the camera to be locked into following the player
	ofVec3f pPosition(player.x, 0, player.z);
	ofVec3f camOffset (25, 30, 25);
	camTarget.interpolate(pPosition + camOffset, 0.15);
	cam.setPosition(camTarget);
	cam.lookAt(pPosition);
	
}

//--------------------------------------------------------------
void ofApp::draw(){

	ofEnableDepthTest();

	// draw the light
	ofEnableLighting();
	light.enable();
	light2.enable();

	// draw the camera
	cam.begin();

	// draw models
	if (player.isAlive == true) {
		character.setPosition(player.x, 0, player.z);
		character.setRotation(1, player.pAngle, 0, 1, 0);
		character.drawFaces();
	}


	// draw the text of the player's cool down
	float time = ofGetElapsedTimef();
	float remaining = player.coolDown - (time - player.lastAttack);
	string twoDP = ofToString(remaining,3);
	
	if (remaining > 0) {
		ofPushMatrix();
		ofTranslate(player.x, 12, player.z);
		ofScale(0.025, 0.025, 0.025);
		ofSetColor(255);
		coolDownFont.drawString(ofToString(twoDP), -10, 0);
		ofPopMatrix();
	}

	// draw enemies in their random positions
	for (auto& e : enemies) {
		enemy.setPosition(e.x, 0, e.z);
		enemy.setRotation(0, 180, 1, 0, 0);
		enemy.setRotation(1, e.enemyRotY, 0, 1, 0);
		enemy.drawFaces();

		// draw each enemy's health on top of their head as a number of hits before death
		ofPushMatrix();
		ofTranslate(e.x, 10, e.z);
		ofScale(0.05, 0.05, 0.05);
		ofSetColor(255);
		enemyHealthFont.drawString(ofToString(e.health), 2, 0);
		ofPopMatrix();
	}

	// draw tiles
	for (auto& b : tiles) {
		switch (b.type) {
			case 0: // grass block
				grassBlock.setPosition(b.position.x, b.position.y, b.position.z);
				grassBlock.setRotation(0, 180, 1, 0, 0);
				grassBlock.drawFaces();
				break;
			case 1: // road block
				roadBlock.setPosition(b.position.x, b.position.y, b.position.z);
				roadBlock.setRotation(0, 180, 1, 0, 0);
				roadBlock.drawFaces();
				break;
			case 2: // tree block (but must place a grass block underneath it)
				grassBlock.setPosition(b.position.x, b.position.y, b.position.z);
				grassBlock.setRotation(0, 180, 1, 0, 0);
				grassBlock.drawFaces();

				treeBlock.setPosition(b.position.x, b.position.y + treeOffset, b.position.z);
				treeBlock.setRotation(0, 180, 1, 0, 0);
				treeBlock.drawFaces();
				break;
		}
	}

	// draw death effect as a red "smoke ring"
	for (auto& d : effect) {
		ofPushMatrix();
		ofTranslate(d.x, 0.5, d.z);
		ofSetColor(180, 20, 20,d.opacity); 
		ofNoFill();
		ofSetLineWidth(10);
		ofRotateXDeg(90);
		ofDrawCircle(0, 0, d.radius);
		ofPopMatrix();
	}

	cam.end();

	// draw score and difficulty tier
	ofSetColor(255);
	scoreFont.drawString("Score: " + ofToString(player.score), 10, 40);
	ofSetColor(255);

	// draw health bar
	float healthPercent = player.health / 10.0f;
	float width = ofGetWidth() * 0.75;
	float height = 25.0f;	
	// current health part of the bar
	ofSetColor(40, 120, 255);
	ofDrawRectangle((ofGetWidth() - width) / 2, (ofGetHeight() - height - 20), width * healthPercent, height);
	// full bar
	ofSetColor(200, 40, 40);
	ofDrawRectangle((ofGetWidth() - width) / 2, (ofGetHeight() - height - 20), width, height);
	// bar outline
	ofNoFill();
	ofSetColor(255);
	ofSetLineWidth(5);
	ofDrawRectangle((ofGetWidth() - width) / 2, (ofGetHeight() - height - 20), width, height);
	ofFill();

	// red flash to indicate getting hit
	if (flash > 0) {
		ofSetColor(255, 0, 0, flash);
		ofDrawRectangle(0, 0, ofGetWidth(), ofGetHeight());
		flash -= 5;
	}

	// difficulty pop up
	/*if (difficulty > prevDifficulty) {
		prevDifficulty = difficulty;
	}*/

	// disable lights
	light.disable();
	light2.disable();
	ofDisableLighting();

	ofDisableDepthTest();

	// draw gameOver message and restart instructions
	if (player.isAlive == false) {
		// create grey transparent screen overlay
		ofSetColor(0, 0, 0, 80);
		ofDrawRectangle(0, 0, ofGetWidth(), ofGetHeight());
		// draw text
		ofSetColor(255);
		scoreFont.drawString("GAME OVER!", ofGetWidth() / 2 - 100, ofGetHeight() / 2 - 120);
		scoreFont.drawString("Score: " + ofToString(player.score), ofGetWidth() / 2 - 50, ofGetHeight() / 2 -60);
		restartFont.drawString("Press 'r' to restart or 'esc' to quit", ofGetWidth() / 2 - 150, ofGetHeight() / 2 + 20);
		return;
	}

}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

	// set movement booleans based on key presses

	if (key == 'w') moveUp = true;
	if (key == 's') moveDown = true;
	if (key == 'a') moveLeft = true;
	if (key == 'd') moveRight = true;

	// call the attck function when spacebar is pressed(with cool down times)
	if (key == ' ') {
		float time = ofGetElapsedTimef();
		if (time - player.lastAttack >= player.coolDown) {
			attack();
			player.lastAttack = time;
		}
	}

	// press 'r' to restart the game
	if (key == 'r') newGame = true;

	// pres 'esc' to terminate the game
	if (key == OF_KEY_ESC) terminate = true;

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

	// unset movement booleans based on key releases
	if (key == 'w') moveUp = false;
	if (key == 's') moveDown = false;
	if (key == 'a') moveLeft = false;
	if (key == 'd') moveRight = false;
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

GameObject::GameObject() {
}

GameObject::~GameObject() {
}

void GameObject::updateObject(float minX, float maxX, float minZ, float maxZ) {
	x += x_vel;
	z += z_vel;

	if (x < minX) { x = minX; x_vel *= -1; }
	if (x > maxX) { x = maxX; x_vel *= -1; }
	if (z < minZ) { z = minZ; z_vel *= -1; }
	if (z > maxZ) { z = maxZ; z_vel *= -1; }
}

void ofApp::spawnEnemy() {
	while (true) {

		int enemyX = ofRandom(0, tileGridSize);
		int enemyZ = ofRandom(0, tileGridSize);

		if (tileMap[enemyX][enemyZ] != 0) continue; // don't place enemies on road or tree tiles
		Enemy E;

		E.enemyRotY = ofRandom(0, 360);
		E.inRadius = 0.0f;
		E.lastAttack = ofGetElapsedTimef();
		E.coolDown = 2.0f;

		float posX = (enemyX - tileGridSize / 2) * 20;
		float posZ = (enemyZ - tileGridSize / 2) * 20;

		E.x = posX;
		E.z = posZ;

		float angle = ofRandom(TWO_PI);
		float speed = 0.15f;

		E.x_vel = cos(angle) * speed;
		E.z_vel = sin(angle) * speed;

		enemies.push_back(E);
		break;
	}
}

void ofApp::attack() {
	// check to see if any enemy is within the attack radius when the function is called
	for (int k = enemies.size() - 1; k >= 0; k--) {
		float distance = ofDist(player.x, player.z, enemies[k].x, enemies[k].z);
		// it takes 2 hits to kill an enemy so the function needs to check the current health and minus 1 if a hit is successful
		if (distance < attackRadius) {
			enemies[k].health--;
		}
		if (enemies[k].health <= 0) {
			// spawn effect on death of enemy
			deathEffect e;
			e.x = enemies[k].x;
			e.z = enemies[k].z;
			effect.push_back(e);
			// erase enemy and add to score
			enemies.erase(enemies.begin() + k);
			player.score += 100;
			// spawn a new enemy once one dies to ensure there are always the same amount of enemies
			spawnEnemy(); 
		}
	}
}

void ofApp::attempt(Enemy& enemy){
/* Enemy attack rules:
If the player is within the attack radius of an enemy for 3 or more seconds then there is a guaranteed attack
However if it is less thn 3 seconds then the probability decreases
0 seconds = 0% chance
1 second = 33% chance
2 seconds = 66% chance
3 seconds = 100% chance
*/
	if (enemy.inRadius >= 3.0f) {
		enemyAttack();
		return;
	}
	
	float chance = (enemy.inRadius) / 3.0f;
	if (ofRandom(1.0f) < chance) enemyAttack();

}

void ofApp::enemyAttack() {
	// reduce player's health upon being attacked by an enemy
	player.health--;
	flash = 80;
	
	// erase player if health reaches 0 (game over)
	if (player.health <= 0) {
		player.isAlive = false;
		gameOver();
	}
}

void ofApp::gameOver() {
	// remove all enemies from the game
	enemies.clear();

}

void ofApp::newGameFunc() {
	// reset player stats and spawn in new enemies
	player.isAlive = true;
	player.health = 10;
	player.score = 0;
	// enemy setup
	enemies.clear();
	for (int l = 0; l < enemyCount; l++) {
		spawnEnemy();
		
	}

}


player::player() {}
player::~player() {}

Enemy::Enemy() {}
Enemy::~Enemy() {}





