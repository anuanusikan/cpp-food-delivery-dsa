#include "Analytics.h"
#include <iostream>
#include <sstream>

using namespace std;

// trim spaces from both sides of a string
static string trimStr(string s) {
    int start = 0;
    int end = (int)s.size() - 1;

    while (start <= end && s[start] == ' ') start++;
    while (end >= start && s[end] == ' ') end--;

    if (start > end) return "";
    return s.substr(start, end - start + 1);
}

int Analytics::findItemIndex(string item) {
    for (int i = 0; i < (int)counter.size(); i++) {
        if (counter[i].first == item) {
            return i;
        }
    }
    return -1;
}

void Analytics::countItems(string items) {
    stringstream ss(items);
    string item;

    while (getline(ss, item, ',')) {
        item = trimStr(item);

        if (!item.empty()) {
            int idx = findItemIndex(item);
            if (idx == -1) {
                counter.push_back({item, 1});
                cout << "[Analytics] New item tracked: " << item << "\n";
            } else {
                counter[idx].second++;
                cout << "[Analytics] " << item << " ordered " << counter[idx].second << " times\n";
            }
        }
    }
}

// using selection sort to find top items - sorting by count descending
vector<pair<string, int>> Analytics::getTop(int limit) {
    vector<pair<string, int>> result = counter;

    int n = result.size();

    // selection sort - simple but works fine for small data
    for (int i = 0; i < n - 1; i++) {
        int maxIdx = i;
        for (int j = i + 1; j < n; j++) {
            if (result[j].second > result[maxIdx].second) {
                maxIdx = j;
            }
        }
        if (maxIdx != i) {
            pair<string, int> temp = result[i];
            result[i] = result[maxIdx];
            result[maxIdx] = temp;
        }
    }

    if ((int)result.size() > limit) {
        result.resize(limit);
    }

    return result;
}

int Analytics::uniqueItemCount() const {
    return (int)counter.size();
}