#include <cmath>
#include <cstddef>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <fstream>
#include <string>
#include <utility>
#include <vector>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <iomanip>
#include <sstream>
#include <map>

typedef std::basic_string<unsigned char> ustring;
struct dict {
    int index;
    int prev;
    char c;
    dict(int x, int y, char ch): index(x), prev(y), c(ch) {};
};

int LZW(std::string filePath, int lzwWidth, std::vector<int> &byteCount, int &fileSize) {
        std::ifstream file (filePath, std::ios::in|std::ios::binary);
        if (!file) {
        std::cout << "Failed to open file" << "\n";
        return -1;
    }
        int lzwBits = 0;
        auto hash = [](const dict& p) { return 14 * p.prev + 33*static_cast<int>(p.c); };
        auto equal = [](const dict& p1, const dict& p2) { return  p1.prev == p2.prev && p1.c == p2.c;};
        std::unordered_set<dict, decltype(hash), decltype(equal)> dictLZW(8, hash, equal);
        char temp;
        int index3 = 256;
        int p = -1;
        for (auto i = 0; i < 256; ++i) {
            dict tmp(i,-1,static_cast<char>(i));
            dictLZW.insert(tmp);
        }
        while(file.get(temp)) {

        auto uTemp = static_cast<unsigned char>(temp);
        auto index1 = static_cast<size_t>(uTemp);
        //auto pair1 = std::make_pair(temp, p);
        dict tmp1(index3,p,temp);
        if (p==-1) {
            tmp1.index = static_cast<int>(temp);
            tmp1.prev = -1;
            tmp1.c = temp;
        }

        if (dictLZW.count(tmp1))  {
            dict tmp2 (-1,p,temp);
            auto find = dictLZW.find(tmp2);
            p = (*find).index;

        }
        else {
            if (dictLZW.size() < static_cast<size_t>(pow(2, lzwWidth))) {

                dictLZW.insert(tmp1);
                ++index3;
                lzwBits += lzwWidth < 8 ? 8: lzwWidth;

                p = static_cast<int>(index1);
            }
            else {
                lzwBits += lzwWidth < 8 ? 8: lzwWidth;
                p = static_cast<int>(index1);

            }

        }

        ++byteCount[index1];
        fileSize += file.gcount();
    }
    lzwBits += lzwWidth < 8 ? 8: lzwWidth;
    return lzwBits;
}
int main(int argc, char* argv[]) {
    //int argc, char* argv[]
    // auto lzwWidth = 18;
    // auto filePath = "/tmp_amd/cage/export/cage/5/z5238914/cs6771/tut02/test.txt";
    auto lzwWidth = 0;
    auto filePath = argv[argc-1];
    if (argc == 2) {
        lzwWidth = 12;
    }
    else {
        lzwWidth = std::stoi(argv[argc-2]);
    }



    std::ifstream file (filePath, std::ios::in|std::ios::binary);
    if (!file) {
        std::cout << "Failed to open file" << "\n";
        return -1;
    }
    std::vector<int> byteCount(256,0);

    // construct initial LZW dictionary


    auto fileSize = 0;
    auto lzwBits = 0;
    lzwBits = LZW(filePath, lzwWidth, byteCount, fileSize);


    if (fileSize == 0) {
        std::cout << "empty file!" << "\n";
        return -1;
    }

    // compute entropy
    auto entropy = 0.0;
    auto p_i = 0.0;
    for (auto byte:byteCount) {
        p_i = byte*1.0/fileSize;
        if(p_i != 0)
            entropy -= p_i * std::log2(p_i);
    }

    unsigned char c = 0;
    ustring temp1 (1,c);
    std::vector<int> symbolBits (256,0);
    int numSymbol = 0;
    auto index = 0;
    auto pair = std::pair<int,ustring> (0,temp1);
    std::priority_queue<std::pair<int,ustring>, std::vector<std::pair<int,ustring>>, std::greater<std::pair<int,ustring>>> q;
    for(auto it = byteCount.begin(); it != byteCount.end(); ++it) {
        temp1[0] = c;

        if (*it != 0) {
            pair.first = *it;
            pair.second = temp1;
            ++numSymbol;
            q.push(pair);
        }
        ++c;
        ++index;
    }


    auto node = q.top();

    // calculate the size of each symbol using a priority queue to emulate the huffman tree generation
    auto averageHuffman = 1.0;
    ustring temp2;
    auto weight = 0;
    while (q.size() > 1) {
        weight = 0;
        if (temp2.size() > 0) {
            temp2.clear();
        }
        node = q.top();
        weight += node.first;
        temp2.append(node.second);
        q.pop();
        node = q.top();
        weight += node.first;
        temp2.append(node.second);
        q.pop();
        for (auto it = temp2.begin(); it != temp2.end(); ++it) {

            auto index1 = static_cast<size_t>(*it);
            ++symbolBits[index1];
        }
        q.push(std::pair<int,ustring>(weight,temp2));

    }

    // calculate average huffman size
    auto totalBits = 0;
    if (numSymbol != 1) {
        for(size_t i = 0; i < 256; ++i) {
            totalBits +=  symbolBits.at(i) * byteCount.at(i);
        }
        averageHuffman = totalBits*1.0 / fileSize;
    }


    auto averageLZW = lzwBits*1.0 / fileSize;

    // output
    std::cout<<std::setiosflags(std::ios::fixed)<<std::setprecision(2);
    std::cout << entropy << "\n";
    std::cout << averageHuffman << "\n";
    std::cout << averageLZW << "\n";
}