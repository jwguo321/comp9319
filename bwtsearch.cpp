#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <ios>
#include <iostream>
#include <iterator>
#include <map>
#include <stdio.h>
#include <string>
#include <utility>
#include <vector>
/*
   The ASCII values is up to 126, so we can use assume the maximum number
   of unique charaters is 127(96 actually). Since the txt file may be up
   to 100MB, and the memory constraint is 16MB, we divide the OCC array
   into blocks, each block has 10000 characters, so the number of blocks
   is 100*1024*1024/10000 + 1 = 10486.

   DECODING BWT FILE HAS NOT BEEN IMPLEMENTED IN THIS PROGRAM.
*/
#define CSIZE 96
#define k 10000
#define BLOCKS 100 * 1024 * 1024 / k + 1
// cast a int to unsigned long.
#define ULONG(i) static_cast<size_t>(i)
std::array<std::array<int, CSIZE>, static_cast<size_t>(BLOCKS)> occ;
std::array<int, CSIZE> c;
FILE* file;
char buffer[k];
// mode 1: output duplicate, mode 2: output unique number, mode 3: output details of unique
// mode 4: outputfile
int mode = -1;
size_t last_block = 0;
int bytes = 0;
// return the index of a char in occ array
int mapping(char ch) {
	if (ch == 10) {
		return 0;
	}
	return ch - 32 + 1;
}
// find the number of occurrences of char c in prefix L[1,q]
int occur(char ch, int q) {
	int ch_index = mapping(ch);
	int b = q / k;
	int offset = b * k;
	int num = 0;
	int bytes_to_read = q;
	if (b != 0) {
		num += occ[static_cast<size_t>(b - 1)][static_cast<size_t>(ch_index)];
		bytes_to_read = q - b * k;
	}
	fseek(file, offset, SEEK_SET);
	fread(buffer, sizeof(char), k, file);
	for (auto i = 0; i < bytes_to_read; ++i) {
		if (ch == buffer[i]) {
			++num;
		}
	}
	return num;
}
// find the position of n-th character in bwt file(use binary search)
int get_pos(int n, char ch) {
	size_t ub = last_block;
	size_t lb = 0;
	int pos = 0;
	int num = 0;
	auto ch_index = ULONG(mapping(ch));
	size_t block = 0;
	while (true) {
		block = (ub + lb) / 2;
		if (block == 0 and occ[block][ch_index] >= n) {
			break;
		}
		if (occ[block][ch_index] < n && occ[block + 1][ch_index] >= n) {
			block += 1;
			break;
		}
		if (occ[block][ch_index] >= n) {
			ub = block;
		}
		else if (occ[block + 1][ch_index] < n) {
			lb = block;
		}
	}
	if (block == 0) {
		num = 0;
	}
	else {
		num = occ[block - 1][ch_index];
	}
	pos = static_cast<int>(block * k);
	fseek(file, pos, SEEK_SET);
	auto byte_readed = fread(buffer, sizeof(char), k, file);
	for (size_t i = 0; i < byte_readed; ++i) {
		++pos;
		if (ch == buffer[i]) {
			++num;
		}
		if (num == n) {
			break;
		}
	}
	return pos;
}
auto back_search(const char* p, size_t last_row) {
	auto i = strlen(p) - 1;
	auto ch = p[i];
	auto ch_index = mapping(ch);
	auto first = c[ULONG(ch_index)] + 1;
	auto last = first;
	auto index = ch_index + 1;
	// get the last row of current character
	while (true) {
		if (index >= CSIZE) {
			last = first + occ[last_row][ULONG(ch_index)] - 1;
			break;
		}
		if (c[ULONG(index)] != 0) {
			last = c[ULONG(index)];
			break;
		}
		++index;
	}
	while (first <= last && i >= 1) {
		ch = p[i - 1];
		ch_index = mapping(ch);
		first = c[ULONG(ch_index)] + occur(ch, first - 1) + 1;
		last = c[ULONG(ch_index)] + occur(ch, last);
		i = i - 1;
	}
	std::pair<int, int> pair(-1, -1);
	if (last < first) {
		return pair;
	}

	pair.first = first;
	pair.second = last;
	return pair;
}
// get line number fron a string
auto get_line_number(std::string s) {
	if (s.find(' ') == std::string::npos) {
		return -1;
	}
	auto first_space = s.find_first_of(' ', 0);
	std::string line_number;
	for (size_t i = 0; i < first_space; ++i) {
		line_number.push_back(s[i]);
	}
	return std::stoi(line_number);
}

