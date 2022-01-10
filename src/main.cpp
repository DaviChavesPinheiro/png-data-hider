#include <iostream>
#include <string>
#include <cstring>
#include <fstream>
#include <cerrno>

using namespace std;

int main(int argc, char const *argv[])
{
    string program = *argv++;
    
    if (*argv == NULL)
    {
        cerr << "Usage:" << program << " <input.png>\n";
        exit(1);
    }
    
    string input_filepath = *argv++;
    cout << "Inspected file is " << input_filepath << "\n";

    // Openning the file
    streampos f_size;
    char *memblock;

    ifstream file (input_filepath, ios::in | ios::binary | ios::ate);
    if (file.is_open())
    {
        f_size = file.tellg();
        memblock = new char[f_size];
        file.seekg(0, ios::beg);
        file.close();
    } else {
        cerr << "Unable to open file " << input_filepath <<": " << strerror(errno) << "\n";
        exit(1);
    }
    
    
    return 0;
}
