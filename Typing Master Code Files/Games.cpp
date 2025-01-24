#include "Games.h"
#include <fstream>
#include <ctime>
#include <cmath>
#include <algorithm>
#include <map>
#include <sstream>

// Add user score tracking
std::map<std::string, int> userHighScores;
std::string currentUser;

FallingWordsGame::FallingWordsGame(const std::string& username) {
    currentUser = username;
    srand(static_cast<unsigned>(time(0)));
    InitializeTheme();
    LoadWordsFromFile("words.txt");
    LoadHighScores();
    ResetGame();
}

void FallingWordsGame::InitializeTheme() {
    // Black and white theme with modified colors
    backgroundColor = Color{ 20, 20, 20, 255 };     // Dark background
    primaryColor = WHITE;                           // Pure white
    secondaryColor = Color{ 200, 200, 200, 255 };  // Light gray
    accentColor = Color{ 150, 150, 150, 255 };     // Medium gray
    textColor = BLACK;                             // Changed text color to black
}
void FallingWordsGame::LoadHighScores() {
    std::ifstream file("highscores.txt");
    std::string line, username;
    int score;

    while (std::getline(file, line)) {
        std::istringstream iss(line);
        if (iss >> username >> score) {
            userHighScores[username] = score;
        }
    }

    // Set current user's high score
    auto it = userHighScores.find(currentUser);
    if (it != userHighScores.end()) {
        highScore = it->second;
    }
    else {
        highScore = 0;
    }
}

void FallingWordsGame::SaveHighScore() {
    if (score > highScore) {
        highScore = score;
        userHighScores[currentUser] = highScore;

        // Save all high scores
        std::ofstream file("highscores.txt");
        for (const auto& pair : userHighScores) {
            file << pair.first << " " << pair.second << "\n";
        }
    }
}

void FallingWordsGame::LoadWordsFromFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        // Add some default words if file can't be opened
        wordList = { "hello", "world", "game", "play", "type", "fast", "score", "win" };
        return;
    }

    std::string word;
    while (file >> word) {
        wordList.push_back(word);
    }
    file.close();

    if (wordList.empty()) {
        // Add default words if file was empty
        wordList = { "hello", "world", "game", "play", "type", "fast", "score", "win" };
    }
}

void FallingWordsGame::CreateCloudShape(FallingWord& word) {
    word.cloudPoints.clear();
    float baseRadius = 50.0f;

    for (int i = 0; i < CLOUD_POINTS; i++) {
        CloudPoint point;
        float angle = (float)i / CLOUD_POINTS * 2 * PI;
        float randRadius = baseRadius * (0.8f + (rand() % 40) / 100.0f);
        point.radius = baseRadius * 0.5f * (0.8f + (rand() % 40) / 100.0f);
        point.x = cosf(angle) * randRadius;
        point.y = sinf(angle) * randRadius * 0.7f;
        word.cloudPoints.push_back(point);
    }
}

void FallingWordsGame::SpawnWord() {
    if (wordList.empty()) return;

    FallingWord newWord;
    newWord.word = wordList[rand() % wordList.size()];
    newWord.typedPart = "";
    newWord.x = rand() % (GetScreenWidth() - 150) + 75;
    newWord.y = -50;
    newWord.speed = currentBaseSpeed + rand() % 100;
    newWord.isActive = false;
    newWord.scale = 1.0f;
    newWord.alpha = 1.0f;
    newWord.wobbleTime = 0;
    newWord.clickAnimScale = 1.0f;

    float hue = rand() % 360;
    Color baseColor = ColorFromHSV(hue, 0.5f, 0.95f);
    newWord.bubbleColor = ColorAlpha(baseColor, 0.9f);

    CreateCloudShape(newWord);
    fallingWords.push_back(newWord);
}

