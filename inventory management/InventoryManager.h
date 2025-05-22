// InventoryManager.h
// This header file declares the InventoryManager class, which orchestrates
// the main functionalities of the inventory system.
#pragma once // Ensures this header is included only once per compilation unit.

#include <string>       // For std::string (used for filenames, commands, item properties).
#include <vector>       // For std::vector (used for command arguments, storing collections of items).
#include "Item.h"       // Definition of the Item struct.
#include "LinkedList.h" // Definition of the SinglyLinkedList class (might be used internally).

// The InventoryManager class encapsulates all inventory operations.
class InventoryManager
{
private:
    // --- Member Variables ---
    // Constant string for the CSV filename. Making it a member allows easy change if needed,
    // though it's compile-time constant here.
    const std::string csvFilename = "inventory.csv";

    // --- Private Helper Methods ---
    // These methods are used internally by the class and are not part of its public interface.

    // Converts a given string to lowercase for case-insensitive command/string comparisons.
    std::string toLowerCase(std::string s) const;

    // Parses a single line of text (from the CSV file) into an Item object.
    bool parseCsvLine(const std::string &line, Item &item) const;

    // Validates if a date string is in "YYYY-MM-DD" format and represents a real date.
    bool isValidDate(const std::string &dateStr) const;

    // Determines the next available auto-incrementing ID for a new item.
    int getNextAvailableId() const;

    // Loads all items from the CSV file into a std::vector<Item>.
    std::vector<Item> loadItemsFromCsv() const;

    // Writes a std::vector<Item> to the CSV file, overwriting its current content.
    bool writeItemsToCsv(const std::vector<Item> &items) const;

    // Helper to display a list of items to the console with a title.
    void displayItems(const std::vector<Item> &items, const std::string &title) const;

public:
    // --- Public Interface ---

    // Constructor: Called when an InventoryManager object is created.
    InventoryManager();

    // The main loop that drives the application, handling user input and command dispatch.
    void run();

    // --- Command Handler Methods ---
    // Each of these methods corresponds to a user command.

    // Handles the "itemadd" command to add a new item to the inventory.
    void addItemCmd(const std::vector<std::string> &args);

    // Handles the "itemslist" command to display all inventory items (default sort).
    void listItemsCmd();

    // Handles the "itemslistby" command to display items sorted by a specified field and order.
    void listItemsByCmd(const std::vector<std::string> &args);

    // Dispatcher for "itemfind" command, leading to search by ID or name.
    void findItemCmd(const std::vector<std::string> &args);
    // Finds and displays an item by its unique ID.
    void findItemById(const std::string &targetId) const;
    // Finds and displays items whose names contain the given search term.
    void findItemsByName(const std::string &searchTerm) const;

    // Handles the "itemdelete" command to remove an item from the inventory by its ID.
    void deleteItemCmd(const std::vector<std::string> &args);

    // Handles the "help" command to display available commands and their syntax.
    void displayHelpCmd() const;
};