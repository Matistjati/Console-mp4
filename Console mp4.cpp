#include <Magick++.h> 
#include <iostream> 
#include <windows.h>
#include <string>
#include <vector>
#include <chrono>
#include <thread>

#include <Mmsystem.h>
#include <mciapi.h>
//these two headers are already included in the <Windows.h> header
#pragma comment(lib, "Winmm.lib")

using std::chrono::high_resolution_clock;
using std::chrono::duration_cast;

#define profile_time 0

void set_console_size(HANDLE screen_buffer, SHORT width, SHORT height)
{

    COORD const size = { width, height };
    BOOL success;

    SMALL_RECT const minimal_window = { 0, 0, 1, 1 };
    success = SetConsoleWindowInfo(screen_buffer, TRUE, &minimal_window);
    if (!success)
    {
        std::cout << GetLastError();
    }

    success = SetConsoleScreenBufferSize(screen_buffer, size);
    if (!success)
    {
        std::cout << GetLastError();
    }

    SMALL_RECT const window = { 0, 0, size.X - 1, size.Y - 1 };
    success = SetConsoleWindowInfo(screen_buffer, TRUE, &window);
    if (!success)
    {
        std::cout << GetLastError();
    }
}

// Get the horizontal and vertical screen sizes in pixel
void GetDesktopResolution(int& horizontal, int& vertical)
{
    RECT desktop;
    // Get a handle to the desktop window
    const HWND hDesktop = GetDesktopWindow();
    // Get the size of screen to the variable desktop
    GetWindowRect(hDesktop, &desktop);
    // The top left corner will have coordinates (0,0)
    // and the bottom right corner will have coordinates
    // (horizontal, vertical)
    horizontal = desktop.right;
    vertical = desktop.bottom;
}

std::wstring s2ws(const std::string& s)
{
    int len;
    int slength = (int)s.length() + 1;
    len = MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, 0, 0);
    wchar_t* buf = new wchar_t[len];
    MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, buf, len);
    std::wstring r(buf);
    delete[] buf;
    return r;
}


using namespace std;
using namespace Magick;

