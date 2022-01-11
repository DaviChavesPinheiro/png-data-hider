#include <iostream>
#include <string>
#include <cstring>
#include <fstream>
#include <cerrno>
#include <cstdint>
#include <cassert>

using namespace std;

/* Table of CRCs of all 8-bit messages. */
unsigned long crc_table[256];

/* Flag: has the table been computed? Initially false. */
int crc_table_computed = 0;

/* Make the table for a fast CRC. */
void make_crc_table(void)
{
    unsigned long c;
    int n, k;

    for (n = 0; n < 256; n++) {
        c = (unsigned long) n;
        for (k = 0; k < 8; k++) {
            if (c & 1)
                c = 0xedb88320L ^ (c >> 1);
            else
                c = c >> 1;
        }
        crc_table[n] = c;
    }
    crc_table_computed = 1;
}

/* Update a running CRC with the bytes buf[0..len-1]--the CRC
   should be initialized to all 1's, and the transmitted value
   is the 1's complement of the final running CRC (see the
   crc() routine below)). */

unsigned long update_crc(unsigned long crc, unsigned char *buf, int len)
{
    unsigned long c = crc;
    int n;

    if (!crc_table_computed)
        make_crc_table();
    for (n = 0; n < len; n++) {
        c = crc_table[(c ^ buf[n]) & 0xff] ^ (c >> 8);
    }
    return c;
}

/* Return the CRC of the bytes buf[0..len-1]. */
unsigned long crc(unsigned char *buf, int len)
{
    return update_crc(0xffffffffL, buf, len) ^ 0xffffffffL;
}


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

#define CHUNK_BUF_CAP (32 * 1024)
uint8_t chunk_buf[CHUNK_BUF_CAP];

int main(int argc, char const *argv[])
{
    make_crc_table();

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
    write_bytes(output_file, sig, PNG_SIG_CAP);
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
        write_bytes(output_file, &chunk_sz, sizeof(chunk_sz));
        reverse_bytes(&chunk_sz, sizeof(chunk_sz));

        // Reads the chunk type
        uint8_t chunk_type[4];
        read_bytes(input_file, chunk_type, sizeof(chunk_type));
        write_bytes(output_file, chunk_type, sizeof(chunk_type));

        // If type is IEND
        if(*(uint32_t*)chunk_type == 1145980233) quit = true;

        // Reads and copies the chunk data
        size_t n = chunk_sz;
        while (n > 0) {
            size_t m = n;
            if (m > CHUNK_BUF_CAP) {
                m = CHUNK_BUF_CAP;
            }
            read_bytes(input_file, chunk_buf, m);
            write_bytes(output_file, chunk_buf, m);
            n -= m;
        }

        // Reads the chunk crc
        uint32_t chunk_crc;
        read_bytes(input_file, &chunk_crc, sizeof(chunk_crc));
        write_bytes(output_file, &chunk_crc, sizeof(chunk_crc));

        // Injects the message
        if (*(uint32_t*)chunk_type == 1380206665) {
            // TODO: Make a custom data injection
            uint32_t injected_sz = 13;
            reverse_bytes(&injected_sz, sizeof(injected_sz));
            write_bytes(output_file, &injected_sz, sizeof(injected_sz));
            reverse_bytes(&injected_sz, sizeof(injected_sz));

            const string injected_type = "ntHG";
            write_bytes(output_file, injected_type.c_str(), 4);

            write_bytes(output_file, "Hello World\n", injected_sz);

            // TODO: This is wrong. Fixe it.
            uint32_t injected_crc = crc((unsigned char *)"Hello World\n", injected_sz);
            write_bytes(output_file, &injected_crc, sizeof(injected_crc));
        }

        cout << "[Info]: Chunk Size: " << chunk_sz << "\n";
        cout << "[Info]: Chunk Type: " << chunk_type[0] << chunk_type[1] << chunk_type[2] << chunk_type[3] << " (" << *(uint32_t*)chunk_type << ")\n";
        cout << "[Info]: Chunk CRC: " << chunk_crc << "\n";
        cout << "--------------------------------------\n";
    }

    input_file.close();
    output_file.close();
    return 0;
}
