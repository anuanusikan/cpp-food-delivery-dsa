#pragma once
#include <string>
#include <vector>
#include "../models/FoodItem.h"
#include "Trie.h"

using namespace std;

struct CategoryMenu {
    string first;
    vector<FoodItem> second;
};

class MenuManager {
private:
    vector<CategoryMenu> menuDB;
    Trie trie;

    int findCategoryIndex(string category);

public:
    void seedDefaultMenu();
    void addItem(const FoodItem& item);
    vector<CategoryMenu> getMenu();
    vector<CategoryMenu> searchMenu(string query);
    int totalItems();
};