#include "MainMenu.h"

MainMenu::MainMenu() :
    currentState(MenuState::LOGIN),
    typingTest(nullptr),
    stats(nullptr),
    game(nullptr),
    isLoggedIn(false),
    shouldClose(false) {

    InitWindow(1280, 800, "Typing Master");
    SetTargetFPS(60);
    SetExitKey(0);
}

MainMenu::~MainMenu() {
    cleanup();
    CloseWindow();
}

void MainMenu::drawButton(Rectangle rect, const char* text, bool isHovered, float animationProgress) {
    Color bgColor = isHovered ?
        CLITERAL(Color) { 40, 40, 40, 255 } :
        CLITERAL(Color) { 0, 0, 0, 255 };
    Color textColor = isHovered ? BLUE : WHITE;
    Color borderColor = isHovered ? WHITE : BLUE;

    float expandAmount = isHovered ? 5.0f * animationProgress : 0;
    Rectangle animatedRect = {
        rect.x - expandAmount,
        rect.y - expandAmount / 2,
        rect.width + (expandAmount * 2),
        rect.height + expandAmount
    };

    float pulseIntensity = (1.0f + sin(GetTime() * 2)) / 2.0f;
    if (isHovered) {
        for (int i = 0; i < 3; i++) {
            float glowSize = (3 - i) * 2 * pulseIntensity;
            DrawRectangleRec(
                Rectangle {
                animatedRect.x - glowSize,
                    animatedRect.y - glowSize,
                    animatedRect.width + (glowSize * 2),
                    animatedRect.height + (glowSize * 2)
            },
                CLITERAL(Color) {
                255, 255, 255, (unsigned char)(20 - i * 5)
            }
            );
        }
    }

    // Main button
    DrawRectangleRec(animatedRect, bgColor);
    DrawRectangleLinesEx(animatedRect, 2, borderColor);

    // Dynamic text styling
    int fontSize = 28;
    Vector2 textSize = MeasureTextEx(GetFontDefault(), text, fontSize, 1);
    Vector2 textPos = {
        animatedRect.x + (animatedRect.width - textSize.x) / 2,
        animatedRect.y + (animatedRect.height - textSize.y) / 2
    };

    // Text glow effect
    if (isHovered) {
        float glowIntensity = (pulseIntensity * 0.5f + 0.5f) * 255;
        DrawText(text, textPos.x + 1, textPos.y + 1, fontSize,
            CLITERAL(Color){200, 200, 200, (unsigned char)glowIntensity});
    }
    DrawText(text, textPos.x, textPos.y, fontSize, textColor);
}
bool MainMenu::isMouseOverButton(Rectangle rect) const {
    Vector2 mousePos = GetMousePosition();
    return CheckCollisionPointRec(mousePos, rect);
}

