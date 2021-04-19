#include <iostream> 
#include <windows.h>
#include <map>
#include <string>
#include <vector>
#include <chrono>
#include <thread>
#include <filesystem>
#include <algorithm>

#include "opencv2/opencv.hpp"
#include "func.h"

using std::chrono::high_resolution_clock;
using std::chrono::duration_cast;
namespace fs = std::filesystem;
using namespace cv;

// The main algorithm can be found in a big comment in the main loop around line 180

int main(int argc, char** argv)
{
    // Resize the console window such that the internal screen buffer is as large the visible one. Otherwise, we will write characters offscreen and the video
    // will be unrecognizable
    HANDLE handle;

    handle = GetStdHandle(-11);

    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    int c_columns = csbi.srWindow.Right - csbi.srWindow.Left + 1;
    int c_rows = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
    set_console_size(handle, c_columns, c_rows);


    // Finding the best character for each shade (rgb grayscale between 0 and 255)
    std::map<int, char> shadeToChar = {};

    for (int i = 0; i < 256; i++)
    {
        float v = float(i) / 255;

        char bestChar = '@';
        float bestValue = 1e9;
        for (int j = 0; j < characterDensities.size(); j++)
        {
            if (abs(v - characterDensities[j].first) < bestValue)
            {
                bestChar = characterDensities[j].second;
                bestValue = abs(v - characterDensities[j].first);
            }
        }

        shadeToChar[i] = bestChar;
    }


    // We do not want to clog up the filesystem, videos can be quite large
    std::filesystem::remove_all("youtube-dl");

    std::filesystem::create_directories("youtube-dl");


    // Get user input of what they want to watch
    std::string uri;

    std::cout << "Please input a link to a youtube video or a file path to a video" << std::endl;
    std::cout << "It is recommended that the video is not longer than 6 minutes unless your wifi is good" << std::endl;
    std::cin >> uri;

    std::string videoPath;

    // A simple check to determine whether the user gave a link to youtube or a filepath
    if (uri.find("youtube.com") != std::string::npos)
    {
        // Strip parts of youtube link such as &list=RDGMEMRpqX2eKUq0kW97LmkR0R1wVMFz6KcyC-L2E&index=6, due to the fact that they will be processed as commands
        size_t ampersandLocation = uri.find("&");
        if (ampersandLocation != std::string::npos)
        {
            uri = uri.substr(0, ampersandLocation);
        }

        std::cout << "Downloading video from youtube, this may take a while." << std::endl;

        // Run ffmpeg with a system call. Even though the output is useful, it will only intimidate 98% of users
        std::string command = "youtube-dl.exe -q --no-warnings -o \"youtube-dl/temp\" " + uri;
        system(command.c_str());

        
        // Sleep for a little while, io synchronization issue if we try to grab it immediately
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        // Get the video. Since youtube-dl won't tell us the filename easily, we simply grab the first file and it is guaranteed to be the correct one
        // since we deleted and recreated the folder at the start of the program
        for (const auto& entry : fs::directory_iterator("youtube-dl"))
        {
            videoPath = entry.path().string();
            break;
        }

        // If we cannot find a video, something went wrong
        if (videoPath == "")
        {
            std::cout << "An error occured when downloading the video, please try again" << std::endl;
            std::cout << "Press any key to exit..." << std::endl;
            return -1;
        }
        std::cout << "Download complete!" << std::endl;
    }
    // Checking that the file exists before we try to open it
    else if (true)
    {
        videoPath = uri;
    }
    else
    {
        std::cout << "You did not give a link to an existing youtube video or file on your computer" << std::endl;
        std::cout << "Press any key to exit..." << std::endl;
        std::cin >> uri;
        return 0;
    }

    // Open the video file
    cv::VideoCapture video(videoPath);

    // Simple error check
    if (!video.isOpened())
    {
        std::cout << "Could not open the file, please try again" << std::endl;
        std::cout << "Press any key to exit..." << std::endl;
        std::cin >> uri;
        return -1;
    }

    // Read file metadata
    double fps = video.get(cv::CAP_PROP_FPS);
    const double timePerFrame = 1. / fps;
    int frameCount = video.get(cv::CAP_PROP_FRAME_COUNT);

    
    // Clear old text, in some edgecases it can stick around
    system("cls");

    // We use ffplay to play the audio, since opencv has no support for audio. Make sure that nothing is printed as not to disrupt the video
    std::string command = "ffplay -nodisp -hide_banner -loglevel error " + videoPath;

    STARTUPINFOA info = { sizeof(info) };
    PROCESS_INFORMATION processInfo;
    // Start a new process and do not wait for it to finish. This way, we can play the audio in parallell, but it will still exit along with the main process
    // if we close the cmd window
    CreateProcessA("ffplay.exe", (LPSTR)(command.c_str()), NULL, NULL, TRUE, 0, NULL, NULL, &info, &processInfo);

    // Save the time when the audio started playing, used for keeping a proper framerate and syncing to audio
    auto global_start = high_resolution_clock::now();

    // Start playing up video frames
    for (int i = 0; i < frameCount; i++)
    {
        // Used at the bottom to keep stable framerate
        auto start = high_resolution_clock::now();

        // Detect whether the window has been resized and adjust
        GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
        c_columns = csbi.srWindow.Right - csbi.srWindow.Left + 1;
        c_rows = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;

        // Read the next frame from the video
        cv::Mat frame;
        video >> frame;

        // If there was an error, do not continue
        if (frame.empty())
            continue;


        // Resize the frame and store it in a new variable (c++ constant size array requiring new variable). We resize it excactly to (columns, rows) of the cmd window
        // so that each pixel corresponds to one cell in the cmd window. We then convert the pixel value to grayscale by averaging the brightnesses of r,g,b channel.
        // We thus the brightness of the pixel. We can now approximate this pixel by using an ascii character which fills a similiar percent of each cell to the
        // percentage brightness of it. 
        // This algorithm has a few shortcomings: 1: it won't retain text present in the video if it is downsampled too due to much.
        // 2: it works poorly for images with small contrast. I tried fixing this by increasing the image contrast, but due to the effects varying highly i decided against it.
        // However, I am still quite pleased with the results and will probably try to solve the first problem using a text detection AI.
        Mat resizedFrame;

        // Use INTER_LINEAR for the simplest resizing algorithm (won't be an issue due quality loss inherent from this algorithm)
        resize(frame, resizedFrame, Size(c_columns, c_rows), 0, 0, INTER_LINEAR);

        // Rotate and flip it due to the internal format being difficult to work with
        cv::rotate(resizedFrame, resizedFrame, cv::ROTATE_90_CLOCKWISE);
        cv::flip(resizedFrame, resizedFrame, 1);

        // Use the actual image size in case something were to fail somehow
        int rows = resizedFrame.rows, columns = resizedFrame.cols;

        
        // Creating a new string each frame is not too efficient. However, it ends up paying off as certain issues can occur by simply clearing the old one
        // There is also the added benefit that it can easily adapt to a resized window
        std::string imageString(columns * rows, ' ');

        // Perform the algorithm described around line 175
        for (size_t row = 0; row < rows; ++row)
        {
            for (size_t column = 0; column < columns; ++column)
            {
                cv::Vec3b pixel = resizedFrame.at<cv::Vec3b>(row, column);
                uchar r = pixel[2];
                uchar g = pixel[1];
                uchar b = pixel[0];
                imageString[column * rows + row] = shadeToChar[int((r + g + b) / 3)];
            }
        }

        // Write the string. We use this function as it overwrites previous characters, eliminating flashing images that occur when first clearing and then writing
        DWORD charsWritten;
        WriteConsoleOutputCharacterA(handle, &imageString[0], imageString.size(), { 0,0 }, &charsWritten);

        // Here, we make sure to sleep so that we do not display more images than per second than the fps of the video
        // This is accomplished by computing how long each frame should be displayed (1/fps), and then sleeping if we finished earlier than that.
        auto end = high_resolution_clock::now();

        auto frame_time = duration_cast<std::chrono::microseconds>(end - start);

        auto durationSinceStart = duration_cast<std::chrono::nanoseconds>(high_resolution_clock::now() - global_start).count() / 1e9;

        // Reasoning: per standard the calculated time of the end of frame is frame_start + timePerFrame
        // However, if we are going too fast or slow, we want to compensate for this. 
        // This is done by subtracting (no_of_frames - fps * durationSinceStart), frames processed vs amount of frames which should have been processed
        auto frame_end = start + std::chrono::microseconds(int(1e6 * (timePerFrame * (1 + i - fps * durationSinceStart))));

        // First we sleep to maintain correct framerate. However, sleep is very low resolution, so don't sleep the entire duration.
        auto now = high_resolution_clock::now();
        std::this_thread::sleep_for(frame_end - now - std::chrono::milliseconds(2));
        // We then perform a spinolock to maintain a highly accurate framerate
        while (high_resolution_clock::now() < frame_end) {}
    }

    CloseHandle(processInfo.hProcess);
    CloseHandle(processInfo.hThread);
    return 0;
}