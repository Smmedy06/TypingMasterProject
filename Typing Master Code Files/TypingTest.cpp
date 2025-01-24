#include "TypingTest.h"
#include <sstream>
#include <fstream>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <chrono>
#include <iomanip>

TypingTest::TypingTest(const std::string& username)
    : username(username), duration(60), complexity(1), useCustomPassage(false),
    correctChars(0), totalChars(0), timer(0.0f), customPassage(""),
    currentWPM(0), currentIndex(0), testActive(true), passageScrollY(0), inputScrollY(0){

    srand(static_cast<unsigned>(time(0)));

    keyboardLayout = {
        {0, {"`", "1", "2", "3", "4", "5", "6", "7", "8", "9", "0", "-", "=", "?"}},
        {1, {"Tab", "Q", "W", "E", "R", "T", "Y", "U", "I", "O", "P", "[", "]", "\\"}},
        {2, {"Caps", "A", "S", "D", "F", "G", "H", "J", "K", "L", ";", "'", "Enter"}},
        {3, {"Shift", "Z", "X", "C", "V", "B", "N", "M", ",", ".", "/", "Shift"}},
        {4, {"Ctrl", "Win", "Alt", "Space", "Alt", "Fn", "Menu", "Ctrl"}}
    };

    specialKeyWidths = {
        {"?", 2.0f}, {"Tab", 1.5f}, {"Caps", 1.75f}, {"Enter", 2.25f},
        {"Shift", 2.25f}, {"Space", 6.25f}, {"Ctrl", 1.25f}, {"Alt", 1.25f},
        {"Win", 1.25f}, {"Fn", 1.25f}, {"Menu", 1.25f}
    };
}

