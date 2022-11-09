#pragma once
#pragma warning(disable : 4996)

#include <cstdio>
#include <fstream>
#include <unordered_set>
#include <vector>

#include "definition.h"
#include "gtl/phmap.hpp"

using namespace std;

class Writer {
   public:
    ofstream file;
    vector<char> buffer;
    static constexpr int MX_BUFFER = 1024;
    Writer() {
        buffer.reserve(MX_BUFFER);
    }

    virtual ~Writer() {
        if (file) {
            file.close();
        }
    }

    Writer(const Writer&) = default;
    Writer& operator=(const Writer&) = default;

    void openFile(string filename) {
        file = ofstream(filename);
    }

    void writeMove(opData data) {
        unionOpData u_opData;
        u_opData.op_data = data;
        writeBuffer(u_opData.data[0]);
        writeBuffer(u_opData.data[1]);
    }

    void writeCount(int cnt) {
        writeBuffer((char)cnt);
    }

    void closeFile() {
        file.write(&buffer[0], buffer.size());
        buffer.clear();
        file.close();
    }

   private:
    void writeBuffer(char c) {
        buffer.push_back(c);
        if (buffer.size() > MX_BUFFER) {
            file.write(&buffer[0], buffer.size());
            buffer.clear();
        }
    }
};