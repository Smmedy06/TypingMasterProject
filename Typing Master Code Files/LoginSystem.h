#pragma once
#include <string>
#include <map>
#include <raylib.h>

class LoginSystem {
private:
    std::map<std::string, std::string> userData;
    std::string usernameInput;
    std::string passwordInput;
    std::string message;
    bool passwordHidden;
    bool isMessageSuccess;
    const std::string dataFile = "user.txt";
    bool usernameActive;
    bool passwordActive;

    void attemptLogin();
    void attemptRegister();

public:
    LoginSystem();
    void loadUserData();
    void saveUserData();
    void resetLoginSystem();
    bool isMouseOver(Rectangle rect);
    void drawInputBox(float x, float y, float width, float height, std::string& input, bool isPassword, bool isActive, bool blinkState);
    void drawButton(float x, float y, float width, float height, const std::string& text, bool isHovered);
    void showMessage(const std::string& text, bool success);
    void draw(); // Changed from run() to draw()
    bool validateCredentials(const std::string& username, const std::string& password);
    bool getLoginSuccessStatus() const { return isMessageSuccess; }
    std::string getUsernameInput() const { return usernameInput; }
    void handleInput(bool& usernameActive, bool& passwordActive); // New method
};