void TypingTest::showCustomPassageInput() {
    static float cursorBlinkTimer = 0.0f;
    static float scrollOffset = 0.0f;
    cursorBlinkTimer += GetFrameTime();
    bool showCursor = (int)(cursorBlinkTimer * 2) % 2 == 0;

    const int screenWidth = GetScreenWidth();
    const int containerWidth = 800;
    const int containerX = (screenWidth - containerWidth) / 2;

    DrawText("Custom Passage", containerX + 40, 360, 24, DARKGRAY);

    // Show character count
    std::string charCount = TextFormat("Characters: %d/1000", customPassage.length());
    DrawText(charCount.c_str(),
        containerX + containerWidth - 40 - MeasureText(charCount.c_str(), 20),
        360,
        20,
        customPassage.length() >= 1000 ? RED : DARKGRAY);

    // Input box with scroll indicators
    Rectangle inputBox = { (float)(containerX + 40), 390, (float)(containerWidth - 80), 120 };
    DrawRectangle(inputBox.x, inputBox.y, inputBox.width, inputBox.height, WHITE);
    DrawRectangleLines(inputBox.x, inputBox.y, inputBox.width, inputBox.height, DARKGRAY);

    // Add scroll bar if content exceeds box height
    float maxScroll = calculateMaxScroll(inputBox);
    if (maxScroll > 0) {
        float scrollBarHeight = (inputBox.height / (maxScroll + inputBox.height)) * inputBox.height;
        float scrollBarY = inputBox.y + (scrollOffset / maxScroll) * (inputBox.height - scrollBarHeight);

        // Draw scroll bar background
        DrawRectangle(inputBox.x + inputBox.width - 8, inputBox.y, 8, inputBox.height, LIGHTGRAY);
        // Draw scroll bar
        DrawRectangle(inputBox.x + inputBox.width - 8, scrollBarY, 8, scrollBarHeight, GRAY);
    }

    // Scissor test to clip text within input box
    BeginScissorMode(inputBox.x, inputBox.y, inputBox.width - 10, inputBox.height);

    static bool isSelecting = false;
    static size_t selectionStart = 0;
    static size_t selectionEnd = 0;
    Vector2 mousePos = GetMousePosition();

    // Handle mouse input for selection and scrolling
    if (CheckCollisionPointRec(mousePos, inputBox)) {
        float wheel = GetMouseWheelMove();
        if (wheel != 0) {
            scrollOffset = Clamp(scrollOffset - (wheel * 20.0f), 0.0f, maxScroll);
        }

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            isSelecting = true;
            selectionStart = getTextPositionFromMouse(mousePos, inputBox, scrollOffset);
            selectionEnd = selectionStart;
        }
        else if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
            // Auto-scroll when dragging near edges
            if (mousePos.y < inputBox.y) {
                scrollOffset = Clamp(scrollOffset - 5.0f, 0.0f, maxScroll);
            }
            else if (mousePos.y > inputBox.y + inputBox.height) {
                scrollOffset = Clamp(scrollOffset + 5.0f, 0.0f, maxScroll);
            }
            selectionEnd = getTextPositionFromMouse(mousePos, inputBox, scrollOffset);
        }
    }

    if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
        isSelecting = false;
    }

    // Handle Ctrl+A
    if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_A)) {
        selectionStart = 0;
        selectionEnd = customPassage.length();
    }

    const int fontSize = 20;
    const int lineHeight = fontSize + 5;
    float textX = inputBox.x + 10;
    float textY = inputBox.y + 10 - scrollOffset;

    // Draw text with wrapping and selection highlighting
    std::vector<std::string> lines = wrapText(customPassage, inputBox.width - 30, fontSize);
    size_t currentPos = 0;

    for (const std::string& line : lines) {
        if (selectionStart != selectionEnd) {
            size_t lineStart = currentPos;
            size_t lineEnd = currentPos + line.length();

            if (lineStart < std::max(selectionStart, selectionEnd) &&
                lineEnd > std::min(selectionStart, selectionEnd)) {

                size_t highlightStart = std::max(lineStart, std::min(selectionStart, selectionEnd));
                size_t highlightEnd = std::min(lineEnd, std::max(selectionStart, selectionEnd));

                std::string beforeHighlight = line.substr(0, highlightStart - lineStart);
                std::string highlightText = line.substr(highlightStart - lineStart, highlightEnd - highlightStart);

                float highlightX = textX + MeasureText(beforeHighlight.c_str(), fontSize);
                float highlightWidth = MeasureText(highlightText.c_str(), fontSize);

                DrawRectangle(highlightX, textY, highlightWidth, lineHeight, { 0, 120, 215, 255 });
                DrawText(highlightText.c_str(), highlightX, textY, fontSize, WHITE);
            }
        }

        DrawText(line.c_str(), textX, textY, fontSize, BLACK);

        // Draw cursor
        if (showCursor && currentPos + line.length() >= selectionStart && currentPos <= selectionStart) {
            float cursorX = textX + MeasureText(line.substr(0, selectionStart - currentPos).c_str(), fontSize);
            DrawRectangle(cursorX, textY, 2, fontSize, BLACK);
        }

        textY += lineHeight;
        currentPos += line.length();
    }

    EndScissorMode();

    // Handle text input and maintain scroll position
    size_t oldLength = customPassage.length();
    handleTextInput(selectionStart, selectionEnd);

    // Auto-scroll to cursor if it's outside visible area
    if (oldLength != customPassage.length()) {
        float cursorY = (selectionStart / (float)customPassage.length()) * maxScroll;
        if (cursorY < scrollOffset) {
            scrollOffset = cursorY;
        }
        else if (cursorY > scrollOffset + inputBox.height - lineHeight) {
            scrollOffset = cursorY - inputBox.height + lineHeight;
        }
        scrollOffset = Clamp(scrollOffset, 0.0f, maxScroll);
    }
}
/*void TypingTest::displayPassage() {
    const int margin = 50;
    const int maxWidth = GetScreenWidth() - 2 * margin;
    const int fontSize = 20;
    const int lineHeight = fontSize + 10;
    const int letterSpacing = 2;

    // Improved container with better visibility
    DrawRectangle(margin - 10, 70, GetScreenWidth() - 2 * (margin - 10), 300, RAYWHITE);
    DrawRectangleLines(margin - 10, 70, GetScreenWidth() - 2 * (margin - 10), 300, LIGHTGRAY);

    // Enhanced WPM display
    std::string wpmText = TextFormat("Current WPM: %d", currentWPM);
    Color wpmColor = currentWPM > 60 ? GREEN : (currentWPM > 30 ? ORANGE : DARKGRAY);
    DrawText(wpmText.c_str(), GetScreenWidth() - 200, 30, 24, wpmColor);

    // Improved passage display with better readability
    DrawText("Type this:", margin, 90, 24, DARKGRAY);

    std::vector<std::string> lines = wrapText(passage, maxWidth, fontSize);
    int y = 140;
    size_t totalCharsDrawn = 0;

    for (const std::string& line : lines) {
        int x = margin;
        for (size_t i = 0; i < line.length(); i++) {
            char c = line[i];
            Color charColor;

            if (totalCharsDrawn == currentIndex) {
                // Current character to type (more visible cursor)
                DrawRectangle(x, y, MeasureText(&c, fontSize), fontSize, { 0, 120, 215, 64 });
                charColor = BLUE;
            }
            else if (totalCharsDrawn < currentIndex) {
                // Typed characters (with success/error indication)
                if (totalCharsDrawn < userInput.length() && userInput[totalCharsDrawn] == c) {
                    charColor = GREEN;
                }
                else {
                    charColor = RED;
                }
            }
            else {
                // Upcoming characters
                charColor = GRAY;
            }

            std::string charStr(1, c);
            DrawText(charStr.c_str(), x, y, fontSize, charColor);
            x += MeasureText(charStr.c_str(), fontSize) + letterSpacing;
            totalCharsDrawn++;
        }
        y += lineHeight;
    }

    // Enhanced user input display
    DrawText("Your input:", margin, y + 20, 24, DARKGRAY);
    y += lineHeight + 20;

    // Draw user input with improved visibility
    std::vector<std::string> inputLines = wrapText(userInput, maxWidth, fontSize);
    for (const std::string& line : inputLines) {
        int x = margin;
        for (size_t i = 0; i < line.length(); i++) {
            char c = line[i];
            Color charColor = (i < passage.length() && c == passage[i]) ? GREEN : RED;
            std::string charStr(1, c);

            DrawText(charStr.c_str(), x, y, fontSize, charColor);
            x += MeasureText(charStr.c_str(), fontSize) + letterSpacing;
        }
        y += lineHeight;
    }
}*/

