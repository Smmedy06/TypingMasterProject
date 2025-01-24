#pragma once
#include "LoginSystem.h"
#include "Stats.h"
#include "TypingTest.h"
#include "Games.h"
#include "raylib.h"
#include <string>

enum class MenuState {
    LOGIN,
    MAIN_MENU,
    TYPING_TEST,
    GAME,
    STATS
};

class MainMenu {
private:
    MenuState currentState;
    LoginSystem loginSystem;
    std::string username;
    TypingTest* typingTest;
    Stats* stats;
    FallingWordsGame* game;
    bool isLoggedIn;
    bool shouldClose;
    Rectangle typingTestBtn;
    Rectangle gameBtn;
    Rectangle statsBtn;
    Rectangle logoutBtn;
    
    // Helper methods for cleanup and state management
    void cleanup();  // New cleanup method
    void handleLogout();  // Added logout handler
    
    // UI Elements
    void drawParticleEffect();
    void drawMainMenuButtons();
    bool isMouseOverButton(Rectangle rect) const;
    void drawButton(Rectangle rect, const char* text, bool isHovered, float animationProgress);
    void handleMainMenuInput();
    void drawStats();
    void handleLogin();

public:
    MainMenu();
    ~MainMenu();
    void run();
    void setState(MenuState newState);
    MenuState getState() const { return currentState; }
};