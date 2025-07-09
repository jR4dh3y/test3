#include <emscripten/emscripten.h>
#include <emscripten/bind.h>
#include <emscripten/html5.h>
#include <vector>
#include <cmath>
#include <random>
#include <algorithm>

struct Vector3 {
    float x, y, z;
    
    Vector3() : x(0), y(0), z(0) {}
    Vector3(float x, float y, float z) : x(x), y(y), z(z) {}
    
    Vector3 operator+(const Vector3& other) const {
        return Vector3(x + other.x, y + other.y, z + other.z);
    }
    
    Vector3 operator-(const Vector3& other) const {
        return Vector3(x - other.x, y - other.y, z - other.z);
    }
    
    Vector3 operator*(float scalar) const {
        return Vector3(x * scalar, y * scalar, z * scalar);
    }
    
    float length() const {
        return std::sqrt(x * x + y * y + z * z);
    }
    
    Vector3 normalize() const {
        float len = length();
        if (len == 0) return Vector3();
        return Vector3(x / len, y / len, z / len);
    }
    
    float distanceTo(const Vector3& other) const {
        return (*this - other).length();
    }
};

struct Enemy {
    Vector3 position;
    int health;
    bool alive;
    
    Enemy() : position(0, 1, 0), health(3), alive(true) {}
    Enemy(Vector3 pos) : position(pos), health(3), alive(true) {}
};

struct Bullet {
    Vector3 position;
    Vector3 direction;
    bool alive;
    
    Bullet() : position(0, 0, 0), direction(0, 0, -1), alive(true) {}
    Bullet(Vector3 pos, Vector3 dir) : position(pos), direction(dir), alive(true) {}
};

class Game {
private:
    Vector3 playerPos;
    Vector3 carPos;
    bool inCar;
    int score;
    float camAngleH;
    float camAngleV;
    float camDist;
    
    std::vector<Enemy> enemies;
    std::vector<Bullet> bullets;
    
    // Input state
    bool keys[256];
    
    std::mt19937 rng;
    
public:
    Game() : playerPos(0, 1, 0), carPos(10, 0.75, 0), inCar(false), score(0),
             camAngleH(0), camAngleV(0.4f), camDist(10), rng(std::random_device{}()) {
        
        // Initialize keys
        for (int i = 0; i < 256; i++) {
            keys[i] = false;
        }
        
        // Spawn initial enemies
        for (int i = 0; i < 3; i++) {
            spawnEnemy();
        }
    }
    
    void spawnEnemy() {
        std::uniform_real_distribution<float> dist(-20.0f, 20.0f);
        Vector3 pos(dist(rng), 1, dist(rng));
        enemies.push_back(Enemy(pos));
    }
    
    void setKey(int keyCode, bool pressed) {
        if (keyCode >= 0 && keyCode < 256) {
            keys[keyCode] = pressed;
        }
    }
    
    void updateCamera(float deltaH, float deltaV, float deltaZoom) {
        camAngleH -= deltaH * 0.005f;
        camAngleV -= deltaV * 0.005f;
        camAngleV = std::max(0.1f, std::min(1.4f, camAngleV)); // clamp
        
        camDist = std::max(5.0f, std::min(20.0f, camDist + deltaZoom * 0.01f));
    }
    
    void handleInteraction() {
        if (keys['e'] || keys['E']) {
            if (!inCar && playerPos.distanceTo(carPos) < 4) {
                inCar = true;
            } else if (inCar) {
                inCar = false;
            }
            keys['e'] = keys['E'] = false;
        }
    }
    
    void updatePlayerMovement() {
        const float speed = 0.2f;
        Vector3 move(0, 0, 0);
        
        if (keys['w'] || keys['W']) move.z -= 1;
        if (keys['s'] || keys['S']) move.z += 1;
        if (keys['a'] || keys['A']) move.x -= 1;
        if (keys['d'] || keys['D']) move.x += 1;
        
        if (move.length() > 0) {
            move = move.normalize();
            
            // Apply camera rotation
            float newX = move.x * std::cos(camAngleH) - move.z * std::sin(camAngleH);
            float newZ = move.x * std::sin(camAngleH) + move.z * std::cos(camAngleH);
            
            playerPos.x += newX * speed;
            playerPos.z += newZ * speed;
        }
    }
    
    void updateCarMovement() {
        const float speed = 0.3f;
        if (keys['w'] || keys['W']) carPos.z -= speed;
        if (keys['s'] || keys['S']) carPos.z += speed;
        if (keys['a'] || keys['A']) carPos.x -= speed;
        if (keys['d'] || keys['D']) carPos.x += speed;
    }
    
    void shoot() {
        if (!inCar) {
            Vector3 dir(0, 0, -1);
            // Apply player rotation based on camera angle
            float newX = dir.x * std::cos(camAngleH) - dir.z * std::sin(camAngleH);
            float newZ = dir.x * std::sin(camAngleH) + dir.z * std::cos(camAngleH);
            dir.x = newX;
            dir.z = newZ;
            
            bullets.push_back(Bullet(playerPos, dir));
        }
    }
    