// Helper function to check if text is selected
bool TypingTest::isTextSelected(size_t start, size_t end, size_t selStart, size_t selEnd) {
    size_t selectionStart = std::min(selStart, selEnd);
    size_t selectionEnd = std::max(selStart, selEnd);
    return (start < selectionEnd && end > selectionStart);
}void TypingTest::handleTextInput(size_t& selectionStart, size_t& selectionEnd) {
    const size_t MAX_CHARS = 1000; // Maximum characters allowed

    // Handle Ctrl+C (Copy)
    if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_C)) {
        if (selectionStart != selectionEnd) {
            size_t start = std::min(selectionStart, selectionEnd);
            size_t length = std::abs((int)(selectionEnd - selectionStart));
            std::string selectedText = customPassage.substr(start, length);
            SetClipboardText(selectedText.c_str());
        }
    }

    // Handle Ctrl+V (Paste)
    if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_V)) {
        const char* clipboard = GetClipboardText();
        if (clipboard != nullptr) {
            std::string newText = clipboard;

            // Remove any newline characters and replace with spaces
            std::replace(newText.begin(), newText.end(), '\n', ' ');
            std::replace(newText.begin(), newText.end(), '\r', ' ');

            // Remove any double spaces
            newText.erase(std::unique(newText.begin(), newText.end(),
                [](char a, char b) { return a == ' ' && b == ' '; }), newText.end());

            // Calculate available space
            size_t availableSpace = MAX_CHARS - (customPassage.length() -
                (selectionEnd > selectionStart ? selectionEnd - selectionStart : selectionStart - selectionEnd));

            // Truncate if necessary
            if (newText.length() > availableSpace) {
                newText = newText.substr(0, availableSpace);
            }

            if (selectionStart != selectionEnd) {
                // Delete selected text before pasting
                size_t start = std::min(selectionStart, selectionEnd);
                size_t length = std::abs((int)(selectionEnd - selectionStart));
                customPassage.erase(start, length);
                selectionEnd = selectionStart = start;
            }

            if (selectionStart <= customPassage.length() && !newText.empty()) {
                customPassage.insert(selectionStart, newText);
                selectionStart = selectionEnd = selectionStart + newText.length();
            }
        }
    }

    // Handle Delete and Backspace
    if (IsKeyPressed(KEY_BACKSPACE) || IsKeyPressed(KEY_DELETE)) {
        if (selectionStart != selectionEnd) {
            size_t start = std::min(selectionStart, selectionEnd);
            size_t length = std::abs((int)(selectionEnd - selectionStart));
            if (start < customPassage.length()) {
                customPassage.erase(start, length);
                selectionStart = selectionEnd = start;
            }
        }
        else if (IsKeyPressed(KEY_BACKSPACE) && selectionStart > 0) {
            customPassage.erase(selectionStart - 1, 1);
            selectionStart = selectionEnd = selectionStart - 1;
        }
        else if (IsKeyPressed(KEY_DELETE) && selectionStart < customPassage.length()) {
            customPassage.erase(selectionStart, 1);
        }
    }

    // Handle regular text input
    int key = GetCharPressed();
    while (key > 0) {
        if ((key >= 32) && (key <= 125) && customPassage.length() < MAX_CHARS) {
            if (selectionStart != selectionEnd) {
                size_t start = std::min(selectionStart, selectionEnd);
                size_t length = std::abs((int)(selectionEnd - selectionStart));
                if (start < customPassage.length()) {
                    customPassage.erase(start, length);
                }
                selectionEnd = selectionStart = start;
            }
            if (selectionStart <= customPassage.length()) {
                customPassage.insert(selectionStart, 1, (char)key);
                selectionStart = selectionEnd = selectionStart + 1;
            }
        }
        key = GetCharPressed();
    }

    // Handle arrow keys for cursor movement
    if (IsKeyPressed(KEY_LEFT) && selectionStart > 0) {
        if (IsKeyDown(KEY_LEFT_SHIFT)) {
            selectionEnd = selectionEnd - 1;
        }
        else {
            selectionStart = selectionEnd = selectionStart - 1;
        }
    }
    if (IsKeyPressed(KEY_RIGHT) && selectionStart < customPassage.length()) {
        if (IsKeyDown(KEY_LEFT_SHIFT)) {
            selectionEnd = selectionEnd + 1;
        }
        else {
            selectionStart = selectionEnd = selectionStart + 1;
        }
    }
}
// Helper functions
size_t TypingTest::getTextPositionFromMouse(Vector2 mousePos, Rectangle inputBox, float scrollOffset) {
    const int fontSize = 20;
    float textX = inputBox.x + 10;
    float textY = inputBox.y + 10 - scrollOffset;

    std::vector<std::string> lines = wrapText(customPassage, inputBox.width - 20, fontSize);
    size_t currentPos = 0;

    for (const std::string& line : lines) {
        if (mousePos.y >= textY && mousePos.y < textY + fontSize + 5) {
            float charX = textX;
            for (size_t i = 0; i <= line.length(); i++) {
                if (mousePos.x < charX) {
                    return currentPos + i;
                }
                if (i < line.length()) {
                    charX += MeasureText(line.substr(i, 1).c_str(), fontSize);
                }
            }
            return currentPos + line.length();
        }
        textY += fontSize + 5;
        currentPos += line.length();
    }
    return customPassage.length();
}

