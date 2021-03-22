#include <Magick++.h> 
#include <iostream> 
#include <windows.h>
#include <string>
#include <vector>

std::wstring ExePath() {
    TCHAR buffer[MAX_PATH] = { 0 };
    GetModuleFileName(NULL, buffer, MAX_PATH);
    std::wstring::size_type pos = std::wstring(buffer).find_last_of(L"\\/");
    return std::wstring(buffer).substr(0, pos);
}

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



void cyclic_roll(int& a, int& b, int& c, int& d)
{
    int temp = a;
    a = b;
    b = c;
    c = d;
    d = temp;
}


using namespace std;
using namespace Magick;

int main(int argc, char** argv)
{
    //HANDLE handle;

    //handle = GetStdHandle(-11);

    //int columns, rows;

    //CONSOLE_SCREEN_BUFFER_INFO csbi;

    //GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    //columns = csbi.srWindow.Right - csbi.srWindow.Left + 1;
    //rows = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;

    //Image image;
    //image.read("C:\\Users\\Matis\\Desktop\\red apple\\frames\\frame_67.jpg");
    //InitializeMagick(*argv);

    //image.resize(std::to_string(rows)+"x"+std::to_string(columns)+"!");

    //// Allocate pixel view 
    //Pixels view(image);



    //std::string hi;
    //std::cout << "console: rows: " << rows << ", columns: " << columns << std::endl;
    //std::cout << "image: rows: " << image.rows() << ", columns: " << image.columns() << std::endl;
    //Quantum* pixels = view.get(0, 0, rows-1, columns-1);
    //for (ssize_t row = 0; row < rows; ++row)
    //    for (ssize_t column = 0; column < columns; ++column)
    //    {
    //        //hi += ( (*pixels++) > 10 ? '#' : ' ');
    //        //*pixels++;
    //        //*pixels++;
    //        *pixels++ = row / 10;
    //        *pixels++ = 0;
    //        *pixels++ = 0;

    //    }

    //view.sync();

    //std::cout << hi;
    //// Save changes to image.
    HANDLE handle;

    handle = GetStdHandle(-11);

    Image image;
    image.read("C:\\Users\\Matis\\Desktop\\red apple\\frames\\frame_1.jpg");
    float aspect_ratio = float(image.columns()) / image.rows();

    int c_columns = (1080-100)/14;
    int c_rows = (1920 - 100) / 7;//int(c_columns * aspect_ratio)*2;

    CONSOLE_SCREEN_BUFFER_INFO csbi;


    //SetWindowPos((HWND)GetProcess(), );
    set_console_size(handle, c_rows, c_columns);

    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    c_columns = csbi.srWindow.Right - csbi.srWindow.Left + 1;
    c_rows = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;

    //std::cout << "console: rows: " << rows << ", columns: " << columns << std::endl;

    
    
    for (size_t i = 1; i < 6000; i++)
    {
        //image.write("logo.png");
        InitializeMagick(*argv);


        // Create base image 
        
        image.read("C:\\Users\\Matis\\Desktop\\red apple\\frames\\frame_" + std::to_string(i) + ".jpg");

        image.rotate(-90);
        image.flip();

        // Allocate pixel view 
        image.resize(std::to_string(c_rows) + "x" + std::to_string(c_columns) + "!");

        // Set the image type to TrueColor DirectClass representation.
        // Ensure that there is only one reference to underlying image 
        // If this is not done, then image pixels will not be modified.

        // Allocate pixel view 
        Pixels view(image);

        // Set all pixels in region anchored at 38x36, with size 160x230 to green. 


        int rows, columns;
        rows = image.rows(); columns = image.columns();
        //std::cout << "console: rows: " << rows << ", columns: " << columns << std::endl;
        //std::cout << "image: rows: " << image.rows() << ", columns: " << image.columns() << std::endl;
        std::string imageString(rows * columns, ' ');
        Quantum* pixels = view.get(0, 0, columns, rows);
        
        for (ssize_t row = 0; row < rows; ++row)
            for (ssize_t column = 0; column < columns; ++column)
            {
                auto red = pixels++;
                if (*red < 10)
                {
                    imageString[column * rows + row] = '#';
                }

                *pixels++;
                *pixels++;
            }


        int size = imageString.size();

        DWORD charsWritten;
        WriteConsoleOutputCharacterA(handle, &imageString[0], size, {0,0}, &charsWritten);

    }



}