void FallingWordsGame::CreatePopEffect(float x, float y, Color color) {
    for (int i = 0; i < 20; i++) {
        Particle p;
        p.position = { x, y };
        float angle = (rand() % 360) * DEG2RAD;
        float speed = rand() % 200 + 100;
        p.velocity = {
            cosf(angle) * speed,
            sinf(angle) * speed
        };
        p.radius = rand() % 5 + 2;
        p.lifetime = 1.0f;
        p.color = color;
        particles.push_back(p);
    }
    screenShake = 0.3f;
}

void FallingWordsGame::UpdateParticles() {
    float dt = GetFrameTime();
    for (auto it = particles.begin(); it != particles.end();) {
        it->position.x += it->velocity.x * dt;
        it->position.y += it->velocity.y * dt;
        it->velocity.y += 500.0f * dt; // Gravity
        it->lifetime -= dt;

        if (it->lifetime <= 0) {
            it = particles.erase(it);
        }
        else {
            ++it;
        }
    }
}

void FallingWordsGame::UpdateCombo() {
    float dt = GetFrameTime();
    if (combo > 0) {
        comboTimer -= dt;
        if (comboTimer <= 0) {
            combo = 0;
        }
    }
}

void FallingWordsGame::HandleClick(int x, int y) {
    if (!isRunning || isPaused) return;

    for (auto& word : fallingWords) {
        float dx = x - word.x;
        float dy = y - word.y;
        float distance = sqrtf(dx * dx + dy * dy);

        if (distance < 50 && !word.isActive) {
            // Deactivate other words
            for (auto& otherword : fallingWords) {
                otherword.isActive = false;
                otherword.typedPart = "";
            }

            word.isActive = true;
            word.clickAnimScale = 1.2f;
            break;
        }
    }
}

void FallingWordsGame::UpdateWordTyping(char typed) {
    // First, check if any word is currently active
    auto activeWord = std::find_if(fallingWords.begin(), fallingWords.end(),
        [](const FallingWord& word) { return word.isActive; });

    if (activeWord != fallingWords.end()) {
        // If there's an active word, try to continue typing it
        if (activeWord->typedPart.length() < activeWord->word.length() &&
            typed == activeWord->word[activeWord->typedPart.length()]) {

            activeWord->typedPart += typed;

            // Word completed
            if (activeWord->typedPart == activeWord->word) {
                int pointsEarned = 100 * (combo + 1);
                score += pointsEarned;

                combo = std::min(combo + 1, MAX_COMBO);
                comboTimer = COMBO_TIME_LIMIT;

                CreatePopEffect(activeWord->x, activeWord->y, activeWord->bubbleColor);
                fallingWords.erase(activeWord);
            }
        }
    }
    else {
        // If no word is active, look for a word that starts with the typed character
        auto newActiveWord = std::find_if(fallingWords.begin(), fallingWords.end(),
            [typed](const FallingWord& word) {
                return !word.isActive && word.word[0] == typed;
            });

        if (newActiveWord != fallingWords.end()) {
            // Deactivate all other words
            for (auto& word : fallingWords) {
                word.isActive = false;
                word.typedPart = "";
            }

            // Activate the new word and add the first character
            newActiveWord->isActive = true;
            newActiveWord->typedPart = typed;
            newActiveWord->clickAnimScale = 1.2f;
        }
    }
}