float TypingTest::calculateMaxScroll(Rectangle inputBox) {
    const int fontSize = 20;
    const int lineHeight = fontSize + 5;
    std::vector<std::string> lines = wrapText(customPassage, inputBox.width - 20, fontSize);
    float totalHeight = lines.size() * lineHeight;
    return std::max(0.0f, totalHeight - inputBox.height);
}

std::vector<std::string> TypingTest::wrapText(const std::string& text, float maxWidth, int fontSize) {
    std::vector<std::string> lines;
    std::string currentLine;
    std::string currentWord;

    for (size_t i = 0; i < text.length(); i++) {
        if (text[i] == ' ' || text[i] == '\n') {
            if (MeasureText((currentLine + currentWord).c_str(), fontSize) <= maxWidth) {
                currentLine += currentWord;
                currentWord = text[i];
            }
            else {
                if (!currentLine.empty()) {
                    lines.push_back(currentLine);
                }
                currentLine = currentWord;
                currentWord = text[i];
            }
        }
        else {
            currentWord += text[i];
        }
    }

    if (!currentWord.empty()) {
        if (MeasureText((currentLine + currentWord).c_str(), fontSize) <= maxWidth) {
            currentLine += currentWord;
        }
        else {
            if (!currentLine.empty()) {
                lines.push_back(currentLine);
            }
            currentLine = currentWord;
        }
    }

    if (!currentLine.empty()) {
        lines.push_back(currentLine);
    }

    return lines;
}

