#pragma once
#pragma warning(disable:4996)

#include <fstream>
#include <vector>
#include <unordered_set>
#include <cstdio>

#include "gtl/phmap.hpp"

#include "definition.h"

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

class HashIO {
public:
	HashIO() {

	}

	template<class T>
	void write(T& s, string filename) {
		vector<ull> v;
		for (auto& item : s) v.push_back(item);
		FILE* f = fopen(filename.c_str(), "wb");
		uint64_t size = v.size();
		cout << size << '\n';
		fwrite(&size, 8, 1, f);
		fwrite(v.data(), sizeof(v[0]), size, f);
		fclose(f);
	}

	template<class T>
	void read(T& s, string filename) {
		s.clear();
		FILE* f = fopen(filename.c_str(), "rb");
		uint64_t size;
		fread(&size, 8, 1, f);
		vector<ull> v(size);
		fread(v.data(), sizeof(v[0]), size, f);
		fclose(f);
		for (auto& item : v) s.insert(item);
	}


	template<class T>
	void write(unordered_map<ull, set<T>>& s, string filename) {
		FILE* f = fopen(filename.c_str(), "wb");
		uint64_t size = s.size();
		fwrite(&size, 8, 1, f);
		for (auto& item : s) {
			fwrite(&item.first, 8, 1, f);
			vector<T> v;
			for (auto& val : item.second) {
				v.push_back(val);
			}
			size = v.size();
			fwrite(&size, 8, 1, f);
			fwrite(v.data(), sizeof(v[0]), size, f);
		}
		fclose(f);
	}

	template<class T>
	void read(unordered_map<ull, set<T>>& s, string filename) {
		s.clear();
		FILE* f = fopen(filename.c_str(), "rb");
		uint64_t size;
		fread(&size, 8, 1, f);
		for (int i = 0; i < size; ++i) {
			ull val;
			fread(&val, sizeof(val), 1, f);
			uint64_t size2;
			fread(&size2, 8, 1, f);
			vector<T> v(size2);
			fread(v.data(), sizeof(v[0]), size2, f);
			for (auto& item : v) s[val].insert(item);
		}
		fclose(f);
	}
};