int main(int argc, char** argv)
{

    std::vector<std::pair<float, char>> characterDensities = {
        { 0.0, ' ' },
        { 0.06041666666666666, '-' },
        { 0.0725, '\''},
        {0.0725, '.'},
        {0.0725, '`'},
        {0.09666666666666666, '_'},
        {0.145, ':'},
        {0.15708333333333332, ','},
        {0.16916666666666666, '='},
        {0.18125, '\"'},
        {0.20541666666666666, '^'},
        {0.2295833333333333, ';'},
        {0.24166666666666664, '!'},
        {0.24166666666666664, '+'},
        {0.24166666666666664, '~'},
        {0.2658333333333333, 'L'},
        {0.29, '<'},
        {0.29, 'T'},
        {0.29, '\\'},
        {0.29, 'c'},
        {0.29, 'z'},
        {0.3020833333333333, 'r'},
        {0.31416666666666665, '/'},
        {0.31416666666666665, 'F'},
        {0.31416666666666665, 'I'},
        {0.31416666666666665, 'i'},
        {0.31416666666666665, 'l'},
        {0.32625, '1'},
        {0.32625, '7'},
        {0.32625, '>'},
        {0.32625, 'J'},
        {0.32625, 't'},
        {0.3383333333333333, '*'},
        {0.3383333333333333, '|'},
        {0.35041666666666665, '?'},
        {0.3625, 'E'},
        {0.3625, '['},
        {0.3625, 's'},
        {0.3745833333333334, '('},
        {0.3745833333333334, 'Z'},
        {0.3745833333333334, 'f'},
        {0.3745833333333334, 'j'},
        {0.3745833333333334, 'v'},
        {0.38666666666666666, ')'},
        {0.38666666666666666, ']'},
        {0.38666666666666666, 'n'},
        {0.38666666666666666, 'u'},
        {0.39875, '2'},
        {0.39875, '5'},
        {0.41083333333333333, 'Y'},
        {0.42291666666666666, '3'},
        {0.42291666666666666, 'x'},
        {0.42291666666666666, '}'},
        {0.435, 'e'},
        {0.44708333333333333, '4'},
        {0.44708333333333333, 'C'},
        {0.44708333333333333, 'P'},
        {0.44708333333333333, '{'},
        {0.4591666666666666, 'S'},
        {0.4591666666666666, 'a'},
        {0.4591666666666666, 'h'},
        {0.47125, 'H'},
        {0.47125, 'V'},
        {0.47125, 'w'},
        {0.4833333333333333, 'k'},
        {0.4833333333333333, 'o'},
        {0.49541666666666667, 'A'},
        {0.49541666666666667, 'b'},
        {0.49541666666666667, 'p'},
        {0.5075, 'K'},
        {0.5075, 'U'},
        {0.5075, 'y'},
        {0.5195833333333333, 'G'},
        {0.54375, '#'},
        {0.54375, 'X'},
        {0.54375, 'd'},
        {0.54375, 'q'},
        {0.5558333333333334, 'R'},
        {0.5679166666666666, '6'},
        {0.5679166666666666, '9'},
        {0.5679166666666666, 'D'},
        {0.5920833333333333, 'B'},
        {0.6041666666666666, 'N'},
        {0.6041666666666666, 'm'},
        {0.61625, '8'},
        {0.6283333333333333, '$'},
        {0.6283333333333333, 'M'},
        {0.6283333333333333, 'O'},
        {0.6283333333333333, 'W'},
        {0.6404166666666666, '0'},
        {0.6645833333333333, '&'},
        {0.7008333333333333, '%'},
        {0.7008333333333333, 'Q'},
        {0.7008333333333333, 'g'},
        {0.9666666666666666, '@'},
        {1, '@'} }; //█


    std::map<int, char> shadeToChar = {};

    for (size_t i = 0; i < 256; i++)
    {
        float v = float(i) / 255;

        char bestChar = '@';
        float bestValue = 1e9;
        for (size_t j = 0; j < characterDensities.size(); j++)
        {
            if (abs(v - characterDensities[j].first) < bestValue)
            {
                bestChar = characterDensities[j].second;
                bestValue = abs(v - characterDensities[j].first);
            }
        }

        shadeToChar[i] = bestChar;
    }

    const double fps = 30;
    const double timePerFrame = 1 / 30.;
    InitializeMagick(*argv);

    HANDLE handle;

    handle = GetStdHandle(-11);

    int desktopWidth, desktopHeight;
    GetDesktopResolution(desktopWidth, desktopHeight);

    CONSOLE_FONT_INFOEX fontInfo;
    fontInfo.cbSize = sizeof(CONSOLE_FONT_INFOEX);

    GetCurrentConsoleFontEx(handle, false, &fontInfo);

    int c_columns = int((desktopHeight * 0.93)) / fontInfo.dwFontSize.Y;
    int c_rows = (int(desktopWidth * 0.95)) / fontInfo.dwFontSize.X;

    CONSOLE_SCREEN_BUFFER_INFO csbi;


    //SetWindowPos((HWND)GetProcess(), );
    set_console_size(handle, c_rows, c_columns);

    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    c_columns = csbi.srWindow.Right - csbi.srWindow.Left + 1;
    c_rows = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;

    //std::cout << "console: rows: " << rows << ", columns: " << columns << std::endl;

    using clock = std::chrono::steady_clock;

    std::chrono::system_clock::time_point a = std::chrono::system_clock::now();
    std::chrono::system_clock::time_point b = std::chrono::system_clock::now();
    double frameTime = 1 / 30.;

    int frames = 0;

    auto profileStart = high_resolution_clock::now();;

    auto global_start = high_resolution_clock::now();

#if profile_time
    std::vector<double> times;
#endif

    mciSendString(s2ws("open \"bad apple.mp3\" type mpegvideo alias mp3").c_str(), NULL, 0, NULL);
    mciSendString(s2ws("play mp3").c_str(), NULL, 0, NULL);

    Magick::Image image;

    for (size_t i = 2000; i < 6000; i++)
    {

        auto start = high_resolution_clock::now();


        //image.write("logo.png");

        GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
        c_columns = csbi.srWindow.Right - csbi.srWindow.Left + 1;
        c_rows = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
        // Create base image 

#if profile_time
        profileStart = high_resolution_clock::now();
#endif

        image.read("frames/frame_" + std::to_string(i) + ".jpg");

#if profile_time
        times.push_back(duration_cast<std::chrono::milliseconds>(high_resolution_clock::now() - profileStart).count());
#endif

#if profile_time
        profileStart = high_resolution_clock::now();
#endif

        image.rotate(-90);

#if profile_time
        times.push_back(duration_cast<std::chrono::milliseconds>(high_resolution_clock::now() - profileStart).count());
#endif

#if profile_time
        profileStart = high_resolution_clock::now();
#endif

        image.flip();

#if profile_time
        times.push_back(duration_cast<std::chrono::milliseconds>(high_resolution_clock::now() - profileStart).count());
#endif

#if profile_time
        profileStart = high_resolution_clock::now();
#endif

        image.resize(std::to_string(c_rows) + "x" + std::to_string(c_columns) + "!");

#if profile_time
        times.push_back(duration_cast<std::chrono::milliseconds>(high_resolution_clock::now() - profileStart).count());
#endif

        // Allocate pixel view 
        Pixels view(image);



        int rows, columns;
        rows = image.rows(); columns = image.columns();


#if profile_time
        profileStart = high_resolution_clock::now();
#endif

        std::string imageString(rows * columns, ' ');
        Quantum* pixels = view.get(0, 0, columns, rows);

        for (ssize_t row = 0; row < rows; ++row)
        {
            for (ssize_t column = 0; column < columns; ++column)
            {
                auto red = *pixels++;
                auto green = *pixels++;
                auto blue = *pixels++;
                imageString[column * rows + row] = shadeToChar[int((red + green + blue) / 3)];
            }
        }


#if profile_time
        times.push_back(duration_cast<std::chrono::milliseconds>(high_resolution_clock::now() - profileStart).count());
#endif


        int size = imageString.size();

#if profile_time
        profileStart = high_resolution_clock::now();
#endif

        DWORD charsWritten;
        WriteConsoleOutputCharacterA(handle, &imageString[0], size, { 0,0 }, &charsWritten);

#if profile_time
        times.push_back(duration_cast<std::chrono::milliseconds>(high_resolution_clock::now() - profileStart).count());
#endif

        auto end = high_resolution_clock::now();

        auto frame_time = duration_cast<std::chrono::microseconds>(end - start);

        auto durationSinceStart = duration_cast<std::chrono::nanoseconds>(high_resolution_clock::now() - global_start).count() / 1e9;

        // Reasoning: per standard the calculated end of frame is frame_start + timePerFrame
        // However, if we are going too fast or slow, we want to compensate for this. 
        // This is done by subtracting (frames - fps * durationSinceStart), frames processed vs amount of frames which should have been processed
        auto frame_end = start + std::chrono::microseconds(int(1e6 * (timePerFrame * (1 + frames - fps * durationSinceStart))));

        // Spinlocking to maintain correct framerate
        while (high_resolution_clock::now() < frame_end) {}
        frames += 1;

        //auto final_time = high_resolution_clock::now();

        //durationSinceStart = duration_cast<std::chrono::nanoseconds>(high_resolution_clock::now() - global_start).count() / 1e9;

        //double currentFps = 1. / (duration_cast<std::chrono::nanoseconds>(final_time-start).count()/1e9);
        //SetConsoleTitleA((std::to_string(frames-fps*durationSinceStart)+", " + std::to_string(frames) + ", " + std::to_string(fps*durationSinceStart)+ "," + std::to_string(currentFps)).c_str());

#if profile_time
        SetConsoleTitleA(("Read image: " + std::to_string(times[0]) + ", " +
            "Rotate image" + std::to_string(times[1]) + ", " +
            "Flip image" + std::to_string(times[2]) + ", " +
            "Resize image" + std::to_string(times[3]) + ", " +
            "Generate image" + std::to_string(times[4]) + ", " +
            "Write image" + std::to_string(times[5])).c_str());

        times.clear();
#endif
    }



}