void TypingTest::displayPassage() {
    const int margin = 50;
    const int maxWidth = GetScreenWidth() - 2 * margin;
    const int fontSize = 20;
    const int lineHeight = fontSize + 10;
    const int letterSpacing = 2;
    const int boxHeight = 120; // For 4 lines

    // Draw WPM and Timer with consistent positioning and size
    const int statusY = 20;  // Vertical position for status items

    std::string wpmText = TextFormat("Current WPM: %d", currentWPM);
    DrawText(wpmText.c_str(), GetScreenWidth() - 220, statusY, 24, DARKGRAY);

    // Adjust vertical positioning
    int passageBoxY = statusY + 50;  // Added gap after timer
    int inputBoxY = passageBoxY + boxHeight + 60;  // 60 pixel gap between boxes

    // === PASSAGE BOX ===
    Rectangle passageBox = {
        (float)(margin - 10),
        (float)passageBoxY,
        (float)(GetScreenWidth() - 2 * (margin - 10)),
        (float)boxHeight
    };
    DrawRectangle(passageBox.x, passageBox.y, passageBox.width, passageBox.height, RAYWHITE);
    DrawRectangleLines(passageBox.x, passageBox.y, passageBox.width, passageBox.height, LIGHTGRAY);

    DrawText("Type this:", margin, passageBoxY - 25, 20, DARKGRAY);

    // Calculate passage content and scrolling
    std::vector<std::string> lines = wrapText(passage, maxWidth - 20, fontSize);
    float totalPassageHeight = lines.size() * lineHeight;
    float maxPassageScroll = std::max(0.0f, totalPassageHeight - (boxHeight - 20));

    // Reset scroll position when starting a new test (currentIndex == 0)
    if (currentIndex == 0) {
        passageScrollY = 0;
    }
    else {
        // Find the current line and character position
        int currentLine = 0;
        int charsInPreviousLines = 0;

        // Calculate which line we're on and how many characters came before
        for (size_t i = 0; i < lines.size(); i++) {
            if (charsInPreviousLines + lines[i].length() >= currentIndex) {
                currentLine = i;
                break;
            }
            charsInPreviousLines += lines[i].length();
        }

        // Adjust scroll position to keep current line visible
        float desiredScrollY = (currentLine * lineHeight) - (boxHeight / 3);
        passageScrollY = Clamp(desiredScrollY, 0.0f, maxPassageScroll);
    }

    if (maxPassageScroll > 0) {
        float scrollBarHeight = (boxHeight / totalPassageHeight) * boxHeight;
        float scrollBarY = passageBox.y + (passageScrollY / maxPassageScroll) * (boxHeight - scrollBarHeight);
        DrawRectangle(passageBox.x + passageBox.width - 8, passageBox.y, 8, boxHeight, LIGHTGRAY);
        DrawRectangle(passageBox.x + passageBox.width - 8, scrollBarY, 8, scrollBarHeight, GRAY);
    }

    // Draw passage text with fixed character highlighting
    BeginScissorMode(margin, passageBoxY, maxWidth, boxHeight);
    int charsDrawn = 0;
    float y = passageBoxY + 10 - passageScrollY;

    for (const std::string& line : lines) {
        if (y + lineHeight >= passageBoxY && y <= passageBoxY + boxHeight) {
            float x = margin;
            for (size_t i = 0; i < line.length(); i++) {
                char c = line[i];
                Color charColor = GRAY;

                if (charsDrawn == currentIndex) {
                    charColor = BLUE;  // Just change color instead of highlighting
                }
                else if (charsDrawn < currentIndex) {
                    charColor = LIGHTGRAY;
                }

                std::string charStr(1, c);
                DrawText(charStr.c_str(), x, y, fontSize, charColor);
                x += MeasureText(charStr.c_str(), fontSize) + letterSpacing;
                charsDrawn++;
            }
        }
        else {
            charsDrawn += line.length();
        }
        y += lineHeight;
    }
    EndScissorMode();

    // === INPUT BOX ===
    Rectangle inputBox = {
        (float)(margin - 10),
        (float)inputBoxY,
        (float)(GetScreenWidth() - 2 * (margin - 10)),
        (float)boxHeight
    };
    DrawRectangle(inputBox.x, inputBox.y, inputBox.width, inputBox.height, RAYWHITE);
    DrawRectangleLines(inputBox.x, inputBox.y, inputBox.width, inputBox.height, LIGHTGRAY);

    DrawText("Your input:", margin, inputBoxY - 25, 20, DARKGRAY);

    // Calculate input content and scrolling
    std::vector<std::string> inputLines = wrapText(userInput, maxWidth - 20, fontSize);
    float totalInputHeight = inputLines.size() * lineHeight;
    float maxInputScroll = std::max(0.0f, totalInputHeight - (boxHeight - 20));

    if (!inputLines.empty()) {
        float desiredInputScrollY = totalInputHeight - boxHeight + 20;
        inputScrollY = Clamp(desiredInputScrollY, 0.0f, maxInputScroll);
    }

    if (maxInputScroll > 0) {
        float scrollBarHeight = (boxHeight / totalInputHeight) * boxHeight;
        float scrollBarY = inputBox.y + (inputScrollY / maxInputScroll) * (boxHeight - scrollBarHeight);
        DrawRectangle(inputBox.x + inputBox.width - 8, inputBox.y, 8, boxHeight, LIGHTGRAY);
        DrawRectangle(inputBox.x + inputBox.width - 8, scrollBarY, 8, scrollBarHeight, GRAY);
    }

    BeginScissorMode(margin, inputBoxY, maxWidth, boxHeight);
    float inputY = inputBoxY + 10 - inputScrollY;
    size_t totalInputChars = 0;

    for (const std::string& line : inputLines) {
        if (inputY + lineHeight >= inputBoxY && inputY <= inputBoxY + boxHeight) {
            float x = margin;
            for (size_t i = 0; i < line.length(); i++) {
                char c = line[i];
                Color charColor = GRAY;

                // Only check characters that have a corresponding position in the passage
                if (totalInputChars < passage.length()) {
                    if (totalInputChars < userInput.length()) {  // Only check characters that have been typed
                        charColor = (c == passage[totalInputChars]) ? GREEN : RED;
                    }
                }

                std::string charStr(1, c);
                DrawText(charStr.c_str(), x, inputY, fontSize, charColor);
                x += MeasureText(charStr.c_str(), fontSize) + letterSpacing;
                totalInputChars++;
            }
        }
        else {
            totalInputChars += line.length();
        }
        inputY += lineHeight;
    }
    EndScissorMode();
}
void TypingTest::update() {
    if (testActive) {
        handleMenuInput();
        if (IsKeyPressed(KEY_ESCAPE)) {
            testActive = false;
        }
    }
}

void TypingTest::render() {
    if (testActive) {
        ClearBackground(LIGHTGRAY);
        drawMainMenu();
    }
}