void FallingWordsGame::UpdateGame() {
    if (!isRunning || isPaused) return;

    float dt = GetFrameTime();
    timer += dt;
    difficultyTimer += dt;

    if (screenShake > 0) {
        screenShake -= dt * 2;
        if (screenShake < 0) screenShake = 0;
    }

    // Handle keyboard input
    int key = GetCharPressed();
    while (key > 0) {
        UpdateWordTyping((char)key);
        key = GetCharPressed();
    }

    // Handle mouse input (optional)
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        Vector2 mousePos = GetMousePosition();
        HandleClick(mousePos.x, mousePos.y);
    }

    // Update falling words
    for (auto& word : fallingWords) {
        word.y += word.speed * dt;
        word.wobbleTime += dt;

        if (word.clickAnimScale > 1.0f) {
            word.clickAnimScale = std::max(1.0f, word.clickAnimScale - dt * 2);
        }

        if (word.y > GetScreenHeight() + 50) {
            lives--;
            CreatePopEffect(word.x, word.y, RED);

            if (lives <= 0) {
                isRunning = false;
                isGameOver = true;
                SaveHighScore();
            }
        }
    }

    fallingWords.erase(
        std::remove_if(fallingWords.begin(), fallingWords.end(),
            [](const FallingWord& word) { return word.y > GetScreenHeight() + 50; }),
        fallingWords.end()
    );

    if (timer >= spawnInterval) {
        timer = 0.0f;
        SpawnWord();
    }

    // Handle difficulty increase
    if (difficultyTimer >= DIFFICULTY_INCREASE_INTERVAL) {
        difficultyTimer = 0;

        // Increase spawn rate but don't go below minimum interval
        spawnInterval = std::max(MIN_SPAWN_INTERVAL, spawnInterval * SPAWN_INTERVAL_DECREASE);

        // Increase base speed but don't exceed maximum
        currentBaseSpeed = std::min(MAX_WORD_SPEED, currentBaseSpeed + SPEED_INCREMENT);
    }


    UpdateParticles();
    UpdateCombo();
}

