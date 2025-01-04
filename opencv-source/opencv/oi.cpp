#include <opencv2/opencv.hpp>
#include <opencv2/objdetect.hpp>
#include <iostream>
#include <chrono>
#include <windows.h>
#include <filesystem>
#include <GL/glew.h> 
#include <GLFW/glfw3.h> 
#include <GL/gl.h>  
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>  
HHOOK hKeyboardHook;

LRESULT CALLBACK KeyboardHookProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode == HC_ACTION) {
        KBDLLHOOKSTRUCT* kbStruct = (KBDLLHOOKSTRUCT*)lParam;

        if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN) {
            switch (kbStruct->vkCode) {
            case VK_TAB:
            case VK_LSHIFT:
            case VK_RSHIFT:
            case VK_LWIN:
            case VK_RWIN:
            case VK_MENU:
            case VK_ESCAPE:
            case VK_F4:
                return 1;
            default:
                break;
            }
        }
    }
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}

void SetKeyboardHook() {
    hKeyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardHookProc, NULL, 0);
    if (hKeyboardHook == NULL) {
        std::cerr << "Failed to set keyboard hook!" << std::endl;
    }
}

void RemoveKeyboardHook() {
    if (hKeyboardHook) {
        UnhookWindowsHookEx(hKeyboardHook);
        hKeyboardHook = NULL;
    }
}

void AddToStartup() {
    char exePath[MAX_PATH];
    if (GetModuleFileNameA(NULL, exePath, MAX_PATH) == 0) {
        std::cerr << "Failed to get executable path. Error code: " << GetLastError() << std::endl;
        return;
    }

    HKEY hKey;
    const char* keyPath = "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run";
    if (RegOpenKeyExA(HKEY_CURRENT_USER, keyPath, 0, KEY_SET_VALUE, &hKey) != ERROR_SUCCESS) {
        std::cerr << "Failed to open registry key. Error code: " << GetLastError() << std::endl;
        return;
    }

    const char* appName = "FullscreenLoginApp";
    if (RegSetValueExA(hKey, appName, 0, REG_SZ, (const BYTE*)exePath, strlen(exePath) + 1) != ERROR_SUCCESS) {
        std::cerr << "Failed to set registry value. Error code: " << GetLastError() << std::endl;
    }
    else {
        std::cout << "Application successfully added to startup." << std::endl;
    }

    RegCloseKey(hKey);
}

int main() {
    AddToStartup();

    if (!glfwInit()) {
        return -1;
    }

    int windowWidth = 1920;
    int windowHeight = 1080;
    glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);
    glfwWindowHint(GLFW_FLOATING, GLFW_TRUE);
    glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);

    GLFWwindow* window = glfwCreateWindow(windowWidth, windowHeight, "Face Detection", NULL, NULL);
    if (!window) {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glewInit();

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    SetKeyboardHook();

    cv::Mat img1 = cv::imread("Screenshot.png");
    if (img1.empty()) {
        std::cerr << "Couldn't read an image. Please take a screenshot first." << std::endl;
        return -1;
    }

    cv::CascadeClassifier face_casc;
    if (!face_casc.load("haarcascade_frontalface_default.xml")) {
        std::cerr << "Failed to load cascade classifier!" << std::endl;
        return -1;
    }

    cv::Mat gray1;
    cv::cvtColor(img1, gray1, cv::COLOR_BGR2GRAY);
    std::vector<cv::Rect> faces1;
    face_casc.detectMultiScale(gray1, faces1);

    if (faces1.empty()) {
        std::cerr << "No face detected in the reference image!" << std::endl;
        return -1;
    }

    cv::Mat face1 = gray1(faces1[0]);
    cv::Mat resized_face;
    cv::resize(face1, resized_face, cv::Size(200, 200));

    cv::Mat hist1;
    int histsize = 256;
    float range[] = { 0, 256 };
    const float* histrange = { range };
    cv::calcHist(&resized_face, 1, 0, cv::Mat(), hist1, 1, &histsize, &histrange);
    cv::normalize(hist1, hist1, 0, 1, cv::NORM_MINMAX);

    cv::VideoCapture op(0);
    if (!op.isOpened()) {
        std::cerr << "Failed to open the webcam." << std::endl;
        return 0;
    }

    std::cout << "Webcam opened successfully!" << std::endl;

    auto last_check_time = std::chrono::steady_clock::now();
    auto last_frame_time = std::chrono::steady_clock::now();
    int check_interval_ms = 2000;
    int no_frame_timeout_ms = 3000;  // 3 seconds timeout
    bool is_frame_received = false;

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        cv::Mat kok;
        op >> kok;

        if (kok.empty()) {
            std::cerr << "Failed to capture webcam frame." << std::endl;
            is_frame_received = false;
        }
        else {
            is_frame_received = true;
            last_frame_time = std::chrono::steady_clock::now();
        }

        if (!is_frame_received) {
            auto now = std::chrono::steady_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - last_frame_time).count();
            if (elapsed >= no_frame_timeout_ms) {
                std::cerr << "No frame received within 3 seconds. Exiting program." << std::endl;
                break;
            }
        }

        cv::Mat gray2;
        cv::cvtColor(kok, gray2, cv::COLOR_BGR2GRAY);

        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - last_check_time).count();

        if (elapsed >= check_interval_ms) {
            last_check_time = now;
            std::vector<cv::Rect> faces2;
            face_casc.detectMultiScale(gray2, faces2);

            if (!faces2.empty()) {
                cv::Mat face2 = gray2(faces2[0]);
                cv::Mat resized_face2;
                cv::resize(face2, resized_face2, cv::Size(200, 200));

                cv::Mat hist2;
                cv::calcHist(&resized_face2, 1, 0, cv::Mat(), hist2, 1, &histsize, &histrange);
                cv::normalize(hist2, hist2, 0, 1, cv::NORM_MINMAX);

                double similarity = cv::compareHist(hist1, hist2, cv::HISTCMP_CORREL);
                std::cout << "Similarity of faces: " << similarity << std::endl;

                if (similarity > 0.5) {
                    glfwTerminate();
                    break;
                }
                else {
                    std::cout << "Not similar" << std::endl;
                    continue;
                }
            }
            else {
                continue;
            }
        }

        GLuint textureID;
        glGenTextures(1, &textureID);
        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        cv::Mat flipped_kok;
        cv::flip(kok, flipped_kok, 0);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, flipped_kok.cols, flipped_kok.rows, 0, GL_RGB, GL_UNSIGNED_BYTE, flipped_kok.data);

        glViewport(0, 0, windowWidth, windowHeight);
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, textureID);

        glBegin(GL_QUADS);
        glTexCoord2f(0.0f, 0.0f); glVertex2f(-1.0f, -1.0f);
        glTexCoord2f(1.0f, 0.0f); glVertex2f(1.0f, -1.0f);
        glTexCoord2f(1.0f, 1.0f); glVertex2f(1.0f, 1.0f);
        glTexCoord2f(0.0f, 1.0f); glVertex2f(-1.0f, 1.0f);
        glEnd();

        glDisable(GL_TEXTURE_2D);
        glfwSwapBuffers(window);
    }
    RemoveKeyboardHook();
    glfwTerminate();
    return 0;
}