void TypingTest::drawMainMenu() {
    const int screenWidth = GetScreenWidth();
    const int centerX = screenWidth / 2;

    // Exit button in top-right corner
    Rectangle exitBtn = { (float)(screenWidth - 100), 20, 80, 40 };
    Color exitBtnColor = CheckCollisionPointRec(GetMousePosition(), exitBtn) ? RED : MAROON;
    DrawRectangleRec(exitBtn, exitBtnColor);
    DrawText("Exit", exitBtn.x + (exitBtn.width - MeasureText("Exit", 20)) / 2,
        exitBtn.y + 10, 20, WHITE);

    if (CheckCollisionPointRec(GetMousePosition(), exitBtn) &&
        IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        setTestActive(false);  // This will trigger return to main menu
        return;
    }


    // Title
    DrawText("Typing Test", centerX - MeasureText("Typing Test", 50) / 2, 50, 50, BLACK);

    // Settings container
    const int containerWidth = 800;
    const int containerX = (screenWidth - containerWidth) / 2;
    DrawRectangle(containerX, 140, containerWidth, 400, RAYWHITE);
    DrawRectangleLines(containerX, 140, containerWidth, 400, LIGHTGRAY);

    // Duration Selection
    const int columnWidth = containerWidth / 3 - 40;
    DrawText("Duration", containerX + 40, 160, 24, DARKGRAY);
    drawButton(containerX + 40, 200, columnWidth, 40, "30 Seconds", duration == 30);
    drawButton(containerX + 40, 250, columnWidth, 40, "1 Minute", duration == 60);
    drawButton(containerX + 40, 300, columnWidth, 40, "3 Minutes", duration == 180);

    // Complexity Selection
    DrawText("Difficulty", containerX + containerWidth / 3 + 20, 160, 24, DARKGRAY);
    drawButton(containerX + containerWidth / 3 + 20, 200, columnWidth, 40, "Easy", complexity == 1);
    drawButton(containerX + containerWidth / 3 + 20, 250, columnWidth, 40, "Medium", complexity == 2);
    drawButton(containerX + containerWidth / 3 + 20, 300, columnWidth, 40, "Hard", complexity == 3);

    // Passage Options
    DrawText("Passage Type", containerX + 2 * containerWidth / 3, 160, 24, DARKGRAY);
    drawButton(containerX + 2 * containerWidth / 3, 200, columnWidth, 40, "Custom", useCustomPassage);
    drawButton(containerX + 2 * containerWidth / 3, 250, columnWidth, 40, "Random", !useCustomPassage);

    if (useCustomPassage) {
        showCustomPassageInput();
    }

    // Start Button
    Rectangle startButton = { (float)(centerX - 100), 570, 200, 50 };
    bool startHovered = CheckCollisionPointRec(GetMousePosition(), startButton);
    DrawRectangle(startButton.x, startButton.y, startButton.width, startButton.height,
        startHovered ? DARKGRAY : BLACK);
    DrawText("Start Test", centerX - MeasureText("Start Test", 24) / 2, 583, 24, WHITE);

    if (startHovered && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        if (useCustomPassage && customPassage.empty()) {
            DrawText("Please enter a custom passage!",
                centerX - MeasureText("Please enter a custom passage!", 20) / 2,
                640, 20, RED);
        }
        else {
            startTest();
        }
    }
}

void TypingTest::drawButton(int x, int y, int width, int height, const std::string& text, bool isSelected) {
    Rectangle buttonRect = { (float)x, (float)y, (float)width, (float)height };
    bool isHovered = CheckCollisionPointRec(GetMousePosition(), buttonRect);

    DrawRectangle(x, y, width, height, isSelected ? BLACK : (isHovered ? LIGHTGRAY : WHITE));
    DrawRectangleLines(x, y, width, height, isSelected ? WHITE : DARKGRAY);

    int textWidth = MeasureText(text.c_str(), 20);
    int textX = x + (width - textWidth) / 2;
    int textY = y + (height - 20) / 2;
    DrawText(text.c_str(), textX, textY, 20, isSelected ? WHITE : BLACK);
}

float TypingTest::calculateRowWidth(const std::vector<std::string>& row, float baseKeyWidth, float spacing) {
    float totalWidth = 0;
    for (const std::string& key : row) {
        float keyWidth = baseKeyWidth * (specialKeyWidths.count(key) ? specialKeyWidths[key] : 1.0f);
        totalWidth += keyWidth + spacing;
    }
    return totalWidth - spacing;
}

void TypingTest::updateWPM() {
    float elapsedMinutes = (duration - timer) / 60.0f;
    if (elapsedMinutes > 0) {
        currentWPM = static_cast<int>((correctChars / 5) / elapsedMinutes);
    }
}

void TypingTest::handleMenuInput() {
    Vector2 mousePos = GetMousePosition();
    const int screenWidth = GetScreenWidth();
    const int containerWidth = 800;
    const int containerX = (screenWidth - containerWidth) / 2;
    const int columnWidth = containerWidth / 3 - 40;

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        // Duration buttons
        if (CheckCollisionPointRec(mousePos, { (float)(containerX + 40), 200, (float)columnWidth, 40 }))
            duration = 30;
        if (CheckCollisionPointRec(mousePos, { (float)(containerX + 40), 250, (float)columnWidth, 40 }))
            duration = 60;
        if (CheckCollisionPointRec(mousePos, { (float)(containerX + 40), 300, (float)columnWidth, 40 }))
            duration = 180;

        // Complexity buttons
        if (CheckCollisionPointRec(mousePos, { (float)(containerX + containerWidth / 3 + 20), 200, (float)columnWidth, 40 }))
            complexity = 1;
        if (CheckCollisionPointRec(mousePos, { (float)(containerX + containerWidth / 3 + 20), 250, (float)columnWidth, 40 }))
            complexity = 2;
        if (CheckCollisionPointRec(mousePos, { (float)(containerX + containerWidth / 3 + 20), 300, (float)columnWidth, 40 }))
            complexity = 3;

        // Passage option buttons
        if (CheckCollisionPointRec(mousePos, { (float)(containerX + 2 * containerWidth / 3), 200, (float)columnWidth, 40 }))
            useCustomPassage = true;
        if (CheckCollisionPointRec(mousePos, { (float)(containerX + 2 * containerWidth / 3), 250, (float)columnWidth, 40 }))
            useCustomPassage = false;
    }
}
// In TypingTest.cpp

