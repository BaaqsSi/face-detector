#include <windows.h>
#include <tchar.h>
#include <opencv2/opencv.hpp>
#include <opencv2/objdetect.hpp>
#include <wchar.h>
#include <string>
#include <filesystem>
//globalze declare vuketebt yleobebs
cv::VideoCapture op;
cv::Mat frame;
cv::CascadeClassifier face_casc;


bool isCameraAvailable() {
    for (int i = 0; i < 10; i++) { 
        cv::VideoCapture tempCamera(i);
        if (tempCamera.isOpened()) {
            tempCamera.release();
            return true; 
        }
    }
    return false; 
}

//exe file gaxsna run is dacherisas
void openexe(const std::string& targetFileName) {
    char exePath[MAX_PATH];

    if (GetModuleFileNameA(NULL, exePath, MAX_PATH) == 0) {
        std::cerr << "Failed to get executable path. Error code: " << GetLastError() << std::endl;
        return;
    }

    std::filesystem::path exeDir = std::filesystem::path(exePath).parent_path();
    std::filesystem::path targetFilePath = exeDir / targetFileName;

    if (!std::filesystem::exists(targetFilePath)) {
        std::cerr << "Executable file not found: " << targetFilePath << std::endl;
        return;
    }

    int result = system(targetFilePath.string().c_str());
    if (result == 0) {
        std::cout << "Execution successful: " << targetFilePath << std::endl;
    }
    else {
        std::cerr << "Failed to open executable file: " << targetFilePath << std::endl;
    }
}


