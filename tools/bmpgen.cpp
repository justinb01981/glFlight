#include <iostream>
#include <fstream>

using namespace std;

typedef unsigned char b8;
typedef unsigned short b16;
typedef unsigned int b32;

struct bmpinfohdr
{
    b32 hdrlen;
    b32 width;
    b32 height;
    b16 planes; // 1
    b16 bitsperpix;
    b32 compression;
    b32 imagesizelen; // 0
    b32 pixpermeterhoriz;
    b32 pixpermetervert;
    b32 colorpalattelen; // 0
    b32 importantcolors; // 0
};

struct bmphdr
{
     b32 filesz;
     b16 res;
     b16 res2;
     b32 offset;
     struct bmpinfohdr bmpinfo;
};

struct pixel
{
    b8 red,green,blue,alpha;
};

void dowrite(fstream& f, b8* p, unsigned int len)
{
    while(len > 0)
    {
         f << *p;
         p++;
         len--;
    }
}

int main(int argc, char **argv)
{
    fstream o("out.bmp", ios::binary | ios::out);
    int width = 512;
    int height = 512;
    struct bmphdr hdr;

    memset(&hdr, 0, sizeof(hdr));

    hdr.bmpinfo.width = width;
    hdr.bmpinfo.height = height;
    hdr.bmpinfo.planes = 1;
    hdr.bmpinfo.bitsperpix = 32;
    hdr.bmpinfo.pixpermetervert = 1024;
    hdr.bmpinfo.pixpermeterhoriz = 1024;
    hdr.bmpinfo.imagesizelen= width * height * sizeof(struct pixel);
    hdr.bmpinfo.hdrlen = sizeof(hdr.bmpinfo);
    
    char pre[2] = {0x42, 0x4d};
    dowrite(o, (b8*) pre, 2);

    hdr.offset = sizeof(pre) + sizeof(struct bmphdr);
    hdr.filesz = sizeof(pre) + sizeof(struct bmphdr) + width*height*sizeof(struct pixel);
    dowrite(o, (b8*) &hdr, sizeof(hdr));    

    srand(time(NULL));

    for(int x = 0; x < width; x++)
        for(int y = 0; y < height; y++) {
            struct pixel pix;
            pix.red = rand() % 0xff;
            pix.green = rand() % 0xff;
            pix.blue = rand() % 0xff;
            pix.alpha = 0;
            dowrite(o, (b8*) &pix, sizeof(pix));
        }
    
    return 0;
}
