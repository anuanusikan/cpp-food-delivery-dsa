#pragma once
#include <string>
#include <vector>

using namespace std;

class Analytics {
private:
    vector<pair<string, int>> counter;

    int findItemIndex(string item);

public:
    void countItems(string items);
    vector<pair<string, int>> getTop(int limit);
    int uniqueItemCount() const;
};