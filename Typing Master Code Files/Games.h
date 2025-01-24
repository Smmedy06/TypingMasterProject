#ifndef GAMES_H
#define GAMES_H

#include <vector>
#include <string>
#include "raylib.h"

class FallingWordsGame {
private:
    const float INITIAL_SPAWN_INTERVAL = 2.0f;
    const float MIN_SPAWN_INTERVAL = 0.5f;  // Fastest spawn rate
    const float SPAWN_INTERVAL_DECREASE = 0.95f;  // How much to decrease interval
    const float BASE_WORD_SPEED = 100.0f;
    const float MAX_WORD_SPEED = 300.0f;  // Maximum falling speed
    const float SPEED_INCREMENT = 20.0f;   // How much to increase spee
    float currentBaseSpeed;  // Add this as a class member
    // Structures
    struct CloudPoint {
        float x, y;
        float radius;
    };

    struct Particle {
        Vector2 position;
        Vector2 velocity;
        float radius;
        float lifetime;
        Color color;
    };

    struct FallingWord {
        std::string word;
        std::string typedPart;    // Track what's been typed correctly
        float x;
        float y;
        float speed;
        bool isActive;
        float scale;
        float alpha;
        Color bubbleColor;
        std::vector<CloudPoint> cloudPoints;
        float wobbleTime;
        float clickAnimScale;
    };

    // Game state variables
    std::vector<std::string> wordList;
    std::vector<FallingWord> fallingWords;
    std::vector<Particle> particles;
    int score;
    int highScore;
    int lives;
    int combo;
    float comboTimer;
    float spawnInterval;
    float timer;
    float difficultyTimer;
    bool isRunning;
    bool isGameOver;
    bool isPaused;

    // Theme colors
    Color backgroundColor;
    Color primaryColor;
    Color secondaryColor;
    Color accentColor;
    Color textColor;  // Added missing textColor member

    // Visual effects
    float screenShake;

    // Constants
    static constexpr float COMBO_TIME_LIMIT = 3.0f;
    static constexpr float DIFFICULTY_INCREASE_INTERVAL = 30.0f;
    static constexpr int MAX_COMBO = 5;
    static constexpr int CLOUD_POINTS = 8;

    // Private member functions
    void SpawnWord();
    void ResetGame();
    void LoadWordsFromFile(const std::string& filename);
    void UpdateWordTyping(char typed);
    void CreateCloudShape(FallingWord& word);
    void CreatePopEffect(float x, float y, Color color);
    void UpdateParticles();
    void UpdateCombo();
    void SaveHighScore();
    void LoadHighScores();  // Changed from LoadHighScore to LoadHighScores
    void InitializeTheme();

public:
    // Changed constructor to accept username parameter
    explicit FallingWordsGame(const std::string& username);
    void StartGame();
    void UpdateGame();
    void DrawGame(bool beginEnd = false);
    void HandleClick(int x, int y);
    bool IsPaused() const { return isPaused; }
    bool IsRunning() const { return isRunning && !isGameOver; }
    void TogglePause();
};

#endif // GAMES_H