void MainMenu::drawMainMenuButtons() {
    static float animationProgress = 0.0f;
    static float titleAngle = 0.0f;

    // Update animations
    animationProgress = fmin(1.0f, animationProgress + GetFrameTime() * 2);
    titleAngle += GetFrameTime() * 2;

    // Background with dynamic pattern
    ClearBackground(BLACK);
    drawParticleEffect();

    // Dynamic grid pattern
    for (int i = 0; i < GetScreenWidth(); i += 40) {
        for (int j = 0; j < GetScreenHeight(); j += 40) {
            float distance = sqrt(pow(i - GetMousePosition().x, 2) +
                pow(j - GetMousePosition().y, 2));
            float intensity = fmax(0, 1 - (distance / 300));
            DrawPixel(i, j, CLITERAL(Color){50, 50, 50,
                (unsigned char)(intensity * 255)});
        }
    }

    // Welcome message
    std::string welcomeMsg = "WELCOME, " + username + " !";
    int welcomeFontSize = 40;
    Vector2 welcomeSize = MeasureTextEx(GetFontDefault(), welcomeMsg.c_str(),
        welcomeFontSize, 1);
    float welcomeX = 50.0f;
    float welcomeY = 50.0f;

    // Welcome message with floating animation
    float offsetY = sin(titleAngle) * 5;
    DrawText(welcomeMsg.c_str(), welcomeX + 2, welcomeY + offsetY + 2,
        welcomeFontSize, CLITERAL(Color){50, 50, 50, 255});
    DrawText(welcomeMsg.c_str(), welcomeX, welcomeY + offsetY, welcomeFontSize, WHITE);

    // Animated decorative lines
    float lineLength = welcomeSize.x + 50.0f * animationProgress;
    float lineStartX = welcomeX;

    // Animated double line design
    for (int i = 0; i < 2; i++) {
        float lineOffset = sin(titleAngle + i) * 2;
        DrawLineEx(
            Vector2{ lineStartX, 110.0f + lineOffset },
            Vector2{ lineStartX + lineLength, 110.0f + lineOffset },
            1,
            CLITERAL(Color){150, 150, 150, 200}
        );
    }

    // Fixed button layout with proper float conversions
    float buttonWidth = 300.0f;
    float buttonHeight = 70.0f;
    float startY = 200.0f;
    float spacing = 100.0f;
    float centerX = (static_cast<float>(GetScreenWidth()) - buttonWidth) / 2.0f;

    // Store button rectangles with proper float values
    typingTestBtn = {
        centerX,
        startY,
        buttonWidth,
        buttonHeight
    };

    gameBtn = {
        centerX,
        startY + spacing,
        buttonWidth,
        buttonHeight
    };

    statsBtn = {
        centerX,
        startY + spacing * 2.0f,
        buttonWidth,
        buttonHeight
    };

    logoutBtn = {
        centerX,
        startY + spacing * 3.0f,
        buttonWidth,
        buttonHeight
    };

    // Draw buttons with hover states
    bool typingTestHovered = isMouseOverButton(typingTestBtn);
    bool gameHovered = isMouseOverButton(gameBtn);
    bool statsHovered = isMouseOverButton(statsBtn);
    bool logoutHovered = isMouseOverButton(logoutBtn);

    // Draw buttons with proper animations
    drawButton(typingTestBtn, "TYPING TEST", typingTestHovered, animationProgress);
    drawButton(gameBtn, "WORD GAME", gameHovered, animationProgress);
    drawButton(statsBtn, "STATISTICS", statsHovered, animationProgress);
    drawButton(logoutBtn, "LOGOUT", logoutHovered, animationProgress);

    // Button descriptions
    /*if (typingTestHovered)
        DrawText("Test your typing speed and accuracy",
            static_cast<int>(centerX - 100), static_cast<int>(startY + buttonHeight + 5), 16, GRAY);
    if (gameHovered)
        DrawText("Challenge yourself with falling words",
            static_cast<int>(centerX - 90), static_cast<int>(startY + spacing + buttonHeight + 5), 16, GRAY);
    if (statsHovered)
        DrawText("View your performance metrics",
            static_cast<int>(centerX - 80), static_cast<int>(startY + spacing * 2 + buttonHeight + 5), 16, GRAY);
    if (logoutHovered)
        DrawText("Return to login screen",
            static_cast<int>(centerX - 60), static_cast<int>(startY + spacing * 3 + buttonHeight + 5), 16, GRAY);
*/
    // Interactive footer
    float footerPulse = (sin(GetTime() * 2) + 1) / 2;
    DrawText("Press ESC to exit",
        10,
        GetScreenHeight() - 30,
        20,
        CLITERAL(Color){100, 100, 100, (unsigned char)(footerPulse * 255)});

    handleMainMenuInput();
}