    void updateBullets() {
        const float bulletSpeed = 0.5f;
        
        // Update bullet positions
        for (auto& bullet : bullets) {
            if (bullet.alive) {
                bullet.position = bullet.position + bullet.direction * bulletSpeed;
            }
        }
        
        // Check collisions
        for (auto& bullet : bullets) {
            if (!bullet.alive) continue;
            
            for (auto& enemy : enemies) {
                if (!enemy.alive) continue;
                
                if (enemy.position.distanceTo(bullet.position) < 1.0f) {
                    enemy.health--;
                    bullet.alive = false;
                    
                    if (enemy.health <= 0) {
                        enemy.alive = false;
                        spawnEnemy();
                        score++;
                    }
                    break;
                }
            }
        }
        
        // Remove dead bullets
        bullets.erase(std::remove_if(bullets.begin(), bullets.end(), 
                      [](const Bullet& b) { return !b.alive; }), bullets.end());
        
        // Remove dead enemies
        enemies.erase(std::remove_if(enemies.begin(), enemies.end(), 
                      [](const Enemy& e) { return !e.alive; }), enemies.end());
    }
    
    void update() {
        handleInteraction();
        
        if (inCar) {
            updateCarMovement();
        } else {
            updatePlayerMovement();
        }
        
        updateBullets();
    }
    
    // Getters for JavaScript
    float getPlayerX() const { return playerPos.x; }
    float getPlayerY() const { return playerPos.y; }
    float getPlayerZ() const { return playerPos.z; }
    
    float getCarX() const { return carPos.x; }
    float getCarY() const { return carPos.y; }
    float getCarZ() const { return carPos.z; }
    
    bool getInCar() const { return inCar; }
    int getScore() const { return score; }
    
    float getCamAngleH() const { return camAngleH; }
    float getCamAngleV() const { return camAngleV; }
    float getCamDist() const { return camDist; }
    
    int getEnemyCount() const { return enemies.size(); }
    float getEnemyX(int index) const { return enemies[index].position.x; }
    float getEnemyY(int index) const { return enemies[index].position.y; }
    float getEnemyZ(int index) const { return enemies[index].position.z; }
    
    int getBulletCount() const { return bullets.size(); }
    float getBulletX(int index) const { return bullets[index].position.x; }
    float getBulletY(int index) const { return bullets[index].position.y; }
    float getBulletZ(int index) const { return bullets[index].position.z; }
};

// Global game instance
Game* gameInstance = nullptr;

extern "C" {
    EMSCRIPTEN_KEEPALIVE
    void initGame() {
        gameInstance = new Game();
    }
    
    EMSCRIPTEN_KEEPALIVE
    void setKey(int keyCode, int pressed) {
        if (gameInstance) {
            gameInstance->setKey(keyCode, pressed != 0);
        }
    }
    
    EMSCRIPTEN_KEEPALIVE
    void updateCamera(float deltaH, float deltaV, float deltaZoom) {
        if (gameInstance) {
            gameInstance->updateCamera(deltaH, deltaV, deltaZoom);
        }
    }
    
    EMSCRIPTEN_KEEPALIVE
    void shoot() {
        if (gameInstance) {
            gameInstance->shoot();
        }
    }
    
    EMSCRIPTEN_KEEPALIVE
    void updateGame() {
        if (gameInstance) {
            gameInstance->update();
        }
    }
    
    EMSCRIPTEN_KEEPALIVE
    float getPlayerX() { return gameInstance ? gameInstance->getPlayerX() : 0; }
    EMSCRIPTEN_KEEPALIVE
    float getPlayerY() { return gameInstance ? gameInstance->getPlayerY() : 0; }
    EMSCRIPTEN_KEEPALIVE
    float getPlayerZ() { return gameInstance ? gameInstance->getPlayerZ() : 0; }
    
    EMSCRIPTEN_KEEPALIVE
    float getCarX() { return gameInstance ? gameInstance->getCarX() : 0; }
    EMSCRIPTEN_KEEPALIVE
    float getCarY() { return gameInstance ? gameInstance->getCarY() : 0; }
    EMSCRIPTEN_KEEPALIVE
    float getCarZ() { return gameInstance ? gameInstance->getCarZ() : 0; }
    
    EMSCRIPTEN_KEEPALIVE
    int getInCar() { return gameInstance ? (gameInstance->getInCar() ? 1 : 0) : 0; }
    EMSCRIPTEN_KEEPALIVE
    int getScore() { return gameInstance ? gameInstance->getScore() : 0; }
    
    EMSCRIPTEN_KEEPALIVE
    float getCamAngleH() { return gameInstance ? gameInstance->getCamAngleH() : 0; }
    EMSCRIPTEN_KEEPALIVE
    float getCamAngleV() { return gameInstance ? gameInstance->getCamAngleV() : 0; }
    EMSCRIPTEN_KEEPALIVE
    float getCamDist() { return gameInstance ? gameInstance->getCamDist() : 0; }
    
    EMSCRIPTEN_KEEPALIVE
    int getEnemyCount() { return gameInstance ? gameInstance->getEnemyCount() : 0; }
    EMSCRIPTEN_KEEPALIVE
    float getEnemyX(int index) { return gameInstance ? gameInstance->getEnemyX(index) : 0; }
    EMSCRIPTEN_KEEPALIVE
    float getEnemyY(int index) { return gameInstance ? gameInstance->getEnemyY(index) : 0; }
    EMSCRIPTEN_KEEPALIVE
    float getEnemyZ(int index) { return gameInstance ? gameInstance->getEnemyZ(index) : 0; }
    
    EMSCRIPTEN_KEEPALIVE
    int getBulletCount() { return gameInstance ? gameInstance->getBulletCount() : 0; }
    EMSCRIPTEN_KEEPALIVE
    float getBulletX(int index) { return gameInstance ? gameInstance->getBulletX(index) : 0; }
    EMSCRIPTEN_KEEPALIVE
    float getBulletY(int index) { return gameInstance ? gameInstance->getBulletY(index) : 0; }
    EMSCRIPTEN_KEEPALIVE
    float getBulletZ(int index) { return gameInstance ? gameInstance->getBulletZ(index) : 0; }
}
