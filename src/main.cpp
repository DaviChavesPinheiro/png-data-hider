#include <iostream>
#include <string>
#include <cstring>
#include <fstream>
#include <cerrno>
#include <cstdint>
#include <cassert>

using namespace std;

int main(int argc, char const *argv[])
{
    string program = *argv++;
    
    if (*argv == NULL) {
        cerr << "Usage:" << program << " <input.png>\n";
        exit(1);
    }
    
    string input_filepath = *argv++;
    cout << "Inspected file is " << input_filepath << ".\n";

    // Openning the file
    ifstream file (input_filepath, ios::in | ios::binary);
    if (!file.is_open()) {
        cerr << "[Error]: Unable to open file " << input_filepath <<": " << strerror(errno) << ".\n";
        exit(1);
    }

    uint8_t sig[8];
    if(!file.read((char*) sig, sizeof(sig))) {
        if(file.eof()) {
            cout << "[Error]: The image is tool small.\n";
        } else if(file.fail())
        {
            cout << "[Error]: Something went wrong when opening the image.\n";
        } else
        {
            assert(0 && "unreachable");
        }
        exit(1);
    }
    
    for (size_t i = 0; i < sizeof(sig); i++)
    {
        cout << (int)sig[i] << " ";
    }
    
    
    return 0;
}
