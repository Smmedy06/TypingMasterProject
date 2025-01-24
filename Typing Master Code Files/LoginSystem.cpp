#include "LoginSystem.h"
#include <raylib.h>
#include <fstream>
#include <sstream>
#include <iostream>
#include<vector>

// Constructor initializes the system
LoginSystem::LoginSystem() :
    passwordHidden(true),
    isMessageSuccess(false),
    usernameActive(true),
    passwordActive(false),
    message(""),
    usernameInput(""),
    passwordInput(""),
    dataFile("users.txt") {
    loadUserData();
}

void LoginSystem::loadUserData() {
    std::ifstream file(dataFile);
    if (!file.is_open()) {
        // If file doesn't exist, create it
        std::ofstream createFile(dataFile);
        createFile.close();
        return;
    }

    std::string line, username, password;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        if (iss >> username >> password) {
            userData[username] = password;
        }
    }

    file.close();
}

void LoginSystem::resetLoginSystem() {
    usernameInput = "";
    passwordInput = "";
    message = "";
    isMessageSuccess = false;
    usernameActive = true;
    passwordActive = false;
    passwordHidden = true;
}

void LoginSystem::saveUserData() {
    std::ofstream file(dataFile, std::ios::trunc);
    if (!file.is_open()) {
        showMessage("Error saving data!", false);
        return;
    }

    for (const auto& pair : userData) {
        file << pair.first << " " << pair.second << "\n";
    }

    file.close();
}

bool LoginSystem::isMouseOver(Rectangle rect) {
    return CheckCollisionPointRec(GetMousePosition(), rect);
}

void LoginSystem::drawInputBox(float x, float y, float width, float height,
    std::string& input, bool isPassword, bool isActive, bool blinkState) {
    Rectangle box = { x, y, width, height };
    DrawRectangleRec(box, isActive ? LIGHTGRAY : DARKGRAY);
    DrawRectangleLinesEx(box, 2, isActive ? BLUE : WHITE);

    // Display input text
    std::string displayText = isPassword && passwordHidden ? std::string(input.size(), '*') : input;
    DrawText(displayText.c_str(), x + 10, y + 10, 20, BLACK);

    // Draw blinking cursor
    if (isActive && blinkState) {
        float textWidth = MeasureText(displayText.c_str(), 20);
        DrawLine(x + 10 + textWidth, y + 10, x + 10 + textWidth, y + 30, BLACK);
    }
}

void LoginSystem::drawButton(float x, float y, float width, float height,
    const std::string& text, bool isHovered) {
    Rectangle button = { x, y, width, height };
    DrawRectangleRec(button, isHovered ? DARKGRAY : GRAY);
    DrawRectangleLinesEx(button, 2, BLACK);

    // Center the text in the button
    float textWidth = MeasureText(text.c_str(), 20);
    float textX = x + (width - textWidth) / 2;
    float textY = y + (height - 20) / 2;
    DrawText(text.c_str(), textX, textY, 20, WHITE);
}

void LoginSystem::showMessage(const std::string& text, bool success) {
    message = text;
    isMessageSuccess = success;
}

void LoginSystem::attemptLogin() {
    if (validateCredentials(usernameInput, passwordInput)) {
        showMessage("Login Successful!", true);
    }
    else {
        showMessage("Invalid username or password.", false);
    }
}

void LoginSystem::attemptRegister() {
    if (usernameInput.empty() || passwordInput.empty()) {
        showMessage("Fields cannot be empty.", false);
    }
    else if (userData.find(usernameInput) != userData.end()) {
        showMessage("User already exists.", false);
    }
    else if (usernameInput.find(' ') != std::string::npos) {
        showMessage("Username cannot contain spaces.", false);
    }
    else {
        userData[usernameInput] = passwordInput;
        saveUserData();
        showMessage("Registration Successful!", true);
    }
}