auto forward_search(std::pair<int, int> pr, const char* pt) {
	std::map<int, char> sorted_c;
	sorted_c.insert(std::pair<int, char>(c[0], '\n'));
	for (auto i = 1; i < 96; ++i) {
		sorted_c.insert(std::pair<int, char>(c[ULONG(i)], i + 32 - 1));
	}
	std::vector<std::string> res_str;
	int size = pr.second - pr.first + 1;
	char buf[1];
	for (auto i = 0; i < size; ++i) {
		std::string temp;
		auto line = pr.first + i;
		fseek(file, line - 1, SEEK_SET);
		fread(buf, sizeof(char), 1, file);
		auto ch = buf[0];
		auto ch_index = mapping(ch);
		auto next = occur(ch, line) + c[ULONG(ch_index)];
		// find previous part of pattern
		if (ch == '\n') {
		}
		else {
			temp.push_back(ch);
			while (true) {
				if (ch == '\n') {
					break;
				}
				fseek(file, next - 1, SEEK_SET);
				fread(buf, sizeof(char), 1, file);
				ch = buf[0];
				ch_index = mapping(ch);
				if (ch != '\n') {
					temp.insert(0, 1, ch);
				}

				next = occur(ch, next) + c[ULONG(ch_index)];
			}
			// res_str.push_back(temp);
		}
		// find tail
		std::string tail;
		ch = pt[0];
		ch_index = mapping(ch);
		int nth = line - c[ULONG(ch_index)];
		// tail.push_back(ch);
		while (true) {
			if (ch == '\n') {
				tail.push_back(ch);
				break;
			}
			auto pos = get_pos(nth, ch);
			fseek(file, pos - 1, SEEK_SET);
			fread(buf, sizeof(char), 1, file);
			ch = buf[0];
			tail.push_back(ch);
			// get next char
			for (auto it = sorted_c.begin(); it != std::prev(sorted_c.end()); ++it) {
				auto it1 = std::next(it, 1);
				if (pos > it->first && pos <= it1->first) {
					ch = it->second;
					break;
				}
				if (it1 == std::prev(sorted_c.end())) {
					ch = it1->second;
				}
			}
			ch_index = mapping(ch);
			nth = pos - c[ULONG(ch_index)];
		}
		std::string res = temp + tail;

		if (std::find(res_str.begin(), res_str.end(), res) == res_str.end()) {
			res_str.push_back(res);
		}
	}
	std::sort(res_str.begin(), res_str.end());
	for (auto it = res_str.begin(); it != res_str.end(); ++it) {
		std::cout << *it;
	}
	return res_str;
}
// remove duplicate terms
auto unique(std::pair<int, int> pr) {
	int size = pr.second - pr.first + 1;
	std::vector<int> res;
	std::vector<std::string> res_str;
	char buf[1];
	for (auto i = 0; i < size; ++i) {
		// current line in L(Last column of a bwt transform)
		bool space = false;
		auto line = pr.first + i;
		fseek(file, line - 1, SEEK_SET);
		fread(buf, sizeof(char), 1, file);
		auto ch = buf[0];
		std::string temp(1, ch);
		auto ch_index = mapping(ch);
		auto next = occur(ch, line) + c[ULONG(ch_index)];
		while (true) {
			if (ch == '\n' && space) {
				break;
			}
			if (ch == ' ') {
				space = true;
			}
			fseek(file, next - 1, SEEK_SET);
			fread(buf, sizeof(char), 1, file);
			ch = buf[0];
			ch_index = mapping(ch);
			if (ch != '\n') {
				temp.insert(0, 1, ch);
			}

			next = occur(ch, next) + c[ULONG(ch_index)];
		}

		int t = get_line_number(temp);
		auto it = std::find(res.begin(), res.end(), t);
		if (it == res.end()) {
			res.push_back(t);
			// print lines into console
			if (mode == 1) {
			}
			res_str.push_back(temp);
		}
	}
	std::sort(res_str.begin(), res_str.end());

	return res;
}

