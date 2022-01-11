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

void read_bytes(ifstream &istream, void *buffer, size_t buffer_size) {
    if(!istream.read((char*) buffer, buffer_size)) {
        if(istream.eof()) {
            cerr << "[Error]: Cannot read " << buffer_size << " bytes from the ifstream.\n";
        } else if(istream.fail())
        {
            cerr << "[Error]: Something went wrong when reading the ifstream.\n";
        } else
        {
            assert(0 && "unreachable");
        }
        exit(1);
    }
}

void write_bytes(ostream &ostream, const void *buffer, size_t buffer_size) {
    ostream.write((const char *)buffer, buffer_size);
    if(!ostream.good()) {
        cerr << "[Error]: Something went wrong when writing the ofstream.\n";
        exit(1);
    }
}

void print_bytes(void *buffer, size_t buffer_size) {
    uint8_t *buff = (uint8_t *)buffer;
    for (size_t i = 0; i < buffer_size; i++)
        cout << (int)buff[i] << " ";
    cout << "\n";
}

void reverse_bytes(void *buffer, size_t buffer_size) {
    uint8_t *buf = (uint8_t *) buffer;
    for (size_t i = 0; i < buffer_size / 2; i++)
    {
        uint8_t temp = buf[i];
        buf[i] = buf[buffer_size - i - 1];
        buf[buffer_size - i - 1] = temp;

    }
}

void print_usage(const string &program) {
    cerr << "Usage:" << program << " <input.png> <output.png>\n";
}

int main(int argc, char const *argv[])
{
    string program = *argv++;
    
    // Gets input file name
    if (*argv == NULL) {
        print_usage(program);
        cerr << "[Error]: No input file is provided. \n";
        exit(1);
    }
    string input_filepath = *argv++;

    // Gets output file name
    if (*argv == NULL) {
        print_usage(program);
        cerr << "[Error]: No output file is provided. \n";
        exit(1);
    }
    string output_filepath = *argv++;

    // Openning the input file
    ifstream input_file (input_filepath, ios::in | ios::binary);
    if (!input_file.is_open()) {
        cerr << "[Error]: Unable to open file " << input_filepath <<": " << strerror(errno) << ".\n";
        exit(1);
    }

    // Openning the output file
    ofstream output_file (output_filepath, ios::out | ios::binary);
    if (!output_file.is_open()) {
        cerr << "[Error]: Unable to open file " << output_filepath <<": " << strerror(errno) << ".\n";
        exit(1);
    }

    // Reads the signature of PNG image
    uint8_t sig[PNG_SIG_CAP];
    read_bytes(input_file, sig, PNG_SIG_CAP);
    if(memcmp(sig, png_sig, PNG_SIG_CAP) != 0) {
        cerr << "[Error]: " << input_filepath << " is not a PNG image.\n";
        exit(1);
    }

    bool quit = false;
    while (!quit)
    {
        // Reads the chunk size
        uint32_t chunk_sz;
        read_bytes(input_file, &chunk_sz, sizeof(chunk_sz));
        reverse_bytes(&chunk_sz, sizeof(chunk_sz));

        // Reads the chunk type
        uint8_t chunk_type[4];
        read_bytes(input_file, chunk_type, sizeof(chunk_type));

        // If type is IEND
        if(*(uint32_t*)chunk_type == 1145980233) quit = true;

        // Jumps the chunk data
        input_file.seekg(chunk_sz, std::ios_base::cur);
        if(!input_file.good()) {
            cerr << "[Error]: Jumping chunk failed.\n";
        }

        // Reads the chunk crc
        uint32_t chunk_crc;
        read_bytes(input_file, &chunk_crc, sizeof(chunk_crc));

        cout << "[Info]: Chunk Size: " << chunk_sz << "\n";
        cout << "[Info]: Chunk Type: " << chunk_type[0] << chunk_type[1] << chunk_type[2] << chunk_type[3] << " (" << *(uint32_t*)chunk_type << ")\n";
        cout << "[Info]: Chunk CRC: " << chunk_crc << "\n";
        cout << "--------------------------------------\n";
    }

    input_file.close();
    output_file.close();
    return 0;
}
