#include <cstring>
#include <list>
#include <dirent.h>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <sstream>

#include <3ds.h>

using namespace std;

string baseurl = "http://luma.martmists.com/";

string getLatestCoreDump() {
    string last = "";

    DIR *dir;
    dirent *ent;
    if ((dir = opendir("/luma/dumps/arm11")) != nullptr) {
        while ((ent = readdir(dir)) != nullptr) {
            // Get last file
            last = ent->d_name;
        }
        closedir(dir);
    }
    return last;
}

string postFile(const string filename) {
    try {
        list<string> headers;
        headers.emplace_back("content-type: application/octet-stream");

        ifstream file;
        file.open(filename);

        file.seekg(0, ios_base::end);
        int size = file.tellg();
        file.seekg(0);

        char buf[size];
        file.read(buf, size);

        string url = baseurl + "upload";
        string b = "----------287032381131322";
        string part = "\nContent-Disposition: form-data; name=\"datafile1\"; filename=\"file.dump\""
                      "\nContent-Type: application/octet-stream\n\n";
        string end = b+"--";
        string h = "multipart/form-data;boundary="+b;

        httpcContext ctx;
        httpcOpenContext(&ctx, HTTPC_METHOD_POST, url.c_str(), 0);
        httpcSetSSLOpt(&ctx, SSLCOPT_DisableVerify);
        httpcSetKeepAlive(&ctx, HTTPC_KEEPALIVE_ENABLED);
        httpcAddRequestHeaderField(&ctx, "User-Agent", "DumpLoader/1.0.0");
        httpcAddRequestHeaderField(&ctx, "Content-Type", h.c_str());
        httpcAddPostDataAscii(&ctx, ("\n" + b).c_str(), "25");
        httpcAddPostDataAscii(&ctx, part.c_str(), "118");
        httpcAddPostDataRaw(&ctx, (u32*)buf, strlen(buf));
        httpcAddPostDataAscii(&ctx, ("\n" + end).c_str(), "27");

        Result ret = httpcBeginRequest(&ctx);

        u32 statuscode = 0;

        if(ret!=0){
            httpcCloseContext(&ctx);
            throw;
        }

        ret = httpcGetResponseStatusCode(&ctx, &statuscode);

        if(ret!=0){
            httpcCloseContext(&ctx);
            throw;
        }

        u8 *res = (u8*)malloc(0x1000);
        u32 readsize=0, rsize=0;
        u8 *lastres;

        if (res== nullptr){
            httpcCloseContext(&ctx);
            throw;
        }

        do {
            // This download loop resizes the resfer as data is read.
            ret = httpcDownloadData(&ctx, res+rsize, 0x1000, &readsize);
            rsize += readsize;
            if (ret == (s32)HTTPC_RESULTCODE_DOWNLOADPENDING){
                lastres = res; // Save the old pointer, in case realloc() fails.
                res = (u8*)realloc(res, rsize + 0x1000);
                if(res==nullptr){
                    httpcCloseContext(&ctx);
                    free(lastres);
                    throw;
                }
            }
        } while (ret == (s32)HTTPC_RESULTCODE_DOWNLOADPENDING);

        if(ret!=0){
            httpcCloseContext(&ctx);
            free(res);
            throw;
        }

        // Resize the resfer back down to our actual final size
        lastres = res;
        res = (u8*)realloc(res, rsize);
        if(res==nullptr){ // realloc() failed.
            httpcCloseContext(&ctx);
            free(lastres);
            throw;
        }

        httpcCloseContext(&ctx);

        return baseurl + (string)buf;
    } catch (exception e) {
        cout << e.what();
        return "An error occured, please make sure you have a working internet connection.";
    }
}

int main(int argc, const char *argv[]) {
    u32 *socubuf = nullptr;
    R_FAILED(socInit(socubuf, 0x100000));
    httpcInit(4 * 1024 * 1024);

    gfxInitDefault();
    consoleInit(GFX_TOP, nullptr);

    bool done = false;

    cout << "Press A to upload your latest error dump, START to exit.\n";

    // Main loop
    while (aptMainLoop()) {
        gspWaitForVBlank();
        gfxSwapBuffers();
        hidScanInput();

        // Your code goes here
        int kDown = hidKeysDown();
        if (kDown & KEY_START) {
            break; // break in order to return to hbmenu
        }

        if (kDown & KEY_A && !done) {
            cout << "Retrieving latest dump...\n";
            string filename = getLatestCoreDump();
            if (filename.empty()) {
                cout << "No dumps found!\n";
            } else {
                cout << "Found file: /luma/dumps/arm11/" + filename + "\n";
                cout << "Uploading...\n";
                string result = postFile(filename);
                cout << "Here is your link:\n" + result + "\n";
            }
            cout << "Press START to exit.";
            done = true;
        }
    }

    gfxExit();
    socExit();
    return 0;
}