LRESULT CALLBACK WindowProc(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam) {
    switch (umsg) {
    case WM_CLOSE:
        PostQuitMessage(0);
        break;

    case WM_DESTROY:
        return 0;

    case WM_CREATE: {


        int screenWidth = 500;
        int screenHeight = 500;


        int buttonWidth = 100, buttonHeight = 30;
        int buttonwidth1 = 80, buttonheight1 = 30;


        int centerX = (screenWidth - buttonWidth) / 2;
        int centerY = (screenHeight - buttonHeight) / 2;

        //close button marjvena zeda kutxeshi
        CreateWindow(
            L"BUTTON",
            L"Close",
            WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
            screenWidth - buttonwidth1 - 5, 5, buttonwidth1, buttonheight1,  
            hwnd,
            (HMENU)1,
            ((LPCREATESTRUCT)lparam)->hInstance,
            NULL
        );

        //open camera qveda marcxena kutxeshi
        CreateWindow(
            L"BUTTON",
            L"open camera",
            WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
            centerX-180, centerY+150,
            buttonWidth, buttonHeight,
            hwnd,
            (HMENU)3,
            ((LPCREATESTRUCT)lparam)->hInstance,
            NULL
        );

        //shuashi take a screenshot button
        CreateWindow(
            L"BUTTON",
            L"take a screenshot",
            WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
            centerX - 50, centerY + 150,
            buttonWidth+25, buttonHeight,
            hwnd,
            (HMENU)4,
            ((LPCREATESTRUCT)lparam)->hInstance,
            NULL
        );

        //marvjniv myofi mtavari run function
        CreateWindow(
            L"BUTTON",
            L"run",
            WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
            centerX + 140, centerY + 150,
            buttonWidth + 25, buttonHeight,
            hwnd,
            (HMENU)5,
            ((LPCREATESTRUCT)lparam)->hInstance,
            NULL
        );

    }
                  break;
    //functionebi buttonebze
    case WM_COMMAND:
        //close button-is function
        if (LOWORD(wparam) == 1) {
            if (op.isOpened()) {
                MessageBox(hwnd, L"Please close the camera first before exiting.", L"Camera Open", MB_OK | MB_ICONWARNING);
                break;
            }
            PostQuitMessage(0);

        }
        //open camera function
        else if (LOWORD(wparam) == 3) {
            op.open(0);

            if (!face_casc.load("haarcascade_frontalface_default.xml")) {
                MessageBox(hwnd, L"Failed to load cascade classifier!", L"Error", MB_OK | MB_ICONERROR);
                return 0;
            }
            MessageBox(hwnd, L"press q to turn it off ", L"Camera Opened", MB_OK | MB_ICONINFORMATION);
            while (true) {
                op >> frame; 
                std::vector<cv::Rect> faces;
                cv::Mat resizedFrame;
                cv::resize(frame, resizedFrame, cv::Size(), 0.5, 0.5);
                face_casc.detectMultiScale(resizedFrame, faces, 1.1, 3, 0, cv::Size(30, 30));

                if (frame.empty()) {
                    break; 
                    return 0;
                }
                for (const auto& face : faces) {
                    cv::rectangle(frame, cv::Rect(face.x * 2, face.y * 2, face.width * 2, face.height * 2),
                        cv::Scalar(255, 0, 0), 2);
                }
                cv::imshow("Face Detector", frame);

                if (cv::waitKey(30) == 'q') {
                    break;
                }

                MSG msg;
                while (PeekMessage(&msg, hwnd, 0, 0, PM_REMOVE)) {
                    TranslateMessage(&msg);
                    DispatchMessage(&msg);
                }
            }
            op.release();
            cv::destroyAllWindows(); 
            return 0;
        }
        //take a screenshot function
        else if (LOWORD(wparam) == 4) {
            if (op.isOpened()) {
                op >> frame;
                cv::imwrite("Screenshot.png", frame);
            }
            else {
                MessageBox(hwnd, L"please open the camera first..", L"Camera is not Opened", MB_OK | MB_ICONWARNING);

            }
        }

        //run function
        else if (LOWORD(wparam) == 5) {
            // vnaxulobt tu kamera chartulia radganac kameris gareshe pc veghar gaixsneba safe mode gareshe
            if (!isCameraAvailable()) {
                MessageBox(hwnd, L"No camera detected on this PC. Please connect a camera to proceed.", L"Camera Not Found", MB_OK | MB_ICONERROR);
                break;
            }
            if (!face_casc.load("haarcascade_frontalface_default.xml")) {
                MessageBox(hwnd, L"Failed to load the cascade classifier!", L"Error", MB_OK | MB_ICONERROR);
                break; // Exit if the classifier cannot be loaded
            }

            // frame wavikitxot screenshotidan
            cv::Mat img = cv::imread("Screenshot.png");

            // vnaxot tu screenshot empty aris frame wakitxvis mere
            if (img.empty()) {
                MessageBox(hwnd, L"Please take a screenshot first..", L"Screenshot not found", MB_OK | MB_ICONWARNING);
                break;
            }

            // vnaxulobt tu face moizebneba face shi
            std::vector<cv::Rect> faces;
            cv::Mat resizedFrame;
            cv::resize(img, resizedFrame, cv::Size(), 0.5, 0.5);
            face_casc.detectMultiScale(resizedFrame, faces, 1.1, 3, 0, cv::Size(30, 30));

            // vnaxulobt tu rame face aris detected vectorshi
            if (faces.empty()) {
                MessageBox(hwnd, L"Can't detect a face in the screenshot. Please capture a new one.", L"Face not found in screenshot", MB_OK | MB_ICONERROR);
                break;
            }

            // tu kamera gaixsna  ,img ipova da aseve face moizebna image-shi ixsneba mtavari exe file.
            openexe("opencv.exe");
        }



        break;

    default:
        return DefWindowProc(hwnd, umsg, wparam, lparam);
    }
    return 0;
}
//mtavari funkcia
int WINAPI WinMain(HINSTANCE hinstance, HINSTANCE, LPSTR,int nCmdShow) {
    const wchar_t class_name[] = L"window";

    WNDCLASS wc = {};
    wc.lpfnWndProc = WindowProc;     
    wc.hInstance = hinstance;
    wc.lpszClassName = class_name;
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);


    RegisterClass(&wc);

    int screenWidth = 500;
    int screenHeight = 500;

    HWND hwnd = CreateWindowEx(
        WS_EX_TOPMOST,
        class_name,
        L"camera managment",
        WS_POPUP | WS_VISIBLE,
        0, 0,
        screenWidth,
        screenHeight,
        NULL,
        NULL,
        hinstance,
        NULL
       );
    if (hwnd == NULL) {
        return 0;
    }
    RECT rect;
    GetClientRect(GetDesktopWindow(), &rect);
    int screenWidthFull = rect.right;
    int screenHeightFull = rect.bottom;

    int posX = (screenWidthFull - screenWidth) / 2;
    int posY = (screenHeightFull - screenHeight) / 2;

    SetWindowPos(hwnd, HWND_TOP, posX, posY, 0, 0, SWP_NOSIZE);
    ShowWindow(hwnd, nCmdShow); 

    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
        
    return (int)msg.wParam;
}