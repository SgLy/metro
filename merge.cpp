#include <cstdio>
#include <vector>
#include <iostream>
#include <algorithm>
#include <cmath>
#include <cstring>
#include <string>
#include <sstream>
#include <fstream>
using namespace std;

const char* SUBWAY_NAME[] = {
    "1",
    "2",
    "3",
    "3_North",
    "4",
    "5",
    "6",
    "8",
    "GuangFo",
    "APM"
};

void solve(const char *name) {
    char file1[100], file2[100];
    sprintf_s(file1, sizeof(file1), "data\\%s.txt", name);
}

int main() {
    int lines = sizeof(SUBWAY_NAME) / 4;
    for(int i = 0; i < lines; ++i)
        solve(SUBWAY_NAME[i]);
    return 0;
}

