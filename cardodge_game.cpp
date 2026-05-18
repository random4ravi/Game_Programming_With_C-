#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <iostream>
#include <vector>
#include <ctime>
#include <algorithm>

using namespace std;
using namespace sf;

// ================= BASE CLASS =================
class Car {
protected:
    Sprite sprite;
    float speed;

public:
    virtual void update(float dt) = 0;

    void render(RenderWindow &window) {
        window.draw(sprite);
    }

    FloatRect getBounds() {
        return sprite.getGlobalBounds();
    }

    void setPosition(float x, float y) {
        sprite.setPosition(x, y);
    }
};

// ================= PLAYER =================
class PlayerCar : public Car {
private:
    float moveSpeed;
    float leftBound, rightBound;

public:
    PlayerCar(Texture &tex) {
        sprite.setTexture(tex);
        sprite.setScale(0.3f, 0.3f);

        moveSpeed = 400.f;
        leftBound = 120;
        rightBound = 480;

        sprite.setPosition(300, 650);
    }

    void update(float dt) override {
        if (Keyboard::isKeyPressed(Keyboard::Left))
            sprite.move(-moveSpeed * dt, 0);

        if (Keyboard::isKeyPressed(Keyboard::Right))
            sprite.move(moveSpeed * dt, 0);

        // Restrict movement
        if (sprite.getPosition().x < leftBound)
            sprite.setPosition(leftBound, sprite.getPosition().y);

        if (sprite.getPosition().x > rightBound)
            sprite.setPosition(rightBound, sprite.getPosition().y);
    }
};

// ================= ENEMY =================
class EnemyCar : public Car {
public:
    EnemyCar(Texture &tex, float spd, float x) {
        sprite.setTexture(tex);
        sprite.setScale(0.3f, 0.3f);

        speed = spd;
        sprite.setPosition(x, -120);
    }

    void update(float dt) override {
        sprite.move(0, speed * dt);
    }

    bool offScreen() {
        return sprite.getPosition().y > 800;
    }
};

// ================= MAIN =================
int main() {
    srand(time(0));

    RenderWindow window(VideoMode(600, 800), "Car Dodging Game");

    // ================= LOAD TEXTURES =================
    Texture playerTex;
    playerTex.loadFromFile("assets/WhiteCar.png");
    playerTex.setSmooth(true);

    vector<Texture> enemyTextures(5);
    enemyTextures[0].loadFromFile("assets/RedCar1.png");
    enemyTextures[1].loadFromFile("assets/RedCar2.png");
    enemyTextures[2].loadFromFile("assets/YellowCar1.png");
    enemyTextures[3].loadFromFile("assets/YellowCar2.png");
    enemyTextures[4].loadFromFile("assets/YellowCar3.png");

    for (auto &t : enemyTextures)
        t.setSmooth(true);

    // ================= LOAD FONT =================
    Font font;
    font.loadFromFile("assets/KOMIKAP_.ttf");

    // ================= LOAD SOUND =================
    SoundBuffer crashBuffer;
    crashBuffer.loadFromFile("assets/crash.wav");
    Sound crashSound(crashBuffer);

    Music bgMusic;
    bgMusic.openFromFile("assets/bg.wav");
    bgMusic.setLoop(true);
    bgMusic.play();

    // ================= OBJECTS =================
    PlayerCar player(playerTex);
    vector<EnemyCar> enemies;

    vector<float> lanes = {150, 300, 450};

    // ================= GAME STATE =================
    float spawnTimer = 0;
    float spawnDelay = 1.0f;
    float speedMultiplier = 1.0f;

    float score = 0;
    bool gameOver = false;
    bool paused = false;
    int difficulty = 2;

    Clock clock;

    // ================= UI =================
    Text scoreText, speedText, gameOverText, pauseText;

    scoreText.setFont(font);
    scoreText.setCharacterSize(24);
    scoreText.setPosition(10, 10);

    speedText.setFont(font);
    speedText.setCharacterSize(24);
    speedText.setPosition(400, 10);

    gameOverText.setFont(font);
    gameOverText.setCharacterSize(40);
    gameOverText.setString("GAME OVER\nPress R to Restart");
    gameOverText.setPosition(100, 300);

    pauseText.setFont(font);
    pauseText.setCharacterSize(40);
    pauseText.setString("PAUSED");
    pauseText.setPosition(200, 350);

    // ================= GAME LOOP =================
    while (window.isOpen()) {
        float dt = clock.restart().asSeconds();

        Event event;
        while (window.pollEvent(event)) {
            if (event.type == Event::Closed)
                window.close();
        }

        // Pause toggle (better control)
        static bool pPressed = false;
        if (Keyboard::isKeyPressed(Keyboard::P)) {
            if (!pPressed) paused = !paused;
            pPressed = true;
        } else pPressed = false;

        // Restart
        if (gameOver && Keyboard::isKeyPressed(Keyboard::R)) {
            enemies.clear();
            score = 0;
            speedMultiplier = 1.0f;
            gameOver = false;
        }
       if (Keyboard::isKeyPressed(Keyboard::Num1)) difficulty = 1;
if (Keyboard::isKeyPressed(Keyboard::Num2)) difficulty = 2;
if (Keyboard::isKeyPressed(Keyboard::Num3)) difficulty = 3;

        // ================= UPDATE =================
        if (!paused && !gameOver) {
            player.update(dt);

            spawnTimer += dt;

            // Dynamic difficulty
          spawnDelay -= 0.1f * dt;
if (spawnDelay < 0.3f) spawnDelay = 0.3f;
            speedMultiplier += 0.2f * dt;   // gradual increase
            if (spawnTimer > spawnDelay) {
                int texIndex = rand() % enemyTextures.size();
                float lane = 120 + rand() % 360;  
                float speed = (200 + rand() % 200) * speedMultiplier;

                enemies.push_back(EnemyCar(enemyTextures[texIndex], speed, lane));
                spawnTimer = 0;
            }

            // Update enemies
            for (auto &e : enemies)
                e.update(dt);

            // Remove off-screen
            enemies.erase(remove_if(enemies.begin(), enemies.end(),
                [](EnemyCar &e) { return e.offScreen(); }),
                enemies.end());

            // Collision
            for (auto &e : enemies) {
                if (player.getBounds().intersects(e.getBounds())) {
                    crashSound.play();
                    gameOver = true;
                }
            }

            // Score
            score += dt * 100;
        }

        // ================= UI UPDATE =================
        scoreText.setString("Score: " + to_string(score));
        speedText.setString("Speed: " + to_string((int)speedMultiplier));

        // ================= RENDER =================
        window.clear();

        // Road
        RectangleShape road(Vector2f(400, 800));
        road.setFillColor(Color(30, 30, 30));
        road.setPosition(100, 0);
        window.draw(road);

        // Lane lines
        for (int i = 0; i < 10; i++) {
            RectangleShape line(Vector2f(10, 40));
            line.setFillColor(Color::White);
            line.setPosition(295, i * 80);
            window.draw(line);
        }

        // Draw player
        player.render(window);

        // Draw enemies
        for (auto &e : enemies)
            e.render(window);

        // Draw UI
        window.draw(scoreText);
        window.draw(speedText);

        if (paused)
            window.draw(pauseText);

        if (gameOver)
            window.draw(gameOverText);

        window.display();
    }

    return 0;
}