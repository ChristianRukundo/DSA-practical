// Item.h
// This header file defines the structure for an inventory item.
// It includes necessary standard library headers for string and output stream.
#pragma once // Ensures this header is included only once per compilation unit, preventing redefinition errors.

#include <string>   // For std::string to store item ID, name, and date.
#include <iostream> // For std::ostream, used in the friend function to overload operator<< for easy printing.

// Structure to represent a single inventory item.
// In C++, struct members are public by default.
struct Item
{
    // Member variables representing the properties of an item.
    std::string id;               // Unique identifier for the item (auto-generated as a string).
    std::string name;             // Name of the item.
    int quantity;                 // Current stock quantity of the item.
    std::string registrationDate; // Date the item was registered, in "YYYY-MM-DD" format.

    // Constructor with default arguments.
    // Allows creating Item objects easily with or without initial values.
    // Example: Item newItem("1", "Book", 10, "2023-01-01");
    // Example: Item emptyItem; // Uses default values (empty strings, 0 quantity)
    Item(std::string _id = "", std::string _name = "", int _qty = 0, std::string _date = "")
        : id(_id), name(_name), quantity(_qty), registrationDate(_date) // Member initializer list
    {
    } // Empty constructor body

    // Friend function to overload the insertion operator (<<) for std::ostream.
    // This allows direct printing of Item objects to output streams like std::cout.
    // Useful for debugging and simple display, though `displayItems` in InventoryManager offers more formatting.
    // It's a 'friend' to access Item's members directly, though they are public here.
    friend std::ostream &operator<<(std::ostream &os, const Item &item)
    {
        os << "ID: " << item.id << ", Name: " << item.name
           << ", Qty: " << item.quantity << ", Date: " << item.registrationDate;
        return os; // Return the ostream to allow chaining (e.g., std::cout << item1 << item2;)
    }
};