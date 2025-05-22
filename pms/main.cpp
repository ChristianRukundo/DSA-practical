// main.cpp
// This is the main entry point for the Inventory Management System application.
// Its primary role is to create an instance of the InventoryManager
// and start its execution loop.

#include "InventoryManager.h" // Include the declaration of the InventoryManager class.
#include <iostream>           // For std::cout, std::endl (optional, if main does any direct output).

// The main function - execution begins here.
int main()
{
    // Create an instance (object) of the InventoryManager class.
    // This will also call the InventoryManager constructor.
    InventoryManager manager;

    // Call the `run` method on the `manager` object.
    // This will start the main command processing loop of the inventory system.
    manager.run();

    // When `manager.run()` finishes (e.g., user enters "exit" command),
    // the program will reach this point.
    // Return 0 to the operating system to indicate successful program termination.
    return 0;
}