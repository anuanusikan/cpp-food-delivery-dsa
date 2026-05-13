#pragma once
#include <string>
#include <vector>
#include <algorithm>
#include "../models/FoodItem.h"
#include "Trie.h"

using namespace std;

struct CategoryMenu {
    string first;
    vector<FoodItem> second;
};

class MenuManager {
private:
    vector<CategoryMenu> menuList;
    Trie trie;

    int findCategory(string category) {
        for (int i = 0; i < (int)menuList.size(); i++) {
            if (menuList[i].first == category) {
                return i;
            }
        }

        return -1;
    }

public:
    void seedDefaultMenu() {
        addItem({1, "Spicy Burger", 1200, "Burgers"});
        addItem({2, "Classic Burger", 900, "Burgers"});
        addItem({3, "Chicken Biryani", 1500, "Biryani"});
        addItem({4, "Beef Biryani", 1800, "Biryani"});
        addItem({5, "Margherita Pizza", 1100, "Pizzas"});
        addItem({6, "Pepperoni Pizza", 1300, "Pizzas"});
    }

    void addItem(const FoodItem& item) {
        int index = findCategory(item.category);

        if (index == -1) {
            CategoryMenu newCategory;
            newCategory.first = item.category;
            newCategory.second.push_back(item);
            menuList.push_back(newCategory);
        } else {
            menuList[index].second.push_back(item);
        }

        trie.insert(item.name);
    }

    vector<CategoryMenu> getMenu() {
        return menuList;
    }

    vector<CategoryMenu> searchMenu(string query) {
        if (query.empty()) {
            return menuList;
        }

        vector<CategoryMenu> result;

        string lowerQuery = query;
        transform(lowerQuery.begin(), lowerQuery.end(), lowerQuery.begin(), ::tolower);

        for (CategoryMenu category : menuList) {
            CategoryMenu matchedCategory;
            matchedCategory.first = category.first;

            for (FoodItem item : category.second) {
                string lowerName = item.name;
                transform(lowerName.begin(), lowerName.end(), lowerName.begin(), ::tolower);

                if (lowerName.find(lowerQuery) != string::npos) {
                    matchedCategory.second.push_back(item);
                }
            }

            if (!matchedCategory.second.empty()) {
                result.push_back(matchedCategory);
            }
        }

        return result;
    }

    bool hasFoodPrefix(string prefix) {
        return trie.startsWith(prefix);
    }

    int totalItems() {
        int count = 0;

        for (CategoryMenu category : menuList) {
            count += category.second.size();
        }

        return count;
    }
};