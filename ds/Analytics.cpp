#include "Analytics.h"
#include <sstream>
#include <algorithm>

static string trimString(string s) {
    size_t start = s.find_first_not_of(" ");
    size_t end = s.find_last_not_of(" ");

    if (start == string::npos) {
        return "";
    }

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
        item = trimString(item);

        if (!item.empty()) {
            int index = findItemIndex(item);

            if (index == -1) {
                counter.push_back({item, 1});
            } else {
                counter[index].second++;
            }
        }
    }
}

vector<pair<string, int>> Analytics::getTop(int limit) {
    vector<pair<string, int>> result = counter;

    sort(result.begin(), result.end(), [](const auto& a, const auto& b) {
        return a.second > b.second;
    });

    if ((int)result.size() > limit) {
        result.resize(limit);
    }

    return result;
}

int Analytics::uniqueItemCount() const {
    return counter.size();
}