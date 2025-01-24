#include "Stats.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <iomanip> 

template<typename T>
T clamp(T value, T min, T max) {
    if (value < min) return min;
    if (value > max) return max;
    return value;
}


Stats::Stats(const std::string& username) :
    username(username),
    scrollOffset(0),
    maxScroll(0),
    avgWPM(0),
    avgAccuracy(0),
    totalTests(0),
    bestWPM(0),
    showMainMenu(false) {

    // Initialize colors
    backgroundColor = BLACK;
    cardColor = { 45, 45, 45, 255 };
    highlightColor = { 60, 60, 60, 255 };
    textColor = WHITE;
    accentColor = { 0, 150, 255, 255 };

    // Initialize button colors
    buttonColor = { 0, 120, 215, 255 };
    buttonHoverColor = { 0, 140, 240, 255 };

    loadStats();
    calculateAverages();
}

void Stats::loadStats() {
    std::ifstream file("typing_history.txt");
    if (!file.is_open()) return;

    TypingRecord currentRecord;
    std::string line;

    while (std::getline(file, line)) {
        if (line.find("User: ") == 0) {
            currentRecord = TypingRecord();
            currentRecord.username = line.substr(6);
        }
        else if (line.find("Date: ") == 0) currentRecord.date = line.substr(6);
        else if (line.find("WPM: ") == 0) currentRecord.wpm = std::stoi(line.substr(5));
        else if (line.find("Accuracy: ") == 0) {
            std::string acc = line.substr(10);
            acc = acc.substr(0, acc.length() - 1); // Remove %
            currentRecord.accuracy = std::stof(acc);
        }
        else if (line.find("Duration: ") == 0) {
            std::string dur = line.substr(10);
            currentRecord.duration = std::stoi(dur.substr(0, dur.find(" ")));
        }
        else if (line.find("Difficulty: ") == 0) {
            currentRecord.difficulty = std::stoi(line.substr(12));
        }
        else if (line.find("------------------------") == 0) {
            if (currentRecord.username == username) {
                userRecords.push_back(currentRecord);
            }
        }
    }

    // Sort records by date (newest first)
    std::sort(userRecords.begin(), userRecords.end(),
        [](const TypingRecord& a, const TypingRecord& b) {
            return a.date > b.date;
        });
}

void Stats::calculateAverages() {
    if (userRecords.empty()) return;

    float totalWPM = 0;
    float totalAccuracy = 0;
    bestWPM = 0;

    for (const auto& record : userRecords) {
        totalWPM += record.wpm;
        totalAccuracy += record.accuracy;
        bestWPM = std::max(bestWPM, record.wpm);
    }

    totalTests = userRecords.size();
    avgWPM = totalWPM / totalTests;
    avgAccuracy = totalAccuracy / totalTests;
}

void Stats::drawBackButton() {
    const char* buttonText = "Back to Main Menu";
    int fontSize = 20;
    float buttonWidth = MeasureText(buttonText, fontSize) + 40;
    float buttonHeight = 40;
    float buttonX = 20;
    float buttonY = 20;

    Rectangle buttonBounds = { buttonX, buttonY, buttonWidth, buttonHeight };
    Vector2 mousePos = GetMousePosition();
    bool isHovered = CheckCollisionPointRec(mousePos, buttonBounds);

    DrawRectangleRec(buttonBounds, isHovered ? buttonHoverColor : buttonColor);
    DrawRectangleLinesEx(buttonBounds, 1, WHITE);

    float textX = buttonX + (buttonWidth - MeasureText(buttonText, fontSize)) / 2;
    float textY = buttonY + (buttonHeight - fontSize) / 2;
    DrawText(buttonText, textX, textY, fontSize, WHITE);

    if (isHovered && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        showMainMenu = true;
    }
}

