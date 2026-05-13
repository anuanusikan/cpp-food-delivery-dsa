#pragma once
#include <string>
#include <vector>
#include <sstream>
#include <algorithm>

using namespace std;

class Analytics {
private:
    vector<pair<string, int>> itemCounter;

    string cleanText(string text) {
        size_t start = text.find_first_not_of(" ");
        size_t end = text.find_last_not_of(" ");

        if (start == string::npos) {
            return "";
        }

        return text.substr(start, end - start + 1);
    }

    int findItem(string item) {
        for (int i = 0; i < (int)itemCounter.size(); i++) {
            if (itemCounter[i].first == item) {
                return i;
            }
        }

        return -1;
    }

public:
    void countItems(string items) {
        stringstream ss(items);
        string item;

        while (getline(ss, item, ',')) {
            item = cleanText(item);

            if (!item.empty()) {
                int index = findItem(item);

                if (index == -1) {
                    itemCounter.push_back({item, 1});
                } else {
                    itemCounter[index].second++;
                }
            }
        }
    }

    vector<pair<string, int>> getTop(int limit) {
        vector<pair<string, int>> result = itemCounter;

        sort(result.begin(), result.end(), [](const auto& a, const auto& b) {
            return a.second > b.second;
        });

        if ((int)result.size() > limit) {
            result.resize(limit);
        }

        return result;
    }

    int uniqueItemCount() const {
        return itemCounter.size();
    }
};