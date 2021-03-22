#include <Magick++.h> 
#include <iostream> 
#include <windows.h>
#include <string>
#include <vector>
#include <chrono>
#include <thread>

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
    const double fps = 30;
    const double timePerFrame = 1 / 30.;
    InitializeMagick(*argv);

    HANDLE handle;

    handle = GetStdHandle(-11);

    Image image;
    image.read("C:\\Users\\Matis\\Desktop\\red apple\\frames\\frame_1.jpg");

    int desktopWidth, desktopHeight;
    GetDesktopResolution(desktopWidth, desktopHeight);

    int c_columns = int((desktopHeight*0.93))/14;
    int c_rows = (int(desktopWidth *0.95)) / 7;

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
    for (size_t i = 1; i < 6000; i++)
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

        image.read("C:\\Users\\Matis\\Desktop\\red apple\\frames\\frame_" + std::to_string(i) + ".jpg");

#if profile_time
        times.push_back(duration_cast<std::chrono::milliseconds>(high_resolution_clock::now()- profileStart).count());
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
            for (ssize_t column = 0; column < columns; ++column)
            {
                auto red = pixels++;
                if (*red > 10)
                {
                    imageString[column * rows + row] = '#';
                }

                *pixels++;
                *pixels++;
            }


#if profile_time
        times.push_back(duration_cast<std::chrono::milliseconds>(high_resolution_clock::now() - profileStart).count());
#endif


        int size = imageString.size();

#if profile_time
        profileStart = high_resolution_clock::now();
#endif

        DWORD charsWritten;
        WriteConsoleOutputCharacterA(handle, &imageString[0], size, {0,0}, &charsWritten);

#if profile_time
        times.push_back(duration_cast<std::chrono::milliseconds>(high_resolution_clock::now() - profileStart).count());
#endif

        auto end = high_resolution_clock::now();

        auto frame_time = duration_cast<std::chrono::microseconds>(end - start);

        auto durationSinceStart = duration_cast<std::chrono::nanoseconds>(high_resolution_clock::now() - global_start).count() / 1e9;


        auto frame_end = start + std::chrono::microseconds(int(1e6*(timePerFrame*(1 + frames - fps * durationSinceStart))));

        while (high_resolution_clock::now() < frame_end) {}
        frames += 1;

        //auto final_time = high_resolution_clock::now();

        //durationSinceStart = duration_cast<std::chrono::nanoseconds>(high_resolution_clock::now() - global_start).count() / 1e9;

        //double currentFps = 1. / (duration_cast<std::chrono::nanoseconds>(final_time-start).count()/1e9);
        //SetConsoleTitleA((std::to_string(frames-fps*durationSinceStart)+", " + std::to_string(frames) + ", " + std::to_string(fps*durationSinceStart)+ "," + std::to_string(currentFps)).c_str());
        
#if profile_time
        SetConsoleTitleA(("Read image: " + std::to_string(times[0]) +", " +
            "Rotate image" + std::to_string(times[1]) + ", " +
            "Flip image" + std::to_string(times[2]) + ", " +
            "Resize image" + std::to_string(times[3]) + ", " +
            "Generate image" + std::to_string(times[4]) + ", " +
            "Write image" + std::to_string(times[5])).c_str());

        times.clear();
#endif
    }



}