void Stats::drawHeader() {
    int centerX = GetScreenWidth() / 2;
    Rectangle headerBg = { 0, 0, (float)GetScreenWidth(), 150 };
    DrawRectangleRec(headerBg, cardColor);

    std::string title = "Typing Statistics for " + username;
    DrawText(title.c_str(),
        centerX - MeasureText(title.c_str(), 40) / 2,
        20, 40, textColor);

    float startX = centerX - 400;
    float startY = 80;
    float width = 200;

    std::vector<std::pair<std::string, std::string>> stats = {
        {"Total Tests", std::to_string(totalTests)},
        {"Avg WPM", std::to_string((int)avgWPM)},
        {"Best WPM", std::to_string(bestWPM)},
        {"Avg Accuracy", std::to_string((int)avgAccuracy) + "%"}
    };

    for (size_t i = 0; i < stats.size(); i++) {
        Rectangle statBg = { startX + (i * width), startY, width - 10, 50 };
        DrawRectangleRec(statBg, highlightColor);
        DrawRectangleLinesEx(statBg, 1, accentColor);

        DrawText(stats[i].first.c_str(),
            statBg.x + 10,
            statBg.y + 5,
            20, textColor);

        DrawText(stats[i].second.c_str(),
            statBg.x + 10,
            statBg.y + 25,
            25, accentColor);
    }
}

void Stats::drawStatsCard(const TypingRecord& record, float x, float y, bool isHovered) {
    Rectangle cardBg = { x, y, CARD_WIDTH, CARD_HEIGHT };
    DrawRectangleRec(cardBg, isHovered ? highlightColor : cardColor);
    DrawRectangleLinesEx(cardBg, 1, accentColor);

    float textX = x + 20;
    float textY = y + 20;
    float spacing = 30;

    DrawText(record.date.c_str(), textX, textY, 20, textColor);

    textY += spacing;
    DrawText(("WPM: " + std::to_string(record.wpm)).c_str(),
        textX, textY, 25, accentColor);

    textY += spacing;
    DrawText(("Accuracy: " + std::to_string((int)record.accuracy) + "%").c_str(),
        textX, textY, 20, textColor);

    textY += spacing;
    DrawText(("Duration: " + std::to_string(record.duration) + "s").c_str(),
        textX, textY, 20, textColor);

    textY += spacing;
    DrawText(("Difficulty: " + std::to_string(record.difficulty)).c_str(),
        textX, textY, 20, textColor);
}

void Stats::drawStatsTable() {
    const float HEADER_HEIGHT = 175;
    const float CONTENT_AREA_HEIGHT = GetScreenHeight() - HEADER_HEIGHT;
    float startY = HEADER_HEIGHT;
    float screenWidth = (float)GetScreenWidth();

    // Calculate grid layout
    int totalRows = (int)ceil(userRecords.size() / (float)CARDS_PER_ROW);
    float totalContentHeight = totalRows * (CARD_HEIGHT + CARD_SPACING);

    // Adjust maxScroll calculation to prevent overscrolling
    maxScroll = std::max(0.0f, totalContentHeight - CONTENT_AREA_HEIGHT);

    // Ensure scrollOffset stays within bounds
    scrollOffset = clamp(scrollOffset, 0.0f, maxScroll);

    // Calculate visible range
    float visibleTop = scrollOffset;
    float visibleBottom = scrollOffset + CONTENT_AREA_HEIGHT;

    Vector2 mousePos = GetMousePosition();

    // Draw only visible cards
    for (int row = 0; row < totalRows; row++) {
        float rowY = startY + row * (CARD_HEIGHT + CARD_SPACING) - scrollOffset;

        // Skip if row is completely above or below visible area
        if (rowY + CARD_HEIGHT < HEADER_HEIGHT || rowY > GetScreenHeight()) {
            continue;
        }

        for (int col = 0; col < CARDS_PER_ROW; col++) {
            int index = row * CARDS_PER_ROW + col;
            if (index >= userRecords.size()) break;

            float x = (screenWidth - (CARD_WIDTH * CARDS_PER_ROW + CARD_SPACING * (CARDS_PER_ROW - 1))) / 2
                + col * (CARD_WIDTH + CARD_SPACING);
            float y = rowY;

            Rectangle cardBounds = { x, y, CARD_WIDTH, CARD_HEIGHT };
            bool isHovered = CheckCollisionPointRec(mousePos, cardBounds);

            // Apply scissor test to clip cards at header
            BeginScissorMode(0, HEADER_HEIGHT, GetScreenWidth(), GetScreenHeight() - HEADER_HEIGHT);
            drawStatsCard(userRecords[index], x, y, isHovered);
            EndScissorMode();
        }
    }

    // Draw scroll bar
    if (maxScroll > 0) {
        float scrollBarHeight = std::max(30.0f, (CONTENT_AREA_HEIGHT / totalContentHeight) * CONTENT_AREA_HEIGHT);
        float scrollBarY = HEADER_HEIGHT + (scrollOffset / maxScroll) * (CONTENT_AREA_HEIGHT - scrollBarHeight);

        // Draw scroll track
        DrawRectangle(
            GetScreenWidth() - 12,
            HEADER_HEIGHT,
            10,
            CONTENT_AREA_HEIGHT,
            Color{ 30, 30, 30, 255 }
        );

        // Draw scroll handle
        DrawRectangleRounded(
            Rectangle{ (float)GetScreenWidth() - 11,
                      scrollBarY,
                      8,
                      scrollBarHeight },
            0.3f,
            8,
            accentColor
        );
    }
}

