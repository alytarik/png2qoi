#include <fstream>
#include <iostream>
#include <string.h>

#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_PNG
#include "stb_image.h"

#define QOI_RGBA   0xff /* 11111111 */
#define QOI_RGB    0xfe /* 11111110 */
#define QOI_RUN    0xc0 /* 11xxxxxx */
#define QOI_LUMA   0x80 /* 10xxxxxx */
#define QOI_DIFF   0x40 /* 01xxxxxx */
#define QOI_INDEX  0x00 /* 00xxxxxx */

#define COLOR_HASH(C) ((C.r*3 + C.g*5 + C.b*7 + C.a*11) % 64)

struct Pixel {
    Pixel() : r(0), g(0), b(0), a(255) {}
    //Pixel(unsigned char r, unsigned char g, unsigned char b) : r(r), g(g), b(b) {}
    Pixel(unsigned char r, unsigned char g, unsigned char b, unsigned char a=255) : r(r), g(g), b(b), a(a) {}
    unsigned char r, g, b, a;

    bool operator==(const Pixel& rhs) {
        return r == rhs.r && g == rhs.g && b == rhs.b && a == rhs.a;
    }
    bool operator!=(const Pixel& rhs) {
        return !(*this==rhs);
    }
};

void write32(unsigned char* data, int &pos, const int &v) {
    data[pos++] = v >> 24;
    data[pos++] = (v & 0x00ffffff) >> 16;
    data[pos++] = (v & 0x0000ffff) >> 8;
    data[pos++] = (v & 0x000000ff);
}

int main(int argc, char** argv) {
    if(!argv[1]) {
        std::cout << "usage: p2q <filename>" << std::endl;
        return 1;
    }
    int x,y,n, idx=0;
    if(!stbi_info(argv[1], &x, &y, &n)) {
        std::cout << "Couldn't read file";
        return 1;
    }
    
    std::cout << x << "x" << y << " with " << n << " channels." << std::endl;

    //load the original png file
    unsigned char* data = (unsigned char*) stbi_load(argv[1], &x, &y, NULL, n);
    //create a buffer
    unsigned char* qoi = (unsigned char*) malloc(x*y*(n+1)*2);
    //write the header
    write32(qoi, idx, (('q' << 24) | ('o' << 16) | ('i' << 8) | ('f')));
    write32(qoi, idx, x);
    write32(qoi, idx, y);
    qoi[idx++] = n;
    qoi[idx++] = 0;

    Pixel prevPx;
    Pixel* table[64] = {};

    unsigned char run = 0;
    for (int i = 0; i < x*y*n; i+=n)
    {
        Pixel currPx(data[i], data[i + 1], data[i + 2]);
        if(n==4) currPx.a = data[i + 3];
        
        //RUN
        if(prevPx == currPx) {
            run++;
            if(run == 62 || i == x * y * n - n) {
                qoi[idx++] = QOI_RUN | run-1;
                run = 0;
            }
        }
        else {
            //RUN
            if(run>0) {
                qoi[idx++] = QOI_RUN | run-1;
                run = 0;
            }
            //calculate difference
            char dr = currPx.r - prevPx.r, dg = currPx.g - prevPx.g, db = currPx.b - prevPx.b;
            if(table[COLOR_HASH(currPx)] && *table[COLOR_HASH(currPx)] == currPx) {
                qoi[idx++] = QOI_INDEX | (COLOR_HASH(currPx));
                prevPx = currPx;
            }
            //RGBA
            else if(prevPx.a!=currPx.a) {
                qoi[idx++] = QOI_RGBA;
                qoi[idx++] = currPx.r;
                qoi[idx++] = currPx.g;
                qoi[idx++] = currPx.b;
                qoi[idx++] = currPx.a;
                prevPx = currPx;
                table[COLOR_HASH(currPx)] = new Pixel(currPx);
            }
            //DIFF
            else if(dr < 2 & dg < 2 & db < 2 & dr > -3 & dg > -3 & db > -3) {
                qoi[idx++] = QOI_DIFF | (dr+2) << 4 | (dg+2) << 2 | (db+2);
                prevPx = currPx;
                table[COLOR_HASH(currPx)] = new Pixel(currPx);
            }
            //LUMA
            else if (dg < 32 && dg > -33 && dr-dg < 8 && db-dg < 8 && dr-dg > -9 && db-dg > -9)
            {
                qoi[idx++] = QOI_LUMA | (dg+32);
                qoi[idx++] = ((dr-dg+8) << 4) | (db-dg+8);
                prevPx = currPx;
                table[COLOR_HASH(currPx)] = new Pixel(currPx);
            }
            
            //RGB
            else {
                qoi[idx++] = QOI_RGB;
                qoi[idx++] = currPx.r;
                qoi[idx++] = currPx.g;
                qoi[idx++] = currPx.b;
                prevPx = currPx;
                table[COLOR_HASH(currPx)] = new Pixel(currPx);
            }
        }
    }
    write32(qoi, idx, 0x00000000);
    write32(qoi, idx, 0x00000001);
    
    std::ofstream file(argv[2], std::ios_base::binary);
    file.write((char* )qoi, idx);

    return 0;
}