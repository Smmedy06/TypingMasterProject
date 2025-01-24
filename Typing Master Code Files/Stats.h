#ifndef STATS_H
#define STATS_H

#include <string>
#include <vector>
#include <raylib.h>

struct TypingRecord {
    std::string username;
    std::string date;
    int wpm;
    float accuracy;
    int duration;
    int difficulty;
};

class Stats {
public:
    Stats(const std::string& username);
    void draw();
    void loadStats();
    bool handleInput(); // Returns true if user wants to exit stats

private:
    void drawHeader();
    void drawStatsTable();
    void drawStatsCard(const TypingRecord& record, float x, float y, bool isHovered);
    void drawAverageStats();
    void calculateAverages();
    bool showMainMenu;
    Color buttonColor;
    Color buttonHoverColor;
    void drawBackButton();

    std::string username;
    std::vector<TypingRecord> userRecords;
    float scrollOffset;
    float maxScroll;

    // Average stats
    float avgWPM;
    float avgAccuracy;
    int totalTests;
    int bestWPM;

    // UI Constants
    const float CARD_WIDTH = 300;
    const float CARD_HEIGHT = 200;
    const float CARD_SPACING = 20;
    const int CARDS_PER_ROW = 3;

    // Colors
    Color backgroundColor;
    Color cardColor;
    Color highlightColor;
    Color textColor;
    Color accentColor;
};

#endif