bool Stats::handleInput() {
    const float WHEEL_SCROLL_SPEED = 60.0f;
    const float KEYBOARD_SCROLL_SPEED = 15.0f;
    const float PAGE_SCROLL_AMOUNT = GetScreenHeight() * 0.8f;

    float previousOffset = scrollOffset;

    // Mouse wheel scrolling
    float wheel = GetMouseWheelMove();
    if (wheel != 0) {
        scrollOffset -= wheel * WHEEL_SCROLL_SPEED;
    }

    // Keyboard scrolling
    if (IsKeyDown(KEY_UP)) {
        scrollOffset -= KEYBOARD_SCROLL_SPEED;
    }
    if (IsKeyDown(KEY_DOWN)) {
        scrollOffset += KEYBOARD_SCROLL_SPEED;
    }

    // Page Up/Down
    if (IsKeyPressed(KEY_PAGE_UP)) {
        scrollOffset -= PAGE_SCROLL_AMOUNT;
    }
    if (IsKeyPressed(KEY_PAGE_DOWN)) {
        scrollOffset += PAGE_SCROLL_AMOUNT;
    }

    // Home/End
    if (IsKeyPressed(KEY_HOME)) {
        scrollOffset = 0;
    }
    if (IsKeyPressed(KEY_END)) {
        scrollOffset = maxScroll;
    }

    // Clamp scrollOffset within valid range
    scrollOffset = clamp(scrollOffset, 0.0f, maxScroll);

    // Handle scroll bar dragging
    static bool isDragging = false;
    static float dragStartY = 0;
    static float dragStartOffset = 0;

    Rectangle scrollBarArea = {
        (float)GetScreenWidth() - 12,
        150.0f,  // HEADER_HEIGHT
        10,
        (float)GetScreenHeight() - 150.0f  // CONTENT_AREA_HEIGHT
    };

    Vector2 mousePos = GetMousePosition();
    if (CheckCollisionPointRec(mousePos, scrollBarArea)) {
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            isDragging = true;
            dragStartY = mousePos.y;
            dragStartOffset = scrollOffset;
        }
    }

    if (isDragging) {
        if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
            float dragDelta = mousePos.y - dragStartY;
            float scrollRatio = dragDelta / (GetScreenHeight() - 150.0f);  // CONTENT_AREA_HEIGHT
            scrollOffset = clamp(dragStartOffset + (scrollRatio * maxScroll), 0.0f, maxScroll);
        }
        else {
            isDragging = false;
        }
    }

    return showMainMenu || IsKeyPressed(KEY_ESCAPE);
}

void Stats::draw() {
    ClearBackground(backgroundColor);
    drawHeader();
    drawBackButton();
    drawStatsTable();
}