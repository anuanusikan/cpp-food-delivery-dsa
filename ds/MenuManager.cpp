#include "MenuManager.h"
#include <algorithm>

using namespace std;

int MenuManager::findCategoryIndex(string category) {
    for (int i = 0; i < (int)menuDB.size(); i++) {
        if (menuDB[i].first == category) {
            return i;
        }
    }
    return -1;
}

void MenuManager::seedDefaultMenu() {
    // Burgers
    addItem({1,  "Spicy Burger",       1200, "Burgers"});
    addItem({2,  "Classic Burger",      900, "Burgers"});
    addItem({3,  "Double Patty Burger",1600, "Burgers"});

    // Biryani
    addItem({4,  "Chicken Biryani",    1500, "Biryani"});
    addItem({5,  "Beef Biryani",       1800, "Biryani"});
    addItem({6,  "Mutton Biryani",     2000, "Biryani"});

    // Pizzas
    addItem({7,  "Margherita",         1100, "Pizzas"});
    addItem({8,  "Pepperoni",          1300, "Pizzas"});
    addItem({9,  "BBQ Chicken",        1500, "Pizzas"});

    // Drinks
    addItem({10, "Coke",                300, "Drinks"});
    addItem({11, "Mineral Water",       100, "Drinks"});
    addItem({12, "Mango Shake",         450, "Drinks"});

    // Sides
    addItem({13, "French Fries",        500, "Sides"});
    addItem({14, "Garlic Bread",        400, "Sides"});
    addItem({15, "Coleslaw",            350, "Sides"});
}

void MenuManager::addItem(const FoodItem& item) {
    int index = findCategoryIndex(item.category);

    if (index == -1) {
        CategoryMenu newCat;
        newCat.first = item.category;
        newCat.second.push_back(item);
        menuDB.push_back(newCat);
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

    for (CategoryMenu cat : menuDB) {
        CategoryMenu matched;
        matched.first = cat.first;

        for (FoodItem item : cat.second) {
            string nameLower = item.name;
            transform(nameLower.begin(), nameLower.end(), nameLower.begin(), ::tolower);

            if (nameLower.find(qLower) != string::npos) {
                matched.second.push_back(item);
            }
        }

        if (!matched.second.empty()) {
            result.push_back(matched);
        }
    }

    return result;
}

int MenuManager::totalItems() {
    int count = 0;
    for (CategoryMenu cat : menuDB) {
        count += cat.second.size();
    }
    return count;
}