void TypingTest::startTest() {
    timer = static_cast<float>(duration);
    userInput.clear();
    correctChars = 0;
    totalChars = 0;
    currentIndex = 0;
    currentWPM = 0;
    returnToMenu = false;
    size_t currentPassageIndex = 0;

    std::string filePath;
    switch (complexity) {
    case 1: filePath = "easy.txt"; break;
    case 2: filePath = "medium.txt"; break;
    case 3: filePath = "hard.txt"; break;
    default: filePath = "easy.txt"; break;
    }

    std::vector<std::string> allPassages;

    if (useCustomPassage) {
        allPassages.push_back(customPassage);
    }
    else {
        std::ifstream file(filePath);
        std::string line;
        while (std::getline(file, line)) {
            if (!line.empty()) {
                allPassages.push_back(line);
            }
        }
        std::random_device rd;
        std::mt19937 gen(rd());
        std::shuffle(allPassages.begin(), allPassages.end(), gen);
    }

    if (allPassages.empty()) {
        allPassages.push_back("Error: No passages found.");
    }

    bool testCompleted = false;
    passage = allPassages[currentPassageIndex];
    testActive = true;

    while (!WindowShouldClose() && timer > 0 && !testCompleted && testActive) {
        timer -= GetFrameTime();

        BeginDrawing();
        ClearBackground(RAYWHITE);

        renderTimer();
        displayPassage();
        renderKeyboard();

        // Handle Enter key to end test
        if (IsKeyPressed(KEY_ENTER)) {
            testCompleted = true;
            break;
        }

        // Modified input handling
        handleKeyPress();

        // Check if passage is completed (regardless of mistakes)
        if (userInput.length() >= passage.length()) {
            currentPassageIndex++;
            if (currentPassageIndex < allPassages.size()) {
                passage = allPassages[currentPassageIndex];
                userInput.clear();
                currentIndex = 0;
            }
        }

        EndDrawing();

        if (returnToMenu) {
            testActive = false;
            break;
        }
    }

    if (!returnToMenu) {
        showResults();
        saveStats();
    }
}

void TypingTest::renderExitButton() {
    Rectangle exitBtn = { (float)(GetScreenWidth() - 100), 20, 80, 40 };
    bool exitHovered = CheckCollisionPointRec(GetMousePosition(), exitBtn);
    Color exitBtnColor = exitHovered ? RED : MAROON;

    DrawRectangleRec(exitBtn, exitBtnColor);
    DrawText("Exit", exitBtn.x + (exitBtn.width - MeasureText("Exit", 20)) / 2,
        exitBtn.y + 10, 20, WHITE);

    if (exitHovered && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        testActive = false;
    }
}

// Update displayMainMenu to handle exit properly
void TypingTest::displayMainMenu() {
    testActive = true;
    while (testActive && !WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(LIGHTGRAY);

        drawMainMenu();
        handleMenuInput();

        EndDrawing();

        if (IsKeyPressed(KEY_ESCAPE)) {
            testActive = false;
        }
    }
}
void TypingTest::renderTimer() {
    int minutes = static_cast<int>(timer) / 60;
    int seconds = static_cast<int>(timer) % 60;
    DrawText(TextFormat("Time Left: %02d:%02d", minutes, seconds), 10, 10, 24, DARKGRAY);
}

void TypingTest::handleKeyPress() {
    int key = GetCharPressed();
    while (key > 0) {
        if ((key >= 32) && (key <= 125)) {
            userInput += static_cast<char>(key);
            totalChars++;
            if (userInput.length() <= passage.length() &&
                static_cast<char>(key) == passage[userInput.length() - 1]) {
                correctChars++;
                currentIndex++;
            }
            updateWPM();
        }
        key = GetCharPressed();
    }

    if (IsKeyPressed(KEY_BACKSPACE) && !userInput.empty()) {
        if (userInput.length() <= passage.length() &&
            userInput.back() == passage[userInput.length() - 1]) {
            correctChars--;
            currentIndex--;
        }
        userInput.pop_back();
        if (totalChars > 0) totalChars--;
        updateWPM();
    }

    if (IsKeyPressed(KEY_SPACE)) {
        if (userInput.empty() || userInput.back() != ' ') {
            userInput += " ";
            totalChars++;
            if (userInput.length() <= passage.length() &&
                passage[userInput.length() - 1] == ' ') {
                correctChars++;
                currentIndex++;
            }
            updateWPM();
        }
    }
}

