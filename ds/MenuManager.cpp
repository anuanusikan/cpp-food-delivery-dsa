#include "MenuManager.h"
#include <algorithm>

int MenuManager::findCategoryIndex(string category) {
    for (int i = 0; i < (int)menuDB.size(); i++) {
        if (menuDB[i].first == category) {
            return i;
        }
    }

    return -1;
}

void MenuManager::seedDefaultMenu() {
    addItem({1, "Spicy Burger", 1200, "Burgers"});
    addItem({2, "Classic Burger", 900, "Burgers"});
    addItem({3, "Chicken Biryani", 1500, "Biryani"});
    addItem({4, "Beef Biryani", 1800, "Biryani"});
    addItem({5, "Margherita", 1100, "Pizzas"});
    addItem({6, "Pepperoni", 1300, "Pizzas"});
}

void MenuManager::addItem(const FoodItem& item) {
    int index = findCategoryIndex(item.category);

    if (index == -1) {
        CategoryMenu newCategory;
        newCategory.first = item.category;
        newCategory.second.push_back(item);

        menuDB.push_back(newCategory);
    } else {
        menuDB[index].second.push_back(item);
    }

    trie.insert(item.name);
}

vector<CategoryMenu> MenuManager::getMenu() {
    return menuDB;
}

vector<CategoryMenu> MenuManager::searchMenu(string query) {
    if (query.empty()) {
        return menuDB;
    }

    vector<CategoryMenu> result;

    string qLower = query;
    transform(qLower.begin(), qLower.end(), qLower.begin(), ::tolower);

    for (CategoryMenu category : menuDB) {
        CategoryMenu matchedCategory;
        matchedCategory.first = category.first;

        for (FoodItem item : category.second) {
            string nameLower = item.name;
            transform(nameLower.begin(), nameLower.end(), nameLower.begin(), ::tolower);

            if (nameLower.find(qLower) != string::npos) {
                matchedCategory.second.push_back(item);
            }
        }

        if (!matchedCategory.second.empty()) {
            result.push_back(matchedCategory);
        }
    }

    return result;
}

int MenuManager::totalItems() {
    int count = 0;

    for (CategoryMenu category : menuDB) {
        count += category.second.size();
    }

    return count;
}