# Food Delivery System (DSA Project)

A C++-based food delivery application implementing Data Structures and Algorithms concepts.

## Description

This project simulates a food delivery system where users can browse menus, add items to cart, place orders, and track deliveries. It demonstrates practical applications of various data structures like HashMaps, Tries, Graphs, Queues, and more.

## Features

- **User Authentication**: Login and registration system
- **Menu Management**: Browse and search food items using Trie data structure
- **Shopping Cart**: Add/remove items with HashMap implementation
- **Order Processing**: Queue-based order management system
- **Driver Assignment**: Graph-based routing for delivery drivers
- **Order History**: Track past orders
- **Analytics**: Performance metrics and insights
- **Web Interface**: HTML-based frontend for user interaction

## Prerequisites

- C++17 compatible compiler (GCC, Clang, MSVC)
- Windows (for ws2_32 library)
- Web browser for frontend

## Building the Project

1. Ensure all source files are in the correct directories
2. Compile using the following command:

```bash
g++ main.cpp ds/*.cpp Services/*.cpp -o server.exe -std=c++17 -lws2_32
```

## Running the Application

1. Execute the compiled server:

```bash
./server.exe
```

2. Open your web browser and navigate to `http://localhost:8080` (or the port specified in the code)

## Project Structure

```
├── main.cpp                 # Main application entry point
├── httplib.h               # HTTP server library
├── ds/                     # Data Structures implementations
│   ├── Analytics.cpp/h     # Analytics functionality
│   ├── Cart.cpp/h          # Shopping cart management
│   ├── Graph.cpp/h         # Graph for routing
│   ├── HashMap.cpp/h       # Hash table implementation
│   ├── MenuManager.cpp/h   # Menu handling
│   ├── OrderHistory.cpp/h  # Order history tracking
│   ├── OrderQueue.cpp/h    # Order queue management
│   └── Trie.cpp/h          # Trie for search functionality
├── models/                 # Data models
│   ├── Driver.h            # Driver entity
│   ├── FoodItem.h          # Food item entity
│   └── Order.h             # Order entity
├── Services/               # Business logic services
│   ├── AuthService.cpp/h   # Authentication service
│   ├── DriverService.cpp/h # Driver management
│   └── OrderService.cpp/h  # Order processing
└── public/                 # Web frontend
    ├── index.html          # Main page
    └── login.html          # Login page
```

## Technologies Used

- **C++17**: Core programming language
- **Data Structures**: Custom implementations of HashMap, Trie, Graph, Queue
- **HTTP Server**: Lightweight HTTP server for web interface
- **HTML/CSS**: Frontend interface

## Data Structures Implemented

- **HashMap**: For efficient key-value storage (cart, user data)
- **Trie**: For fast prefix-based search in menus
- **Graph**: For delivery route optimization
- **Queue**: For order processing pipeline
- **Linked Lists**: For various list operations

## Contributing

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Test thoroughly
5. Submit a pull request

## License

This project is for educational purposes demonstrating DSA concepts.