auto decode(std::pair<int, int> pr) {
	auto myfile = std::fstream("decode.txt", std::ios::out | std::ios::binary);
	std::vector<int> res;
	std::vector<std::string> res_str;
	char buffer1[1024 * 1024];
	char buf[1];
	int byteread = 0;
	// current line in L(Last column of a bwt transform)
	auto line = pr.second;
	fseek(file, line - 1, SEEK_SET);
	fread(buf, sizeof(char), 1, file);
	auto ch = buf[0];
	buffer1[0] = ch;
	++byteread;
	auto ch_index = mapping(ch);
	auto next = occur(ch, line) + c[ULONG(ch_index)];
	while (byteread <= bytes) {
		if (byteread % (1024 * 1024) == 0) {
			myfile.write(&buffer1[0], 1024 * 1024);
		}
		fseek(file, next - 1, SEEK_SET);
		fread(buf, sizeof(char), 1, file);
		ch = buf[0];
		ch_index = mapping(ch);

		buffer1[byteread % 1024 * 1024] = ch;
		++byteread;

		next = occur(ch, next) + c[ULONG(ch_index)];
	}
	if (byteread % (1024 * 1024) != 0) {
		myfile.write(&buffer1[0], byteread % (1024 * 1024));
	}
	myfile.close();
}
auto main(int argc, char** argv) -> int {
	const char* f;
	const char* pattern;
	mode = 3;
	if (argc < 4) {
		std::cout << "wrong arguments"
		          << "\n";
	}
	if (argc == 4) {
		mode = 3;
		f = argv[1];
		pattern = argv[3];
	}
	else if (argc == 5) {
		f = argv[2];
		pattern = argv[4];
		if (strncmp(argv[1], "-m", 2) == 0) {
			mode = 1;
		}
		else if (strncmp(argv[1], "-n", 2) == 0) {
			mode = 2;
		}
		else {
			mode = 4;
		}
	}
	file = std::fopen(f, "r");

	if (file == nullptr) {
		std::cout << "Unable to open file"
		          << "\n";
		return -1;
	}
	size_t block = 0;
	while (std::feof(file) != 1) {
		auto bytes_readed = fread(buffer, sizeof(char), k, file);
		bytes += bytes_readed;
		if (block > 0) {
			for (size_t i = 0; i < CSIZE; ++i) {
				occ[block][i] = occ[block - 1][i];
			}
		}
		for (size_t i = 0; i < bytes_readed; ++i) {
			auto index = mapping(buffer[i]);
			++occ[block][static_cast<size_t>(index)];
		}
		++block;
	}

	// generate C
	last_block = block - 1;
	c[0] = 0;
	auto previous = occ[last_block][0];
	for (size_t i = 1; i < CSIZE; ++i) {
		// current char existed in the text file
		if (occ[last_block][i] != 0) {
			c[i] = previous;
			previous += occ[last_block][i];
		}
	}
	// -m

	std::pair<int, int> result = back_search(pattern, last_block);
	if (result.first == -1) {
		if (mode == 1 || mode == 2) {
			std::cout << "0"
			          << "\n";
		}
	}
	else {
		if (mode == 1) {
			std::cout << result.second - result.first + 1 << "\n";
		}
		if (mode == 2) {
			auto u = unique(result).size();
			std::cout << u << "\n";
		}
		if (mode == 3) {
			auto u = forward_search(result, pattern);
		}
		// auto u = forward_search(result, patern).size();
		// std::cout << "unique: " << u << "\n";
		// decode(result);
	}

	return 0;
}