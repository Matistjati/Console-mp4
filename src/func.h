#ifndef FUNC_H
#define FUNC_H
#include <windows.h>
#include <string>
#include <vector>

// Set the console buffer size
void set_console_size(HANDLE screen_buffer, SHORT width, SHORT height);

// Convert 1-byte string to 2-byte string.
std::wstring s2ws(const std::string& s);


// A simple check whether a file exists or not
inline bool FileExists(const std::string& name);


// How many percent each ascii character fills each "cell"
// Detailed: because we are using a monospaced font in cmd, the console is divided into rectangles with uniform size. We want to know how many percent of this cell
// each character occupies. This was computed using fontrenderer.py by selecting a specific font size (think it was 16, which corresponds to 8x7). We then draw
// each character to an image and count how many pixels are not black and then dividing that by the total size of the cell (with some adjustments to even out the ranges).
extern std::vector<std::pair<float, char>> characterDensities;

#endif // !FUNC_H