// First modify the DrawGame() method in Games.cpp
void FallingWordsGame::DrawGame(bool beginEnd) {
    if (!isRunning && !isGameOver) return;  // Don't draw anything if game isn't active

    if (beginEnd) BeginDrawing();
    ClearBackground(backgroundColor);  // Always clear the entire screen first

    // Draw the game elements only if we're running or showing game over screen
    if (isGameOver) {
        // Game over screen with full screen background
        DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), backgroundColor);
        DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), ColorAlpha(BLACK, 0.7f));

        int centerX = GetScreenWidth() / 2;
        int centerY = GetScreenHeight() / 2;

        float frameWidth = 600;
        float frameHeight = 550;  // Increased from 500 to 580 to accommodate the exit button
        float pulseScale = 1.0f + sinf(GetTime() * 2) * 0.02f;

        // Outer frame
        DrawRectangleLines(
            centerX - (frameWidth / 2) * pulseScale,
            centerY - (frameHeight / 2) * pulseScale,
            frameWidth * pulseScale,
            frameHeight * pulseScale,
            primaryColor
        );

        // Inner frame
        DrawRectangleLines(
            centerX - (frameWidth / 2 - 10) * pulseScale,
            centerY - (frameHeight / 2 - 10) * pulseScale,
            (frameWidth - 20) * pulseScale,
            (frameHeight - 20) * pulseScale,
            secondaryColor
        );

        // Game Over text
        DrawText("GAME OVER",
            centerX - MeasureText("GAME OVER", 60) / 2,
            centerY - frameHeight / 2 + 50,
            60, primaryColor);

        std::string userText = "Player: " + currentUser;
        DrawText(userText.c_str(),
            centerX - MeasureText(userText.c_str(), 30) / 2,
            centerY - 80,
            30, secondaryColor);

        DrawText(TextFormat("Final Score: %d", score),
            centerX - MeasureText(TextFormat("Final Score: %d", score), 40) / 2,
            centerY-30,
            40, primaryColor);

        DrawText(TextFormat("Personal Best: %d", highScore),
            centerX - MeasureText(TextFormat("Personal Best: %d", highScore), 40) / 2,
            centerY + 20,
            40, accentColor);

        // Global high score display
        int globalHighScore = 0;
        std::string topPlayer;
        for (const auto& pair : userHighScores) {
            if (pair.second > globalHighScore) {
                globalHighScore = pair.second;
                topPlayer = pair.first;
            }
        }

        std::string globalBestText = "Global Best: " + std::to_string(globalHighScore) + " by " + topPlayer;
        DrawText(globalBestText.c_str(),
            centerX - MeasureText(globalBestText.c_str(), 20) / 2,
            centerY + 100,
            20, secondaryColor);

        Rectangle btnBounds = {
            centerX - 100,
            centerY + frameHeight / 2 - 120,
            200,
            40
        };

        Rectangle exitBounds = {
           centerX - 100,
           centerY + frameHeight / 2 - 70, // Position it below Play Again
           200,
           40
        };



        Color btnColor = primaryColor;
        if (CheckCollisionPointRec(GetMousePosition(), btnBounds)) {
            btnColor = secondaryColor;
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                StartGame();
            }
        }

        Color exitBtnColor = primaryColor;
        if (CheckCollisionPointRec(GetMousePosition(), exitBounds)) {
            exitBtnColor = secondaryColor;
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                isRunning = false;
                isGameOver = false;  // This will signal the main menu to take over
            }
        }

        DrawRectangleLinesEx(btnBounds, 2, btnColor);
        DrawText("PLAY AGAIN",
            centerX - MeasureText("PLAY AGAIN", 20) / 2,
            centerY + frameHeight / 2 - 110,
            20, btnColor);

        DrawRectangleLinesEx(exitBounds, 2, exitBtnColor);
        DrawText("EXIT TO MENU",
            centerX - MeasureText("EXIT TO MENU", 20) / 2,
            centerY + frameHeight / 2 - 60,
            20, exitBtnColor);
    }
    else {
        // Draw game background
        DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), backgroundColor);

        Vector2 shakeOffset = { 0, 0 };
        if (screenShake > 0) {
            shakeOffset.x = (rand() % 100 - 50) * screenShake * 0.1f;
            shakeOffset.y = (rand() % 100 - 50) * screenShake * 0.1f;
        }

        // Draw falling words
        for (const auto& word : fallingWords) {
            Vector2 pos = { word.x + shakeOffset.x, word.y + shakeOffset.y };

            // Draw cloud shape
            for (const auto& point : word.cloudPoints) {
                float wobble = sinf(word.wobbleTime * 2 + point.x * 0.1f) * 3;
                Vector2 cloudPos = {
                    pos.x + point.x * word.clickAnimScale,
                    pos.y + point.y * word.clickAnimScale + wobble
                };

                float alpha = word.isActive ? 0.9f : 0.7f;
                Color cloudColor = ColorAlpha(primaryColor, alpha);
                DrawCircleV(cloudPos, point.radius * word.clickAnimScale, cloudColor);
            }

            // Draw main cloud
            DrawCircleV(pos, 45 * word.clickAnimScale,
                ColorAlpha(primaryColor, word.isActive ? 1.0f : 0.8f));

            // Draw word text
            const char* fullWord = word.word.c_str();
            float textWidth = MeasureText(fullWord, 20);

            if (word.isActive) {
                Color typedColor = GREEN;
                DrawText(word.typedPart.c_str(),
                    pos.x - textWidth / 2,
                    pos.y - 10,
                    20,
                    typedColor);

                DrawText(fullWord + word.typedPart.length(),
                    pos.x - textWidth / 2 + MeasureText(word.typedPart.c_str(), 20),
                    pos.y - 10,
                    20,
                    textColor);
            }
            else {
                DrawText(fullWord,
                    pos.x - textWidth / 2,
                    pos.y - 10,
                    20,
                    textColor);
            }
        }

        // Draw particles
        for (const auto& p : particles) {
            Vector2 particlePos = { p.position.x + shakeOffset.x, p.position.y + shakeOffset.y };
            DrawCircleV(particlePos, p.radius, ColorAlpha(p.color, p.lifetime));
        }

        // Draw HUD
        DrawRectangle(0, 0, GetScreenWidth(), 60, ColorAlpha(BLACK, 0.8f));

        float scoreScale = 1.0f + sinf(GetTime() * 4) * 0.1f;
        DrawText(TextFormat("Score: %d", score),
            20, 15, 30 * scoreScale, primaryColor);

        DrawText("Lives:", GetScreenWidth() - 220, 15, 30, primaryColor);
        int heartSpacing = 35;
        for (int i = 0; i < lives; i++) {
            DrawText("?", GetScreenWidth() - 120 + (i * heartSpacing), 15, 30, RED);
        }

        // Draw combo
        if (combo > 0) {
            float comboWidth = 140 * (comboTimer / COMBO_TIME_LIMIT);
            std::string comboText = TextFormat("Combo x%d", combo + 1);
            DrawText(comboText.c_str(),
                GetScreenWidth() / 2 - MeasureText(comboText.c_str(), 30) / 2,
                15, 30, accentColor);

            DrawRectangle(GetScreenWidth() / 2 - 70, 50, 140, 5,
                ColorAlpha(GRAY, 0.3f));
            DrawRectangle(GetScreenWidth() / 2 - 70, 50, comboWidth, 5, accentColor);
        }

        // Draw pause button
        Rectangle pauseBtn = {
            GetScreenWidth() - 50.0f,
            70.0f,
            40.0f,
            40.0f
        };

        if (CheckCollisionPointRec(GetMousePosition(), pauseBtn)) {
            DrawRectangleLinesEx(pauseBtn, 2, secondaryColor);
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                TogglePause();
            }
        }
        else {
            DrawRectangleLinesEx(pauseBtn, 2, primaryColor);
        }

        DrawRectangle(pauseBtn.x + 10, pauseBtn.y + 10, 8, 20, primaryColor);
        DrawRectangle(pauseBtn.x + 22, pauseBtn.y + 10, 8, 20, primaryColor);

        // Draw pause overlay
        if (isPaused) {
            DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(),
                ColorAlpha(BLACK, 0.7f));

            int menuCenterX = GetScreenWidth() / 2;
            int menuCenterY = GetScreenHeight() / 2;

            DrawText("PAUSED",
                menuCenterX - MeasureText("PAUSED", 60) / 2,
                menuCenterY - 100,
                60, primaryColor);

            Rectangle continueBtn = {
                menuCenterX - 100,
                menuCenterY,
                200,
                40
            };

            if (CheckCollisionPointRec(GetMousePosition(), continueBtn)) {
                DrawRectangleLinesEx(continueBtn, 2, secondaryColor);
                if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                    TogglePause();
                }
            }
            else {
                DrawRectangleLinesEx(continueBtn, 2, primaryColor);
            }

            DrawText("CONTINUE",
                menuCenterX - MeasureText("CONTINUE", 20) / 2,
                menuCenterY + 10,
                20, primaryColor);

            Rectangle quitBtn = {
                menuCenterX - 100,
                menuCenterY + 60,
                200,
                40
            };

            if (CheckCollisionPointRec(GetMousePosition(), quitBtn)) {
                DrawRectangleLinesEx(quitBtn, 2, secondaryColor);
                if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                    isRunning = false;
                    isGameOver = true;
                    SaveHighScore();
                    isPaused = false;
                }
            }
            else {
                DrawRectangleLinesEx(quitBtn, 2, primaryColor);
            }

            DrawText("QUIT GAME",
                menuCenterX - MeasureText("QUIT GAME", 20) / 2,
                menuCenterY + 70,
                20, primaryColor);
        }
    }
    if (beginEnd)
    EndDrawing();
}


void FallingWordsGame::ResetGame() {
    fallingWords.clear();
    particles.clear();
    score = 0;
    lives = 3;
    combo = 0;
    comboTimer = 0;
    timer = 0.0f;
    difficultyTimer = 0.0f;
    spawnInterval = INITIAL_SPAWN_INTERVAL;
    currentBaseSpeed = BASE_WORD_SPEED;  // Initialize base spee
    screenShake = 0.0f;
    isRunning = true;
    isGameOver = false;
    isPaused = false;
}

void FallingWordsGame::StartGame() {
    ResetGame();
}

void FallingWordsGame::TogglePause() {
    isPaused = !isPaused;
}

