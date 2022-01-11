#include <iostream>
#include <string>
#include <cstring>
#include <fstream>
#include <cerrno>
#include <cstdint>
#include <cassert>

using namespace std;

#define PNG_SIG_CAP 8
const uint8_t png_sig[PNG_SIG_CAP] = {137, 80, 78, 71, 13, 10, 26, 10};

void read_buffer(ifstream &istream, uint8_t *buffer, size_t buffer_size) {
    if(!istream.read((char*) buffer, buffer_size)) {
        if(istream.eof()) {
            cerr << "[Error]: The image is tool small.\n";
        } else if(istream.fail())
        {
            cerr << "[Error]: Something went wrong when opening the image.\n";
        } else
        {
            assert(0 && "unreachable");
        }
        exit(1);
    }
}

void print_bytes(uint8_t *buffer, size_t buffer_size) {
    for (size_t i = 0; i <buffer_size; i++)
        cout << (int)buffer[i] << " ";
    cout << "\n";
}

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

    // Reads the signature of PNG image
    uint8_t sig[PNG_SIG_CAP];
    read_buffer(file, sig, PNG_SIG_CAP);
    print_bytes(sig, PNG_SIG_CAP);
    if(memcmp(sig, png_sig, PNG_SIG_CAP) != 0) {
        cerr << "[Error]: " << input_filepath << " is not a PNG image.\n";
        exit(1);
    }

    file.close();
    return 0;
}