void LoginSystem::handleInput(bool& usernameActive, bool& passwordActive) {
    if (usernameActive || passwordActive) {
        int key = GetCharPressed();
        while (key > 0) {
            if (key >= 32 && key <= 126) {
                if (usernameActive && key != ' ') {
                    if (usernameInput.length() < 20) { // Limit username length
                        usernameInput += (char)key;
                    }
                }
                if (passwordActive) {
                    if (passwordInput.length() < 20) { // Limit password length
                        passwordInput += (char)key;
                    }
                }
            }
            key = GetCharPressed();
        }

        if (IsKeyPressed(KEY_BACKSPACE)) {
            if (usernameActive && !usernameInput.empty()) usernameInput.pop_back();
            if (passwordActive && !passwordInput.empty()) passwordInput.pop_back();
        }
    }
}
bool LoginSystem::validateCredentials(const std::string& username, const std::string& password) {
    auto it = userData.find(username);
    return (it != userData.end() && it->second == password);
}

void LoginSystem::draw() {
    float screenWidth = GetScreenWidth();
    float screenHeight = GetScreenHeight();

    // Modern dimensions
    float inputBoxWidth = 400;
    float inputBoxHeight = 50;  // Slightly taller for better visibility
    float buttonWidth = 200;
    float buttonHeight = 55;
    float showHideButtonWidth = 100;
    float showHideButtonHeight = 50;

    // Positioning
    float centerX = screenWidth / 2;
    float startY = screenHeight / 3;  // Move everything up slightly
    float usernameX = centerX - (inputBoxWidth / 2);
    float passwordX = centerX - (inputBoxWidth / 2);
    float loginButtonX = centerX - (buttonWidth / 2);
    float togglePasswordButtonX = passwordX + inputBoxWidth + 10;

    // Create rectangles for interactive elements
    Rectangle usernameBox = { usernameX, startY, inputBoxWidth, inputBoxHeight };
    Rectangle passwordBox = { passwordX, startY + 80, inputBoxWidth, inputBoxHeight };
    Rectangle loginButton = { loginButtonX, startY + 180, buttonWidth, buttonHeight };
    Rectangle registerButton = { loginButtonX, startY + 260, buttonWidth, buttonHeight };
    Rectangle togglePasswordButton = { togglePasswordButtonX, startY + 80, showHideButtonWidth, showHideButtonHeight };

    // Background animation
    ClearBackground(BLACK);

    // Animated particle effect similar to main menu
    static std::vector<Vector2> particles;
    static std::vector<float> particleTimers;
    if (particles.empty()) {
        for (int i = 0; i < 50; i++) {
            particles.push_back({
                static_cast<float>(GetRandomValue(0, GetScreenWidth())),
                static_cast<float>(GetRandomValue(0, GetScreenHeight()))
                });
            particleTimers.push_back(GetRandomValue(0, 100) / 100.0f);
        }
    }

    // Update and draw particles
    for (size_t i = 0; i < particles.size(); i++) {
        particleTimers[i] += GetFrameTime();
        float alpha = (sin(particleTimers[i] * 2) + 1) / 2;
        DrawCircle(particles[i].x, particles[i].y, 3,
            CLITERAL(Color){255, 255, 255, static_cast<unsigned char>(alpha * 100)});
        particles[i].y -= 0.2f;
        if (particles[i].y < 0) particles[i].y = GetScreenHeight();
    }

    // Title with glow effect
    static float titleAngle = 0.0f;
    titleAngle += GetFrameTime() * 2;
    float offsetY = sin(titleAngle) * 3;

    const char* title = "TYPING MASTER";
    int titleFontSize = 48;
    float titleX = (screenWidth - MeasureText(title, titleFontSize)) / 2;
    DrawText(title, titleX + 2, 50 + offsetY + 2, titleFontSize, CLITERAL(Color){40, 40, 40, 255});
    DrawText(title, titleX, 50 + offsetY, titleFontSize, WHITE);

    // Decorative lines
    float lineLength = 400;
    float lineStartX = (screenWidth - lineLength) / 2;
    DrawLineEx(
        Vector2{ lineStartX, 120 + offsetY },
        Vector2{ lineStartX + lineLength, 120 + offsetY },
        2,
        CLITERAL(Color){150, 150, 150, 200}
    );

    // Input field labels with modern styling
    DrawText("USERNAME", usernameX, startY - 25 , 20, WHITE);
    DrawText("PASSWORD", passwordX, startY + 55, 20, WHITE);

    // Blinking cursor animation
    static bool blinkState = true;
    static float blinkTimer = 0.0f;
    blinkTimer += GetFrameTime();
    if (blinkTimer >= 0.5f) {
        blinkState = !blinkState;
        blinkTimer = 0.0f;
    }

    // Draw modern input boxes
    auto drawModernInputBox = [](Rectangle box, const std::string& text, bool isPassword, bool isHidden,
        bool isActive, bool blink) {
            // Background
            Color bgColor = isActive ? CLITERAL(Color) { 30, 30, 30, 255 } : CLITERAL(Color) { 20, 20, 20, 255 };
            Color borderColor = isActive ? WHITE : GRAY;

            DrawRectangleRec(box, bgColor);
            DrawRectangleLinesEx(box, 2, borderColor);

            // Text
            std::string displayText = isPassword && isHidden ? std::string(text.length(), '*') : text;
            DrawText(displayText.c_str(), box.x + 15, box.y + (box.height - 20) / 2, 20, WHITE);

            // Cursor
            if (isActive && blink) {
                float textWidth = MeasureText(displayText.c_str(), 20);
                DrawRectangle(box.x + 15 + textWidth, box.y + 15, 2, 20, WHITE);
            }
        };

    // Draw input fields
    drawModernInputBox(usernameBox, usernameInput, false, false, usernameActive, blinkState);
    drawModernInputBox(passwordBox, passwordInput, true, passwordHidden, passwordActive, blinkState);

    // Draw modern buttons
    auto drawModernButton = [](Rectangle btn, const char* text, bool isHovered) {
        Color bgColor = isHovered ? CLITERAL(Color) { 40, 40, 40, 255 } : CLITERAL(Color) { 20, 20, 20, 255 };
        Color borderColor = isHovered ? WHITE : GRAY;

        // Button shadow
        DrawRectangle(btn.x + 2, btn.y + 2, btn.width, btn.height, CLITERAL(Color){0, 0, 0, 100});

        // Main button
        DrawRectangleRec(btn, bgColor);
        DrawRectangleLinesEx(btn, 2, borderColor);

        // Centered text
        float textWidth = MeasureText(text, 20);
        float textX = btn.x + (btn.width - textWidth) / 2;
        float textY = btn.y + (btn.height - 20) / 2;
        DrawText(text, textX, textY, 20, WHITE);
        };

    // Draw buttons
    drawModernButton(loginButton, "LOGIN", isMouseOver(loginButton));
    drawModernButton(registerButton, "REGISTER", isMouseOver(registerButton));
    drawModernButton(togglePasswordButton, passwordHidden ? "SHOW" : "HIDE", isMouseOver(togglePasswordButton));

    // Message display with animation
    if (!message.empty()) {
        static float messageAlpha = 0.0f;
        messageAlpha = messageAlpha + (1.0f - messageAlpha) * 0.1f;
        Color messageColor = isMessageSuccess ?
            CLITERAL(Color) { 0, 255, 0, static_cast<unsigned char>(messageAlpha * 255) } :
            CLITERAL(Color) { 255, 0, 0, static_cast<unsigned char>(messageAlpha * 255) };

        float messageY = startY + 340;
        DrawText(message.c_str(),
            (screenWidth - MeasureText(message.c_str(), 20)) / 2,
            messageY, 20, messageColor);
    }

    // Footer with subtle animation
    float footerPulse = (sin(GetTime() * 2) + 1) / 2;
    DrawText("Developed by BCSF23M023 & BCSF23M048",
        (screenWidth - MeasureText("Developed by BCSF23M023 & BCSF23M048", 20)) / 2,
        screenHeight - 40, 20,
        CLITERAL(Color){150, 150, 150, static_cast<unsigned char>(footerPulse * 200)});

    // Handle input
    handleInput(usernameActive, passwordActive);

    // Handle interaction
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        usernameActive = isMouseOver(usernameBox);
        passwordActive = isMouseOver(passwordBox);

        if (isMouseOver(togglePasswordButton)) {
            passwordHidden = !passwordHidden;
        }
        if (isMouseOver(loginButton)) {
            attemptLogin();
        }
        else if (isMouseOver(registerButton)) {
            attemptRegister();
        }
    }

    if (IsKeyPressed(KEY_TAB)) {
        if (usernameActive) {
            usernameActive = false;
            passwordActive = true;
        }
        else if (passwordActive) {
            passwordActive = false;
            usernameActive = true;
        }
    }

    if (IsKeyPressed(KEY_ENTER)) {
        attemptLogin();
    }
}