void MainMenu::drawParticleEffect() {
    static std::vector<Vector2> particles;
    static std::vector<float> particleTimers;

    // Initialize larger particles
    if (particles.empty()) {
        for (int i = 0; i < 50; i++) {
            particles.push_back({
                (float)(GetRandomValue(0, GetScreenWidth())),
                (float)(GetRandomValue(0, GetScreenHeight()))
                });
            particleTimers.push_back(GetRandomValue(0, 100) / 100.0f);
        }
    }

    // Update and draw larger particles
    for (size_t i = 0; i < particles.size(); i++) {
        particleTimers[i] += GetFrameTime();
        float alpha = (sin(particleTimers[i] * 2) + 1) / 2;

        // Increased particle size to 3
        DrawCircle(particles[i].x, particles[i].y, 3,
            CLITERAL(Color){255, 255, 255, (unsigned char)(alpha * 100)});

        // Slowly move particles
        particles[i].y -= 0.2f;
        if (particles[i].y < 0) particles[i].y = GetScreenHeight();
    }
}
// The rest of the methods remain the same as they handle logic rather than UI
void MainMenu::handleMainMenuInput() {
    // Handle ESC key for exit
    if (IsKeyPressed(KEY_ESCAPE)) {
        shouldClose = true;
        return;
    }

    // Handle button clicks with fixed positioning
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        Vector2 mousePos = GetMousePosition();

        if (CheckCollisionPointRec(mousePos, typingTestBtn)) {
            cleanup();
            typingTest = new TypingTest(username);
            typingTest->setTestActive(true);
            currentState = MenuState::TYPING_TEST;
        }
        else if (CheckCollisionPointRec(mousePos, gameBtn)) {
            cleanup();
            game = new FallingWordsGame(username);
            game->StartGame();
            currentState = MenuState::GAME;
        }
        else if (CheckCollisionPointRec(mousePos, statsBtn)) {
            cleanup();
            stats = new Stats(username);
            currentState = MenuState::STATS;
        }
        else if (CheckCollisionPointRec(mousePos, logoutBtn)) {
            handleLogout();
        }
    }
}

void MainMenu::handleLogout() {
    cleanup();
    isLoggedIn = false;
    username = "";
    loginSystem.resetLoginSystem();
    currentState = MenuState::LOGIN;
}

void MainMenu::cleanup() {
    if (typingTest) {
        delete typingTest;
        typingTest = nullptr;
    }
    if (game) {
        delete game;
        game = nullptr;
    }
    if (stats) {
        delete stats;
        stats = nullptr;
    }
}

void MainMenu::run() {
    while (!WindowShouldClose() && !shouldClose) {
        BeginDrawing();

        switch (currentState) {
        case MenuState::LOGIN:
            handleLogin();
            break;

        case MenuState::MAIN_MENU:
            drawMainMenuButtons();
            break;

        case MenuState::TYPING_TEST:
            if (typingTest) {
                if (!typingTest->isTestActive()) {
                    cleanup();
                    currentState = MenuState::MAIN_MENU;
                }
                else {
                    typingTest->update();
                    typingTest->render();
                }
            }
            break;

        case MenuState::GAME:
            if (game) {
                if (IsKeyPressed(KEY_ESCAPE) && !game->IsPaused()) {
                    cleanup();
                    currentState = MenuState::MAIN_MENU;
                }
                else {
                    game->UpdateGame();
                    game->DrawGame(false);

                    if (!game->IsRunning()) {
                        if (IsKeyPressed(KEY_ENTER) || IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                            cleanup();
                            currentState = MenuState::MAIN_MENU;
                        }
                    }
                }
            }
            break;

        case MenuState::STATS:
            drawStats();
            if (IsKeyPressed(KEY_ESCAPE)) {
                currentState = MenuState::MAIN_MENU;
            }
            break;
        }

        EndDrawing();
    }
}

void MainMenu::handleLogin() {
    loginSystem.draw();
    if (loginSystem.getLoginSuccessStatus()) {
        username = loginSystem.getUsernameInput();
        isLoggedIn = true;
        currentState = MenuState::MAIN_MENU;
    }
}

void MainMenu::drawStats() {
    if (!stats) {
        stats = new Stats(username);
    }

    stats->draw();
    if (stats->handleInput()) {
        delete stats;
        stats = nullptr;
        currentState = MenuState::MAIN_MENU;
    }
}