void TypingTest::renderKeyboard() {
    const float baseKeyWidth = 50;
    const float keyHeight = 50;
    const float spacing = 4;
    const int startY = GetScreenHeight() - (5 * (keyHeight + spacing)) - 50;

    float currentY = startY;

    for (const auto& row : keyboardLayout) {
        float rowWidth = calculateRowWidth(row.second, baseKeyWidth, spacing);
        float currentX = (GetScreenWidth() - rowWidth) / 2;

        for (const std::string& key : row.second) {
            float keyWidth = baseKeyWidth * (specialKeyWidths.count(key) ? specialKeyWidths[key] : 1.0f);
            Rectangle keyRect = { currentX, currentY, keyWidth, keyHeight };

            bool isPressed = false;
            if (key.length() == 1) {
                isPressed = IsKeyDown(static_cast<int>(key[0]));
            }
            else {
                if (key == "Space") isPressed = IsKeyDown(KEY_SPACE);
                else if (key == "?") isPressed = IsKeyDown(KEY_BACKSPACE);
                else if (key == "Enter") isPressed = IsKeyDown(KEY_ENTER);
                else if (key == "Tab") isPressed = IsKeyDown(KEY_TAB);
                else if (key == "Shift") isPressed = IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT);
                else if (key == "Ctrl") isPressed = IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL);
                else if (key == "Alt") isPressed = IsKeyDown(KEY_LEFT_ALT) || IsKeyDown(KEY_RIGHT_ALT);
            }

            DrawKey(keyRect, key.c_str(), isPressed);
            currentX += keyWidth + spacing;
        }
        currentY += keyHeight + spacing;
    }
}

void TypingTest::DrawKey(Rectangle keyRect, const char* key, bool isPressed) {
    Color keyColor = isPressed ? DARKGRAY : WHITE;
    Color borderColor = LIGHTGRAY;
    Color textColor = isPressed ? WHITE : BLACK;

    DrawRectangle(keyRect.x + 2, keyRect.y + 2, keyRect.width, keyRect.height, GRAY);
    DrawRectangleRounded(keyRect, 0.2f, 8, keyColor);
    DrawRectangleRoundedLines(keyRect, 0.2f, 8, borderColor);

    int fontSize = strlen(key) > 1 ? 14 : 18;
    float textX = keyRect.x + (keyRect.width - MeasureText(key, fontSize)) / 2;
    float textY = keyRect.y + (keyRect.height - fontSize) / 2;

    DrawText(key, textX, textY, fontSize, textColor);
}

void TypingTest::showResults() {
    bool resultScreenActive = true;
    while (!WindowShouldClose() && resultScreenActive) {
        BeginDrawing();
        ClearBackground(RAYWHITE);

        const int centerX = GetScreenWidth() / 2;
        DrawText("Test Completed!", centerX - MeasureText("Test Completed!", 40) / 2, 50, 40, DARKGREEN);

        std::string wpmText = TextFormat("Words Per Minute (WPM): %d", calculateWPM());
        DrawText(wpmText.c_str(), 50, 150, 30, DARKGRAY);

        std::string accuracyText = TextFormat("Accuracy: %.1f%%", calculateAccuracy());
        DrawText(accuracyText.c_str(), 50, 200, 30, DARKGRAY);

        std::string correctCharsText = TextFormat("Correct Characters: %d", correctChars);
        DrawText(correctCharsText.c_str(), 50, 250, 30, DARKGRAY);

        std::string totalCharsText = TextFormat("Total Characters: %d", totalChars);
        DrawText(totalCharsText.c_str(), 50, 300, 30, DARKGRAY);

        DrawText("Press [ESC] to return to Typing Test", 50, 400, 20, DARKGRAY);

        // Add exit button
        //Rectangle exitBtn = { (float)(GetScreenWidth() - 100), 20, 80, 30 };
      //  bool exitHovered = CheckCollisionPointRec(GetMousePosition(), exitBtn);
      //  DrawRectangleRec(exitBtn, exitHovered ? RED : MAROON);
      //  DrawText("Exit", exitBtn.x + (exitBtn.width - MeasureText("Exit", 20)) / 2,
          //  exitBtn.y + 5, 20, WHITE);

        if ((IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) || IsKeyPressed(KEY_ESCAPE)) {
            resultScreenActive = false;
            displayMainMenu();  // Return to typing test menu
        }

        EndDrawing();
    }
}
int TypingTest::calculateWPM() {
    int wordsTyped = correctChars / 5;
    float minutes = static_cast<float>(duration - timer) / 60.0f;
    return static_cast<int>(wordsTyped / (minutes > 0 ? minutes : 1));
}

float TypingTest::calculateAccuracy() {
    return (totalChars > 0) ? (static_cast<float>(correctChars) / totalChars) * 100.0f : 0.0f;
}

std::string TypingTest::getRandomPassage(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        return "Error: Could not load passage file.";
    }

    std::vector<std::string> passages;
    std::string line;

    while (std::getline(file, line)) {
        if (!line.empty()) {
            passages.push_back(line);
        }
    }
    file.close();

    if (passages.empty()) {
        return "Error: No passages found in file.";
    }

    return passages[rand() % passages.size()];
}

void TypingTest::saveStats() {
    std::ofstream file("typing_history.txt", std::ios::app);
    if (file.is_open()) {
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);

        struct tm timeinfo;
        localtime_s(&timeinfo, &time);

        std::stringstream ss;
        ss << std::put_time(&timeinfo, "%Y-%m-%d %H:%M:%S");

        file << "User: " << username << "\n";
        file << "Date: " << ss.str() << "\n";
        file << "WPM: " << calculateWPM() << "\n";
        file << "Accuracy: " << calculateAccuracy() << "%\n";
        file << "Duration: " << duration << " seconds\n";
        file << "Difficulty: " << complexity << "\n";
        file << "------------------------\n";
        file.close();
    }
}