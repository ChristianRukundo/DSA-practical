// InventoryManager.cpp
// This file contains the implementation of the InventoryManager class, which is the heart
// of our inventory management system. It handles command processing, file I/O,
// and the core logic for managing inventory items.

// ============== Includes ==============
// These are necessary header files that provide functionalities used in this class.
#include "InventoryManager.h" // The declaration of our InventoryManager class. Essential!

#include <iostream>  // For standard input (std::cin) and output (std::cout, std::cerr).
#include <fstream>   // For file stream operations (std::ifstream for reading, std::ofstream for writing).
#include <sstream>   // For std::stringstream, which allows treating strings as streams for easy parsing.
#include <vector>    // For std::vector, a dynamic array used to store collections of items or arguments.
#include <algorithm> // For various useful algorithms like std::sort, std::transform, std::reverse.
#include <limits>    // For std::numeric_limits, used here to clear the input buffer effectively after an error.
#include <cctype>    // For ::isdigit and ::tolower (C-style functions, often pulled into std too)

// ============== Private Helper Implementations ==============
// These functions are internal utilities for the InventoryManager class. They are not meant
// to be called directly from outside the class, hence they are declared 'private' in the header.

/**
 * @brief Converts a given string to lowercase.
 * @param s The input string.
 * @return A new string with all alphabetic characters converted to lowercase.
 *
 * Time Complexity: O(N), where N is the length of the string `s`.
 *                  std::transform iterates through each character once.
 * Space Complexity: O(N) because `s` is passed by value (a copy is made for the parameter),
 *                   and a new string is returned. The transformation happens on the copy.
 */
std::string InventoryManager::toLowerCase(std::string s) const // `const` because it doesn't modify InventoryManager's members
{
    // std::transform applies a function to a range of elements.
    // s.begin(), s.end(): The input range (all characters in the string `s`).
    // s.begin(): The output range starts at the beginning of `s`, so it modifies the copy of `s` in place.
    // The lambda function `[](unsigned char c){ return std::tolower(c); }` is applied to each character.
    // `std::tolower` (from <cctype> or <locale>) converts an uppercase character to lowercase; others are unchanged.
    // Using `unsigned char` in lambda is a good practice with `std::tolower` to handle all char values correctly.
    std::transform(s.begin(), s.end(), s.begin(),
                   [](unsigned char c)
                   { return ::tolower(c); }); // Using C-style ::tolower from <cctype> for simplicity
    return s;                                 // Return the modified (now lowercase) string.
}

/**
 * @brief Validates if a string represents a date in YYYY-MM-DD format and is a logically correct date.
 * @param dateStr The date string to validate.
 * @return true if the date is valid, false otherwise.
 *
 * Time Complexity: O(1) because the date string length is fixed (10 chars for format check).
 *                  String operations like substr, stoi on small fixed-size strings are effectively constant time.
 *                  The loops for ::isdigit also run a fixed number of times (4, 2, 2).
 * Space Complexity: O(1) for the few temporary std::string variables (yearStr, monthStr, dayStr)
 *                    which are of fixed small size.
 */
