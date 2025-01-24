#pragma once
#include <string>
#include "raylib.h"
#include <vector>
#include <unordered_map>
#include <random>
#include <algorithm>

class TypingTest {
public:
    // Constructor
    TypingTest(const std::string& username);
    // Public member functions
    void displayMainMenu();
    void startTest();
    void saveStats();
    void showResults();
    int calculateWPM();
    float calculateAccuracy();
    void updateWPM();
    int currentWPM;
    size_t currentIndex;
    void update(); // New method to update typing test state
    void render(); // New method to render typing test
    bool isTestActive() const { return testActive; }
    void setTestActive(bool active) { testActive = active; }



private:
    // Menu-related functions
    void drawMainMenu();
    void drawButton(int x, int y, int width, int height, const std::string& text, bool isSelected);
    void handleMenuInput();
    void showCustomPassageInput();
    void updateCustomPassageInput();
    bool testActive;
    bool returnToMenu;

    // Custom passage input helper functions
    size_t getTextPositionFromMouse(Vector2 mousePos, Rectangle inputBox, float scrollOffset);
    float calculateMaxScroll(Rectangle inputBox);
    std::vector<std::string> wrapText(const std::string& text, float maxWidth, int fontSize);
    void handleTextInput(size_t& selectionStart, size_t& selectionEnd);
    float Clamp(float value, float min, float max) {
        if (value < min) return min;
        if (value > max) return max;
        return value;
    }

    // Test-related functions
    void renderTimer();
    void renderExitButton();
    void displayPassage();
    void renderKeyboard();
    void handleKeyPress();
    void calculateResults();
    std::string getRandomPassage(const std::string& filePath);
    void DrawKey(Rectangle keyRect, const char* key, bool isPressed);
    float calculateRowWidth(const std::vector<std::string>& row, float baseKeyWidth, float spacing);

    // Member variables
    float passageScrollY;  // Add to private members
    float inputScrollY;    // Add to private members
    std::string username;
    std::string passage;
    std::string userInput;
    std::string customPassage;
    int duration;
    int complexity;
    bool useCustomPassage;
    int correctChars;
    int totalChars;
    float timer;
    bool isTextSelected(size_t start, size_t end, size_t selStart, size_t selEnd);

    // Keyboard layout related
    std::unordered_map<int, std::vector<std::string>> keyboardLayout;
    std::unordered_map<std::string, float> specialKeyWidths;
};