bool InventoryManager::isValidDate(const std::string &dateStr) const
{
    // Check 1: Overall length must be 10 characters (e.g., "2023-10-27").
    if (dateStr.length() != 10)
    {
        return false;
    }

    // Check 2: Hyphens must be at the 5th and 8th positions (index 4 and 7).
    if (dateStr[4] != '-' || dateStr[7] != '-')
    {
        return false;
    }

    // Extract year, month, and day parts as strings.
    // substr(startIndex, length)
    std::string yearStr = dateStr.substr(0, 4);
    std::string monthStr = dateStr.substr(5, 2);
    std::string dayStr = dateStr.substr(8, 2);

    int year, month, day; // To store the integer values of date components.

    try
    {
        // Check 3: Ensure year, month, day parts consist only of digits, then convert to integers.
        for (char c : yearStr) // Range-based for loop (C++11)
            if (!::isdigit(c)) // `::isdigit` (from <cctype>) checks if a character is a decimal digit.
                return false;
        year = std::stoi(yearStr); // `std::stoi` converts string to integer. Can throw exceptions.

        for (char c : monthStr)
            if (!::isdigit(c))
                return false;
        month = std::stoi(monthStr);

        for (char c : dayStr)
            if (!::isdigit(c))
                return false;
        day = std::stoi(dayStr);
    }
    catch (const std::invalid_argument &ia) // `std::stoi` throws if conversion is not possible (e.g., "abc").
    {
        // This catch handles cases where a non-digit might have slipped past ::isdigit
        // or if the string is empty after substr (though length checks should prevent that).
        return false;
    }
    catch (const std::out_of_range &oor) // `std::stoi` throws if converted value is out of int range.
    {
        // Unlikely for typical date components (year, month, day) to exceed int limits, but good for completeness.
        return false;
    }

    // Check 4: Basic logical range checks for year, month, and day.
    if (year < 1000 || year > 9999) // A sensible range for a 4-digit year. Could be stricter (e.g., 1900-2100).
        return false;
    if (month < 1 || month > 12) // Month must be between 1 and 12.
        return false;
    if (day < 1 || day > 31) // Day must be between 1 and 31 (this is a general check, refined below).
        return false;

    // Check 5: More specific check for the number of days in each month, including leap years for February.
    if (month == 4 || month == 6 || month == 9 || month == 11) // Months with 30 days.
    {
        if (day > 30)
            return false;
    }
    else if (month == 2) // February - requires leap year check.
    {
        // Leap year condition:
        // A year is a leap year if it is divisible by 4,
        // unless it is divisible by 100 but not by 400.
        bool isLeapYear = (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
        if (isLeapYear)
        {
            if (day > 29)
                return false; // Leap February has 29 days.
        }
        else
        {
            if (day > 28)
                return false; // Non-leap February has 28 days.
        }
    }
    // For months with 31 days (1, 3, 5, 7, 8, 10, 12), the earlier `day <= 31` check is sufficient.

    return true; // If all checks passed, the date is considered valid.
}

/**
 * @brief Determines the next available auto-incrementing ID for new items.
 *        It scans the CSV file for the highest existing numeric ID and returns that ID + 1.
 * @return The next integer ID to be used.
 *
 * Time Complexity: O(L * M_id_avg), where L is the number of lines in the CSV, and M_id_avg is the
 *                  average length of an ID string (due to std::getline for ID and std::stoi).
 *                  The function reads the entire CSV file in the worst case.
 * Space Complexity: O(M_line_max), where M_line_max is the maximum length of a line read from the file,
 *                   used for storing the `line` buffer and `idStr`.
 */
int InventoryManager::getNextAvailableId() const
{
    std::ifstream inFile(csvFilename); // Open the CSV file for reading.
    int maxId = 0;                     // Initialize maxId to 0. If the file is empty or contains no valid numeric IDs,
                                       // the first ID generated will be 0 + 1 = 1.

    if (inFile.is_open()) // Proceed only if the file was successfully opened.
    {
        std::string line;                  // Buffer to store each line read from the file.
        std::string idStr;                 // Buffer to store the ID part of the line as a string.
        while (std::getline(inFile, line)) // Read the file line by line.
        {
            if (line.empty()) // Skip any empty lines in the CSV.
                continue;

            std::stringstream ss(line); // Use a stringstream to easily parse the ID from the line.
            // Read from the stringstream `ss` into `idStr` until a comma ',' is encountered.
            // This extracts the first field, which should be the item ID.
            if (std::getline(ss, idStr, ','))
            {
                try
                {
                    int currentId = std::stoi(idStr); // Attempt to convert the ID string to an integer.
                    if (currentId > maxId)            // If this ID is greater than the current maximum ID found,
                    {
                        maxId = currentId; // update maxId.
                    }
                }
                catch (const std::exception &) // Catches std::invalid_argument or std::out_of_range from std::stoi.
                {
                    // If an ID in the CSV is not a valid integer (e.g., "A101", or corrupt data),
                    // this line will be ignored for the purpose of finding the maximum numeric ID.
                    // Optionally, a warning could be logged here:
                    // std::cerr << "Warning: Non-integer ID '" << idStr << "' found in CSV during max ID scan." << std::endl;
                }
            }
        }
        inFile.close(); // Close the file after reading.
    }
    // If the file wasn't opened (e.g., doesn't exist), maxId remains 0.
    return maxId + 1; // The next available ID is one greater than the highest valid numeric ID found.
}

/**
 * @brief Parses a single line from the CSV string into an Item object.
 * @param line The CSV string line to parse. Assumed format: id,name,quantity,date
 * @param item A reference to an Item object to be populated with data from the line.
 * @return true if parsing was successful and all fields were extracted, false otherwise.
 *
 * Time Complexity: O(M), where M is the length of the `line`.
 *                  Operations like creating stringstream, std::getline, and std::stoi (on small parts)
 *                  are generally proportional to the amount of data they process from the line.
 * Space Complexity: O(M) in the worst case for the std::stringstream copy of `line` and the temporary `field` string.
 */
bool InventoryManager::parseCsvLine(const std::string &line, Item &item) const // Populates `item` by reference
{
    std::stringstream ss(line); // Create a stringstream from the input line for easy field extraction.
    std::string field;          // Temporary string to hold individual fields, like quantity before conversion to int.

    // Attempt to parse each field based on the expected CSV format.
    // std::getline extracts characters into the target string until the delimiter is found (or EOF).
    // It returns a reference to the stream, which evaluates to true in boolean context if successful.

    // 1. Parse Item ID
    if (!std::getline(ss, item.id, ','))
        return false; // If getline fails, the line is malformed or incomplete.
    // 2. Parse Item Name
    if (!std::getline(ss, item.name, ','))
        return false;
    // 3. Parse Quantity (first as string, then convert to int)
    if (!std::getline(ss, field, ','))
        return false; // Read quantity string into `field`.
    try
    {
        item.quantity = std::stoi(field); // Convert the quantity string to an integer.
    }
    catch (const std::exception &) // Catches both std::invalid_argument and std::out_of_range.
    {
        // If conversion fails (e.g., `field` is "abc" or a number too large for int).
        // std::cerr << "Warning: Invalid quantity string '" << field << "' in CSV line: \"" << line << "\"" << std::endl; // Optional debug
        return false; // Parsing fails for this line.
    }
    // 4. Parse Registration Date
    // For the last field, std::getline without a delimiter reads until the end of the line in the stringstream.
    if (!std::getline(ss, item.registrationDate))
        return false;

    return true; // All fields were successfully parsed.
}

/**
 * @brief Loads all items from the CSV file into a std::vector<Item>.
 * @return A vector of Item objects. Returns an empty vector if the file is not found, is empty, or all lines are malformed.
 *
 * Time Complexity: O(L * P_avg), where L is the number of lines in the CSV, and P_avg is the
 *                  average time to parse one line (which is itself proportional to average line length).
 *                  This is dominated by reading and parsing the entire file.
 * Space Complexity: O(N * S_item + M_line_max), where N is the number of items successfully parsed,
 *                   S_item is the average size of an Item object in memory, and M_line_max is for the `line` buffer.
 *                   The `items` vector stores all Item objects.
 */
std::vector<Item> InventoryManager::loadItemsFromCsv() const
{
    std::vector<Item> items;           // The vector to store loaded Item objects.
    std::ifstream inFile(csvFilename); // Open the CSV file for reading.

    if (!inFile.is_open()) // If the file couldn't be opened (e.g., it doesn't exist).
    {
        // This is not necessarily an error for loading; it might just mean the inventory is new/empty.
        // std::cerr << "Info: CSV file '" << csvFilename << "' not found or could not be opened. Starting with empty inventory." << std::endl;
        return items; // Return an empty vector.
    }

    std::string line; // Buffer to store each line read from the file.
    // Note: The original `past paper` prompt suggested using a linked list.
    // If strict adherence to using a linked list for *all* intermediate read operations was required,
    // the logic here would be:
    //   SinglyLinkedList tempList;
    //   while (std::getline(inFile, line)) { /* ... parse ... */ tempList.append(parsedItem); }
    //   items = tempList.toVector();
    // However, for a helper like `loadItemsFromCsv` whose primary goal is to return a vector,
    // loading directly into the vector is often more straightforward unless the LL step is explicitly mandated for this specific part.
    // The current design uses LinkedList in `listItemsCmd` (original version) before converting for sort.

    int csvLineNumber = 0;             // Counter for reporting errors on specific lines.
    while (std::getline(inFile, line)) // Read the file line by line.
    {
        csvLineNumber++;
        if (line.empty() || line.find_first_not_of(" \t\n\v\f\r") == std::string::npos)
        {
            // Skip lines that are completely empty or contain only whitespace characters.
            continue;
        }

        Item item;                    // Create an Item object to populate from the current line's data.
        if (parseCsvLine(line, item)) // Call our helper method to parse the line.
        {
            items.push_back(item); // If parsing is successful, add the populated item to our vector.
        }
        else // If parseCsvLine returned false (parsing failed for this line).
        {
            // Output a warning to standard error, indicating the problematic line.
            std::cerr << "Warning: Skipping malformed CSV line " << csvLineNumber << ": \"" << line << "\"" << std::endl;
        }
    }
    inFile.close(); // Close the input file.
    return items;   // Return the vector of successfully loaded items.
}

/**
 * @brief Writes a vector of Item objects to the CSV file, overwriting any existing content.
 * @param items The constant reference to a vector of Item objects to write.
 * @return true if writing was successful and the file was closed properly, false otherwise.
 *
 * Time Complexity: O(N * W_avg), where N is the number of items in the `items` vector,
 *                  and W_avg is the average time to write one item's string representation to the file.
 * Space Complexity: O(1) additional space beyond the input `items` vector. File stream buffers are internal.
 */
bool InventoryManager::writeItemsToCsv(const std::vector<Item> &items) const
{
    // Open the CSV file for writing.
    // std::ofstream by default opens in truncation mode (std::ios::trunc),
    // meaning if the file exists, its content is discarded before new data is written.
    // If the file doesn't exist, it's created.
    std::ofstream outFile(csvFilename);
    if (!outFile.is_open()) // If the file couldn't be opened for writing.
    {
        std::cerr << "CRITICAL ERROR: Could not open file '" << csvFilename << "' for writing. Data may not be saved." << std::endl;
        return false;
    }

    // Iterate through the vector of items provided.
    for (const auto &item : items) // Range-based for loop. `item` is a const reference to each Item object.
    {
        // Write each item's data to the file in CSV format: id,name,quantity,date
        outFile << item.id << ","
                << item.name << ","
                << item.quantity << ","
                << item.registrationDate << std::endl; // std::endl adds a newline and flushes the output buffer.
    }
    outFile.close(); // Close the output file. This is crucial to ensure all data is written to disk.

    // After closing, check if any error flags were set on the stream during operations.
    if (outFile.fail())
    {
        std::cerr << "CRITICAL ERROR: An error occurred while writing to '" << csvFilename << "'. File may be corrupt or incomplete." << std::endl;
        return false; // Indicate failure.
    }
    return true; // Writing was successful.
}

/**
 * @brief Displays a collection of items to the console, formatted with a given title.
 * @param items A constant reference to a vector of Item objects to display.
 * @param title A std::string title to print before the items list.
 *
 * Time Complexity: O(N), where N is the number of items in the `items` vector.
 *                  The loop iterates through each item once to print its details.
 * Space Complexity: O(1) additional space (excluding the console output buffer itself).
 */
void InventoryManager::displayItems(const std::vector<Item> &items, const std::string &title) const
{
    // Print a formatted header with the provided title.
    std::cout << "--------------------------------------------------------------------" << std::endl;
    std::cout << "                        " << title << "                              " << std::endl;
    std::cout << "--------------------------------------------------------------------" << std::endl;

    if (items.empty()) // If the vector of items is empty.
    {
        std::cout << "No items to display." << std::endl;
    }
    else // If there are items to display.
    {
        // Iterate through the items and print their details in a structured format.
        for (const auto &item : items) // `item` is a const reference to each Item.
        {
            std::cout << "Item ID:" << item.id
                      << "\tItem Name:" << item.name // \t (tab character) for basic column alignment.
                      << "\tQuantity:" << item.quantity
                      << "\tReg Date:" << item.registrationDate << std::endl;
        }
    }
    // Print a footer line.
    std::cout << "--------------------------------------------------------------------" << std::endl;
}

// ============== Public Method Implementations ==============
// These are the methods that form the public interface of the InventoryManager class.
// They are typically called from `main.cpp` in response to user commands.

/**
 * @brief Constructor for the InventoryManager class.
 *        Currently, it performs minimal setup. The `csvFilename` member is initialized
 *        via its in-class initializer in `InventoryManager.h`.
 */
InventoryManager::InventoryManager()
{
    // This constructor is called when an InventoryManager object is created.
    // e.g., `InventoryManager manager;` in main.cpp.
    // Could be used for one-time setup tasks, such as:
    // - Ensuring the CSV data file exists (and creating it with headers if it's the very first run).
    //   (Current implementation in ParkingManager does this, could be added here too).
    // - Loading initial configuration settings from a separate config file.
    // For this inventory system, it's kept simple.
}

/**
 * @brief Main execution loop of the inventory management system.
 *        It continuously prompts the user for commands, reads their input,
 *        parses the command and arguments, and dispatches to the appropriate handler methods.
 *
 * Time Complexity: The loop runs indefinitely until the "exit" command is issued.
 *                  Each iteration's complexity depends on the command executed.
 *                  Reading and parsing user input is typically O(L_input), where L_input is the
 *                  length of the user's input line.
 * Space Complexity: O(L_input_max) for storing the `line` buffer, `command_token`, and `args` vector
 *                   (where L_input_max is the maximum expected length of a command line).
 */
void InventoryManager::run()
{
    std::string line; // Buffer to store the full line of user input.
    // Display a welcome message and a hint to the user.
    std::cout << "Inventory Management System" << std::endl;
    std::cout << "Enter 'help' for commands." << std::endl;

    while (true) // This creates an infinite loop that only terminates on "exit" command or EOF.
    {
        std::cout << "\ninventory> "; // Print the command prompt. Added newline for better spacing.

        // Read an entire line of input from the user (standard input, std::cin).
        if (!std::getline(std::cin, line))
        {
            // `std::getline` returns false on failure (e.g., EOF or a critical I/O error).
            if (std::cin.eof()) // Check if failure was due to End-Of-File (e.g., Ctrl+D on Linux, Ctrl+Z then Enter on Windows).
            {
                std::cout << "\nExiting (EOF reached)." << std::endl; // Added newline
                break;                                                // Exit the `while(true)` loop, which will end the `run` method and the program.
            }
            // If it's another type of I/O error on std::cin:
            std::cerr << "Warning: Input stream error. Attempting to recover." << std::endl;
            std::cin.clear(); // Clear error flags (like failbit, badbit) on std::cin.
            // Discard the rest of the potentially problematic input buffer up to the next newline character.
            // `std::numeric_limits<std::streamsize>::max()` gets the maximum number of characters
            // that can be held in a stream buffer, effectively meaning "discard as many characters as possible".
            // From <limits> header.
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            continue; // Go to the next iteration of the loop (re-prompt the user).
        }

        // If the user just pressed Enter (resulting in an empty line), skip processing.
        if (line.empty() || line.find_first_not_of(" \t") == std::string::npos)
        {
            // Also skip lines containing only whitespace
            continue;
        }

        // Use a std::stringstream to parse the command and its arguments from the input line.
        // This allows us to easily extract words (tokens) separated by whitespace.
        std::stringstream ss(line);
        std::string command_token; // To store the first word from the line, which is the command.
        ss >> command_token;       // Extract the first whitespace-separated word into `command_token`.
                                   // This uses the overloaded stream extraction operator `>>`.

        // Convert the extracted command to lowercase to make command processing case-insensitive.
        // (e.g., "ITEMADD", "itemadd", "ItemAdd" will all be treated as "itemadd").
        command_token = toLowerCase(command_token);

        // Store the remaining words on the line as arguments for the command.
        std::vector<std::string> args; // Vector to hold command arguments.
        std::string arg;               // Temporary string to hold each argument as it's extracted.
        while (ss >> arg)              // While there are more words to extract from the stringstream `ss`.
        {
            args.push_back(arg); // Add the extracted argument `arg` to the end of the `args` vector.
        }

        // Command Dispatching:
        // Based on the value of `command_token`, call the appropriate handler method.
        if (command_token == "itemadd")
        {
            addItemCmd(args);
        }
        else if (command_token == "itemslist")
        {
            listItemsCmd(); // Handles the basic "itemslist" command (sorts by name by default).
        }
        else if (command_token == "itemslistby")
        {
            listItemsByCmd(args); // Handles "itemslistby <field> [order]" for custom sorting.
        }
        else if (command_token == "itemfind")
        {
            findItemCmd(args); // Acts as a dispatcher for "itemfind id <...>" or "itemfind name <...>".
        }
        else if (command_token == "itemdelete")
        {
            deleteItemCmd(args);
        }
        else if (command_token == "help")
        {
            displayHelpCmd();
        }
        else if (command_token == "exit")
        {
            std::cout << "Exiting program. Goodbye!" << std::endl;
            break; // Exit the `while(true)` loop, which terminates the `run` method and the program.
        }
        else // If the command_token doesn't match any of the known commands.
        {
            std::cout << "Error: Unknown command '" << command_token << "'. Type 'help' for available commands." << std::endl;
        }
    } // End of the main command loop (`while(true)`).
}

/**
 * @brief Handles the "itemadd" command. Generates a new unique ID and adds the item to the inventory CSV.
 *        Syntax: itemadd <name_part1> [...name_parts] <quantity> <registration_date (YYYY-MM-DD)>
 *        The item ID is auto-generated and is not provided by the user.
 * @param args A constant reference to a vector of strings containing the arguments provided by the user
 *             after the "itemadd" command itself (i.e., name parts, quantity, date).
 *
 * Time Complexity: O(G + V + W_append), where G is the time for `getNextAvailableId()` (which reads the entire CSV),
 *                  V is the time for `isValidDate()` (effectively constant time), and W_append is the time to
 *                  append one line to the CSV (proportional to the length of the item's string representation).
 *                  Dominated by `getNextAvailableId()` if the CSV is large.
 * Space Complexity: O(L_name_max) for storing `itemName` and processing arguments.
 *                   `getNextAvailableId` also has its own space complexity for line buffers.
 */
void InventoryManager::addItemCmd(const std::vector<std::string> &args)
{
    // The `itemadd` command now expects at least 3 arguments from the user:
    // 1. First part of the item name
    // 2. Quantity (as a string, will be converted to int)
    // 3. Registration date (as a string, in YYYY-MM-DD format)
    // The item name can span multiple arguments if it contains spaces.
    if (args.size() < 3) // e.g., "name qty date" requires 3 arguments.
    {
        std::cout << "Error: Insufficient arguments for itemadd." << std::endl;
        std::cout << "Usage: itemadd <name_part1> [...name_parts] <quantity> <registration_date (YYYY-MM-DD)>" << std::endl;
        return; // Exit the function; do not proceed with adding the item.
    }

    // --- Auto-generate the new Item ID ---
    int newIdInt = getNextAvailableId();           // Get the next available integer ID by scanning existing IDs.
    std::string itemId = std::to_string(newIdInt); // Convert the integer ID to a std::string for storage and display.
    // Since IDs are auto-generated, the previous `itemIdExists` check is no longer necessary.

    // --- Parse Item Name ---
    // The item name starts with the first argument provided by the user (args[0]).
    std::string itemName = args[0];
    // If the item name contains spaces, it would have been split into multiple arguments by the initial parsing.
    // This loop reconstructs the full item name. The name parts are from args[0] up to args[args.size() - 3].
    // The last two arguments are reserved for quantity and date.
    // Example: User types "itemadd Big Red Apple 10 2024-01-01"
    // `args` vector: ["Big", "Red", "Apple", "10", "2024-01-01"] (args.size() is 5)
    // `itemName` initially "Big" (args[0])
    // Loop for `i` from 1 up to (but not including) `args.size() - 2` (which is 5 - 2 = 3):
    //   i=1: `itemName` becomes "Big" + " " + args[1]("Red")  => "Big Red"
    //   i=2: `itemName` becomes "Big Red" + " " + args[2]("Apple") => "Big Red Apple"
    // Loop terminates.
    for (size_t i = 1; i < args.size() - 2; ++i) // Loop starts from the *second* argument (index 1) for additional name parts.
    {
        itemName += " " + args[i]; // Append a space and the current name part.
    }

    // --- Parse Quantity and Date ---
    // Quantity is the second-to-last argument. Date is the last argument.
    // `args.size() - 2` gives the index of the quantity string.
    // `args.size() - 1` gives the index of the date string.
    const std::string &quantityStr = args[args.size() - 2];
    const std::string regDate = args[args.size() - 1];
    int quantity; // Variable to store the integer value of the quantity.

    // Validate the date string format (YYYY-MM-DD) and its logical correctness.
    if (!isValidDate(regDate))
    {
        std::cout << "Error: Invalid date '" << regDate << "'. Please use YYYY-MM-DD format and ensure it's a valid date." << std::endl;
        return; // Exit if date is invalid.
    }

    // Convert the quantity string to an integer and perform basic validation.
    try
    {
        quantity = std::stoi(quantityStr); // `std::stoi` can throw std::invalid_argument or std::out_of_range.
        if (quantity < 0)                  // Business rule: quantity cannot be negative.
        {
            std::cout << "Error: Quantity cannot be negative." << std::endl;
            return; // Exit if quantity is negative.
        }
    }
    catch (const std::exception &e) // Catches both std::invalid_argument and std::out_of_range.
    {
        std::cout << "Error: Invalid quantity format ('" << quantityStr << "'). Quantity must be a whole number." << std::endl;
        // std::cout << "       Details: " << e.what() << std::endl; // For debugging the specific exception
        return; // Exit if quantity parsing fails.
    }

    // --- Write the New Item to the CSV File ---
    // Open the CSV file in append mode (std::ios::app).
    // If the file doesn't exist, it will be created by the ofstream constructor.
    std::ofstream outFile(csvFilename, std::ios::app);
    if (!outFile.is_open()) // Check if opening the file failed (e.g., permissions issue).
    {
        std::cerr << "CRITICAL ERROR: Could not open file '" << csvFilename << "' for writing. Item not added." << std::endl;
        return;
    }

    // Write the new item's data to the file in CSV format.
    outFile << itemId << "," << itemName << "," << quantity << "," << regDate << std::endl;
    outFile.close(); // Close the file to ensure data is flushed to disk.

    // Check for errors after closing the file stream
    if (outFile.fail())
    {
        std::cerr << "CRITICAL ERROR: Failed to write item to '" << csvFilename << "' after opening. Data may not be saved." << std::endl;
        // Item might be partially written or not at all.
    }
    else
    {
        std::cout << "Item added successfully: " << itemName << " (ID: " << itemId << ")" << std::endl;
    }
}

/**
 * @brief Handles the basic "itemslist" command. Lists all items from the inventory,
 *        sorted by item name in ascending, case-insensitive order by default.
 *
 * Time Complexity: O(L_csv + N log N), where L_csv is the time to load items from CSV
 *                  (see `loadItemsFromCsv`), and N log N is for `std::sort` on N items.
 *                  Displaying items is O(N). The dominant part is usually loading or sorting.
 * Space Complexity: O(N * S_item) for storing the `items` vector in memory, where S_item is
 *                   the average size of an Item object.
 */
void InventoryManager::listItemsCmd()
{
    std::vector<Item> items = loadItemsFromCsv(); // Load all items from the CSV file.
    if (items.empty())                            // If no items were loaded (e.g., file is empty, not found, or all lines were malformed).
    {
        std::cout << "Inventory is empty or file not found." << std::endl;
        return; // Nothing to list.
    }

    // Default sort for the "itemslist" command: by item name, ascending, case-insensitive.
    // `std::sort` takes iterators to the beginning and end of the range to sort,
    // and an optional custom comparison function (lambda in this case).
    // The lambda captures `this` to be able to call the `toLowerCase` member function.
    std::sort(items.begin(), items.end(), [this](const Item &a, const Item &b)
              {
        // Compare the lowercase versions of the names for case-insensitive sorting.
        return toLowerCase(a.name) < toLowerCase(b.name); });

    displayItems(items, "Inventory List (Sorted by Name Asc)"); // Display the sorted items.
}

/**
 * @brief Handles the "itemslistby" command. Lists items sorted by a specified field and order.
 *        Syntax: itemslistby <field> [asc|desc]
 *        Valid fields: id, name, quantity, date. Default order is ascending.
 * @param args User arguments: args[0] is the field to sort by, args[1] (optional) is the order ("desc").
 *
 * Time Complexity: Similar to `listItemsCmd`, O(L_csv + N log N), dominated by loading or sorting.
 * Space Complexity: O(N * S_item) for the `items` vector.
 */
void InventoryManager::listItemsByCmd(const std::vector<std::string> &args)
{
    if (args.empty()) // The user must specify a field to sort by.
    {
        std::cout << "Error: Missing sort criteria for itemslistby." << std::endl;
        std::cout << "Usage: itemslistby <field> [asc|desc]" << std::endl;
        std::cout << "Fields: id, name, quantity, date" << std::endl;
        return;
    }

    std::vector<Item> items = loadItemsFromCsv(); // Load all items.
    if (items.empty())
    {
        std::cout << "Inventory is empty or file not found." << std::endl;
        return;
    }

    std::string sortByField = toLowerCase(args[0]); // Get the sort field from user input (case-insensitive).
    bool ascending = true;                          // Default sort order is ascending.
    // Check if a second argument "desc" was provided to request descending order.
    if (args.size() > 1 && toLowerCase(args[1]) == "desc")
    {
        ascending = false;
    }

    std::string sortTitle = "Inventory List (Sorted by "; // Base for the display title.

    // Select the appropriate comparator lambda function for `std::sort` based on `sortByField`.
    if (sortByField == "id")
    {
        sortTitle += "ID";
        std::sort(items.begin(), items.end(), [](const Item &a, const Item &b)
                  {
            // IDs are currently auto-generated integers stored as strings.
            // For natural numeric sort if IDs could be like "1", "2", "10":
            //   try { return std::stoi(a.id) < std::stoi(b.id); } catch(...) { /* handle non-int IDs */ return a.id < b.id; }
            // For simple lexicographical sort (works if IDs have same padding or are purely sequential strings):
            return a.id < b.id; });
    }
    else if (sortByField == "name")
    {
        sortTitle += "Name";
        // Capture `this` to allow calling the `toLowerCase` member function from within the lambda.
        std::sort(items.begin(), items.end(), [this](const Item &a, const Item &b)
                  {
                      return toLowerCase(a.name) < toLowerCase(b.name); // Case-insensitive name sort.
                  });
    }
    else if (sortByField == "quantity")
    {
        sortTitle += "Quantity";
        std::sort(items.begin(), items.end(), [](const Item &a, const Item &b)
                  {
                      return a.quantity < b.quantity; // Sort by integer quantity.
                  });
    }
    else if (sortByField == "date")
    {
        sortTitle += "Date";
        std::sort(items.begin(), items.end(), [](const Item &a, const Item &b)
                  {
            // Assuming YYYY-MM-DD format, direct string comparison works correctly for chronological sorting of dates.
            return a.registrationDate < b.registrationDate; });
    }
    else // If the user provided an invalid sort field.
    {
        std::cout << "Error: Invalid sort field '" << args[0] << "'." << std::endl;
        std::cout << "Valid fields: id, name, quantity, date." << std::endl;
        return;
    }

    // If descending order was requested, reverse the (now ascendingly sorted) vector.
    // `std::reverse` reverses the order of elements in a range. It's an O(N) operation.
    if (!ascending)
    {
        sortTitle += " Desc)";
        std::reverse(items.begin(), items.end());
    }
    else
    {
        sortTitle += " Asc)";
    }

    displayItems(items, sortTitle); // Display the items with the appropriately constructed title.
}

/**
 * @brief Acts as a dispatcher for "itemfind" sub-commands.
 *        Syntax: itemfind <id|name> <search_term>
 *        It determines whether to search by ID or by name based on the first argument.
 * @param args User arguments: args[0] is the criteria ("id" or "name"),
 *             args[1] (and potentially subsequent) is the search term.
 *
 * Time Complexity: Parsing arguments is O(L_input). The complexity of the subsequent
 *                  call to `findItemById` or `findItemsByName` will dominate.
 * Space Complexity: O(L_input_max) for storing `args` and `searchTerm`.
 */
void InventoryManager::findItemCmd(const std::vector<std::string> &args)
{
    // The command requires at least two arguments: the criteria (id/name) and the search term.
    if (args.size() < 2)
    {
        std::cout << "Error: Insufficient arguments for itemfind." << std::endl;
        std::cout << "Usage: itemfind <id|name> <search_term>" << std::endl;
        return;
    }

    std::string findBy = toLowerCase(args[0]); // Get the find criteria (e.g., "id" or "name"), case-insensitive.
    std::string searchTerm = args[1];          // Get the first part of the search term.

    // If finding by name, the search term might contain spaces and thus span multiple arguments.
    // This loop reconstructs the full search term if it was split.
    // Example: User types "itemfind name Red Apple"
    // `args` vector: ["name", "Red", "Apple"] (after "itemfind" is removed by `run`)
    // `findBy` = "name" (from args[0])
    // `searchTerm` initially "Red" (from args[1])
    // Loop for `i` from 2 up to `args.size()`:
    //   i=2: `searchTerm` becomes "Red" + " " + args[2]("Apple") => "Red Apple"
    if (findBy == "name" && args.size() > 2) // If criteria is "name" and there are more words for the search term.
    {
        for (size_t i = 2; i < args.size(); ++i)
        {
            searchTerm += " " + args[i]; // Append subsequent parts with a space.
        }
    }

    // Dispatch to the specific find function based on the `findBy` criteria.
    if (findBy == "id")
    {
        findItemById(searchTerm);
    }
    else if (findBy == "name")
    {
        findItemsByName(searchTerm);
    }
    else // If the user provided an invalid find criterion.
    {
        std::cout << "Error: Invalid find criteria '" << args[0] << "'. Use 'id' or 'name'." << std::endl;
    }
}

/**
 * @brief Finds and displays items by an exact match of their ID.
 * @param targetId The ID string to search for.
 *
 * Time Complexity: O(L_csv + N), where L_csv is the time to load items from the CSV,
 *                  and N is the number of items (for the linear scan). Dominated by `loadItemsFromCsv`.
 * Space Complexity: O(N * S_item) for storing the `items` vector and potentially `foundItems`
 *                   (though `foundItems` will be small if IDs are unique).
 */
void InventoryManager::findItemById(const std::string &targetId) const
{
    std::vector<Item> items = loadItemsFromCsv(); // Load all items.
    std::vector<Item> foundItems;                 // Vector to store items that match the target ID.

    // Linearly scan through the loaded items.
    for (const auto &item : items)
    {
        if (item.id == targetId) // If an item's ID exactly matches the target ID.
        {
            foundItems.push_back(item);
            // Since item IDs are now auto-generated and unique, we could theoretically `break;` here
            // after finding the first match. However, keeping the loop allows for the possibility
            // (however unlikely with auto-ID) of multiple items if the CSV were manually edited
            // or if a different ID scheme was used previously.
        }
    }
    displayItems(foundItems, "Search Results for ID: " + targetId); // Display any found items.
}

/**
 * @brief Finds and displays items whose names contain the given search term (case-insensitive).
 * @param searchTerm The string to search for as a substring within item names.
 *
 * Time Complexity: O(L_csv + N * M_name_avg), where L_csv is load time, N is the number of items,
 *                  and M_name_avg is the average item name length (due to `toLowerCase` and `std::string::find`).
 *                  Dominated by loading or the N * M_name_avg part if names are very long.
 * Space Complexity: O(N * S_item) for the `items` and `foundItems` vectors.
 */
void InventoryManager::findItemsByName(const std::string &searchTerm) const
{
    std::vector<Item> items = loadItemsFromCsv();          // Load all items.
    std::vector<Item> foundItems;                          // Vector to store matching items.
    std::string lowerSearchTerm = toLowerCase(searchTerm); // Convert search term to lowercase once for efficiency.

    // Linearly scan through the items.
    for (const auto &item : items)
    {
        // Convert the current item's name to lowercase for case-insensitive comparison.
        // `std::string::find(substring)` searches for `substring`.
        // It returns `std::string::npos` if the substring is not found.
        if (toLowerCase(item.name).find(lowerSearchTerm) != std::string::npos)
        {
            foundItems.push_back(item); // Add the item if its name contains the search term.
        }
    }
    displayItems(foundItems, "Search Results for Name containing: \"" + searchTerm + "\"");
}

/**
 * @brief Handles the "itemdelete" command. Removes an item by its ID from the inventory.
 *        This involves reading all items from the CSV, filtering out the one to delete,
 *        and then rewriting the entire CSV file with the remaining items.
 * @param args User arguments, where args[0] is the ID (as a string) of the item to delete.
 *
 * Time Complexity: O(L_csv_read + N + L_csv_write).
 *                  L_csv_read = time to load all items (see `loadItemsFromCsv`).
 *                  N = time to iterate through items to build the `remainingItems` vector.
 *                  L_csv_write = time to write all remaining items back to CSV (see `writeItemsToCsv`).
 *                  Dominated by file I/O operations.
 * Space Complexity: O(N * S_item) for storing `allItems` and `remainingItems` vectors in memory.
 */
void InventoryManager::deleteItemCmd(const std::vector<std::string> &args)
{
    if (args.empty()) // The user must provide an ID for the item to be deleted.
    {
        std::cout << "Error: Missing item_id for delete command." << std::endl;
        std::cout << "Usage: itemdelete <item_id>" << std::endl;
        return;
    }
    const std::string &idToDelete = args[0]; // The ID (as a string) of the item to be deleted.

    std::vector<Item> allItems = loadItemsFromCsv(); // Load all current items from the CSV.

    if (allItems.empty()) // Handles the case where the inventory CSV is empty or doesn't exist.
    {
        std::cout << "Inventory is empty. Nothing to delete." << std::endl;
        return;
    }

    std::vector<Item> remainingItems; // Vector to store items that are *not* deleted (i.e., to be kept).
    bool found = false;               // Flag to track if the item with `idToDelete` was actually found.

    // Iterate through all loaded items.
    for (const auto &item : allItems)
    {
        if (item.id == idToDelete) // If the current item's ID matches the ID to delete.
        {
            found = true; // Mark as found. This item will *not* be added to `remainingItems`.
            std::cout << "Info: Item found for deletion: Name: " << item.name << ", ID: " << item.id << std::endl;
        }
        else // If it's not the item to delete.
        {
            remainingItems.push_back(item); // Add it to the list of items to keep.
        }
    }

    if (!found) // If the item with the given ID was not found in the inventory.
    {
        std::cout << "Item with ID '" << idToDelete << "' not found. No item deleted." << std::endl;
        return;
    }

    // If the item was found (and thus excluded from `remainingItems`),
    // rewrite the CSV file with only the content of the `remainingItems` vector.
    // This effectively deletes the item from the persistent storage.
    if (writeItemsToCsv(remainingItems)) // `writeItemsToCsv` overwrites the old CSV file.
    {
        std::cout << "Item with ID '" << idToDelete << "' deleted successfully." << std::endl;
    }
    else // If writing the updated inventory back to the file failed.
    {
        // This is a critical error state. The in-memory representation (if any beyond this function)
        // might be out of sync with a potentially corrupted or non-updated file.
        std::cout << "CRITICAL ERROR: Failed to write updated inventory to file after attempting deletion of ID '"
                  << idToDelete << "'. The CSV file may be in an inconsistent state." << std::endl;
    }
}

/**
 * @brief Displays the help message with command syntaxes and examples to the user.
 *
 * Time Complexity: O(1) - It's just printing a fixed set of pre-defined strings.
 * Space Complexity: O(1) - No significant dynamic memory allocation.
 */
void InventoryManager::displayHelpCmd() const
{
    // Prints a formatted block of help text to std::cout.
    std::cout << "\n************************************************************************" << std::endl;
    std::cout << "*                           Commands Syntaxes                          *" << std::endl;
    std::cout << "************************************************************************" << std::endl;

    std::cout << "itemadd <name_part1> [...name_parts] <qty> <date (YYYY-MM-DD)>" << std::endl;
    std::cout << "  - Adds a new item. ID is auto-generated. Name can have spaces." << std::endl;
    std::cout << "    Example: itemadd Red Fuji Apple 150 2023-11-01" << std::endl;
    std::cout << std::endl;

    std::cout << "itemslist" << std::endl;
    std::cout << "  - Lists all items, sorted by name (case-insensitive, ascending)." << std::endl;
    std::cout << std::endl;

    std::cout << "itemslistby <field> [asc|desc]" << std::endl;
    std::cout << "  - Lists items sorted by <field> (id, name, quantity, date)." << std::endl;
    std::cout << "  - Default order is 'asc' (ascending). 'desc' for descending." << std::endl;
    std::cout << "    Example: itemslistby quantity desc" << std::endl;
    std::cout << "    Example: itemslistby date" << std::endl;
    std::cout << std::endl;

    std::cout << "itemfind id <item_id>" << std::endl;
    std::cout << "  - Finds and displays an item by its exact ID." << std::endl;
    std::cout << "    Example: itemfind id 1" << std::endl; // Example ID, actual IDs will be auto-generated.
    std::cout << std::endl;

    std::cout << "itemfind name <search_term>" << std::endl;
    std::cout << "  - Finds items where name contains <search_term> (case-insensitive)." << std::endl;
    std::cout << "    Example: itemfind name apple" << std::endl;
    std::cout << std::endl;

    std::cout << "itemdelete <item_id>" << std::endl;
    std::cout << "  - Deletes an item by its ID from the inventory." << std::endl;
    std::cout << "    Example: itemdelete 1" << std::endl; // Example ID.
    std::cout << std::endl;

    std::cout << "help" << std::endl;
    std::cout << "  - Displays this help message." << std::endl;
    std::cout << std::endl;

    std::cout << "exit" << std::endl;
    std::cout << "  - Exits the application." << std::endl;
    std::cout << "************************************************************************" << std::endl;
}