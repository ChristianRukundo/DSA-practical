// ParkingManager.cpp
#include "ParkingManager.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <limits>
#include <iomanip>
using namespace std;
std::string ParkingManager::toLowerCase(std::string s) const
{
    std::transform(s.begin(), s.end(), s.begin(),
                   [](unsigned char c)
                   { return std::tolower(c); });
    return s;
}

// Helper for robustly parsing arguments, handling quoted strings.
// Reads from a generic istream (could be std::cin or std::stringstream).
std::vector<std::string> ParkingManager::parseArguments(std::istream &stream) const
{
    std::vector<std::string> args;
    std::string token;
    // Temporarily change stream to not skip whitespace to correctly read leading spaces for quoted args
    // This is a bit tricky with direct istream modification; an alternative is to read the whole line
    // and then parse it. For simplicity with stringstream >>, we'll handle quotes manually after extraction.

    // For this iteration, we'll parse the line manually to handle quotes better.
    std::string line;
    // This function assumes `stream` is already positioned after the command token.
    // So, we read the rest of the line.
    std::getline(stream, line);

    std::stringstream line_ss(line);
    std::string current_arg;
    bool in_quote = false;

    char ch;
    while (line_ss.get(ch))
    {
        if (ch == '"')
        {
            in_quote = !in_quote;
            if (!in_quote && !current_arg.empty())
            { // End of a quoted argument
              // args.push_back(current_arg); // Pushed when next space or end of line
              // current_arg.clear();
            }
            else if (in_quote && !current_arg.empty())
            { // A quote inside an unquoted string, treat as literal.
                current_arg += ch;
            }
            // Skip adding the quote itself unless it's meant to be part of the arg
        }
        else if (std::isspace(ch) && !in_quote)
        {
            if (!current_arg.empty())
            {
                args.push_back(current_arg);
                current_arg.clear();
            }
        }
        else
        {
            current_arg += ch;
        }
    }
    if (!current_arg.empty())
    { // Add the last argument
        args.push_back(current_arg);
    }
    return args;
}

// Parses a line from a CSV file into a ParkingRecord object.
bool ParkingManager::parseRecordFromCsvLine(const std::string &line, ParkingRecord &record, ParkingStatus expectedStatus) const
{
    std::stringstream ss(line);
    std::string field;

    // Order in CSV for HISTORY: historyRecordId,vehicleNumber,entryTime,exitTime,durationMinutes,fee
    // Order in CSV for ACTIVE:  vehicleNumber,entryTime

    if (expectedStatus == ParkingStatus::EXITED)
    {
        if (!std::getline(ss, field, ','))
            return false; // historyRecordId
        try
        {
            record.historyRecordId = std::stoi(field);
        }
        catch (const std::exception &)
        {
            record.historyRecordId = 0; /* Or return false */
        }
    }
    else
    {
        record.historyRecordId = 0; // Active records don't use this from CSV directly
    }

    if (!std::getline(ss, record.vehicleNumber, ','))
        return false;
    if (!std::getline(ss, record.entryTime, ','))
        return false;

    record.status = expectedStatus;

    if (expectedStatus == ParkingStatus::EXITED)
    {
        if (!std::getline(ss, record.exitTime, ','))
            return false;
        if (!std::getline(ss, field, ','))
            return false; // durationMinutes
        try
        {
            record.durationMinutes = std::stoll(field);
        }
        catch (const std::exception &)
        {
            return false;
        }

        if (!std::getline(ss, field))
            return false; // fee
        try
        {
            record.fee = std::stod(field);
        }
        catch (const std::exception &)
        {
            return false;
        }
    }
    // No more fields to parse for ACTIVE from CSV in this design
    return true;
}

int ParkingManager::getNextHistoryRecordId() const
{
    std::vector<ParkingRecord> history = loadRecords(parkingHistoryFilename, ParkingStatus::EXITED);
    int maxId = 0;
    for (const auto &record : history)
    {
        if (record.historyRecordId > maxId)
        {
            maxId = record.historyRecordId;
        }
    }
    return maxId + 1;
}

std::vector<ParkingRecord> ParkingManager::loadRecords(const std::string &filename, ParkingStatus expectedStatus) const
{
    std::vector<ParkingRecord> records;
    std::ifstream inFile(filename);
    if (!inFile.is_open())
    {
        // If file doesn't exist, it's not an error for loading, just means no records.
        // Create the file if it's one of our main data files and it's missing?
        // For now, just return empty.
        // std::cout << "Debug: File " << filename << " not found or couldn't be opened for loading." << std::endl;
        return records;
    }

    std::string line;
    int lineNumber = 0;
    while (std::getline(inFile, line))
    {
        lineNumber++;
        if (line.empty() || line.find_first_not_of(" \t\n\v\f\r") == std::string::npos)
        { // Skip empty or whitespace-only lines
            continue;
        }
        ParkingRecord record;
        if (parseRecordFromCsvLine(line, record, expectedStatus))
        {
            records.push_back(record);
        }
        else
        {
            std::cerr << "Warning: Skipping malformed CSV line " << lineNumber << " in " << filename << "." << std::endl;
            // std::cerr << "         Line content: \"" << line << "\"" << std::endl; // For debugging
        }
    }
    inFile.close();
    return records;
}

bool ParkingManager::saveRecords(const std::string &filename, const std::vector<ParkingRecord> &records) const
{
    std::ofstream outFile(filename);
    if (!outFile.is_open())
    {
        std::cerr << "CRITICAL ERROR: Could not open file " << filename << " for writing. Data may be lost." << std::endl;
        return false;
    }
    outFile << std::fixed << std::setprecision(2);

    for (const auto &record : records)
    {
        if (record.status == ParkingStatus::EXITED)
        {                                             // History records have more fields
            outFile << record.historyRecordId << ","; // Save historyRecordId FIRST for history
        }
        outFile << record.vehicleNumber << ","
                << record.entryTime;
        if (record.status == ParkingStatus::EXITED)
        {
            outFile << "," << record.exitTime
                    << "," << record.durationMinutes
                    << "," << record.fee;
        }
        outFile << std::endl;
    }
    outFile.close();
    if (outFile.fail())
    { // Check if any write operations failed before closing
        std::cerr << "CRITICAL ERROR: Failed to write all data to " << filename << ". File may be corrupt." << std::endl;
        return false;
    }
    return true;
}

bool ParkingManager::appendRecord(const std::string &filename, const ParkingRecord &record, bool isHistoryRecord) const
{
    std::ofstream outFile(filename, std::ios::app);
    if (!outFile.is_open())
    {
        std::cerr << "CRITICAL ERROR: Could not open file " << filename << " for appending." << std::endl;
        return false;
    }
    outFile << std::fixed << std::setprecision(2);

    if (isHistoryRecord)
    {
        outFile << record.historyRecordId << ",";
    }
    outFile << record.vehicleNumber << ","
            << record.entryTime;
    if (isHistoryRecord)
    { // Only history records have these extra fields written
        outFile << "," << record.exitTime
                << "," << record.durationMinutes
                << "," << record.fee;
    }
    outFile << std::endl;

    outFile.close();
    if (outFile.fail())
    {
        std::cerr << "CRITICAL ERROR: Failed to append data to " << filename << "." << std::endl;
        return false;
    }
    return true;
}

ParkingRecord ParkingManager::createParkingRecordForEntry(const std::string &vehicleNumber, const std::string &entryTime) const
{
    ParkingRecord pr;
    pr.vehicleNumber = vehicleNumber;
    pr.entryTime = entryTime;
    pr.status = ParkingStatus::PARKED;
    pr.historyRecordId = 0; // Not applicable for active parking records yet
    return pr;
}

ParkingManager::ParkingManager()
{
    // Ensure files exist on startup, or create them (empty)
    // This helps avoid "file not found" if it's the first run and loadRecords expects them.
    std::ofstream activeFile(activeParkingFilename, std::ios::app); // Open in append, creates if not exist
    if (activeFile.is_open())
        activeFile.close();
    else
        std::cerr << "Warning: Could not ensure active parking file '" << activeParkingFilename << "' exists or is accessible." << std::endl;

    std::ofstream historyFile(parkingHistoryFilename, std::ios::app); // Open in append, creates if not exist
    if (historyFile.is_open())
        historyFile.close();
    else
        std::cerr << "Warning: Could not ensure parking history file '" << parkingHistoryFilename << "' exists or is accessible." << std::endl;
}

void ParkingManager::displayParkingRecords(const std::vector<ParkingRecord> &records, const std::string &title) const
{
    std::cout << "\n--- " << title << " ---" << std::endl;
    if (records.empty())
    {
        std::cout << "No records to display." << std::endl;
        std::cout << "-----------------------------------------" << std::endl;
        return;
    }

    bool isHistory = (title.find("History") != std::string::npos || title.find("Exited") != std::string::npos);

    if (isHistory)
    {
        std::cout << std::left
                  << std::setw(8) << "Hist.ID"
                  << std::setw(15) << "Vehicle No."
                  << std::setw(22) << "Entry Time"
                  << std::setw(22) << "Exit Time"
                  << std::setw(15) << "Duration(m)"
                  << std::setw(10) << "Fee ($)" << std::endl;
    }
    else
    { // Active
        std::cout << std::left
                  << std::setw(15) << "Vehicle No."
                  << std::setw(22) << "Entry Time" << std::endl;
    }
    std::cout << std::string(isHistory ? 92 : 37, '-') << std::endl; // Adjust line length

    for (const auto &record : records)
    {
        if (isHistory)
        {
            std::cout << std::left
                      << std::setw(8) << record.historyRecordId
                      << std::setw(15) << record.vehicleNumber
                      << std::setw(22) << record.entryTime
                      << std::setw(22) << record.exitTime
                      << std::setw(15) << record.durationMinutes
                      << std::fixed << std::setprecision(2) << std::setw(10) << record.fee << std::endl;
        }
        else
        { // Active
            std::cout << std::left
                      << std::setw(15) << record.vehicleNumber
                      << std::setw(22) << record.entryTime << std::endl;
        }
    }
    std::cout << std::string(isHistory ? 92 : 37, '-') << std::endl;
}

// --- Public Command Handlers ---

void ParkingManager::parkVehicleCmd(const std::vector<std::string> &args)
{
    if (args.empty() || args[0].empty())
    {
        std::cout << "Error: Vehicle number missing or empty." << std::endl;
        std::cout << "Usage: park <vehicle_number>" << std::endl;
        return;
    }
    // Assuming vehicle number is the first argument and can't contain spaces for simplicity with current parser
    // If vehicle numbers could have spaces, args[0] would need to be combined with subsequent args or quoted.
    std::string vehicleNumber = args[0];

    std::vector<ParkingRecord> activeParking = loadRecords(activeParkingFilename, ParkingStatus::PARKED);
    for (const auto &record : activeParking)
    {
        if (record.vehicleNumber == vehicleNumber)
        {
            std::cout << "Error: Vehicle " << vehicleNumber << " is already parked (Entry: " << record.entryTime << ")." << std::endl;
            return;
        }
    }

    std::string entryTime = DateTimeUtil::getCurrentTimestamp();
    ParkingRecord newRecord = createParkingRecordForEntry(vehicleNumber, entryTime);

    if (appendRecord(activeParkingFilename, newRecord, false))
    {
        std::cout << "Vehicle " << vehicleNumber << " parked successfully at " << entryTime << "." << std::endl;
    }
    else
    {
        std::cout << "Error: Failed to record parking for vehicle " << vehicleNumber << "." << std::endl;
    }
}

void ParkingManager::exitVehicleCmd(const std::vector<std::string> &args)
{
    if (args.empty() || args[0].empty())
    {
        std::cout << "Error: Vehicle number missing or empty." << std::endl;
        std::cout << "Usage: exit <vehicle_number>" << std::endl;
        return;
    }
    std::string vehicleNumberToExit = args[0];

    std::vector<ParkingRecord> activeParking = loadRecords(activeParkingFilename, ParkingStatus::PARKED);
    ParkingRecord exitingRecord;
    bool found = false;

    std::vector<ParkingRecord> updatedActiveParking; // To store non-exiting vehicles

    for (const auto &record : activeParking)
    {
        if (record.vehicleNumber == vehicleNumberToExit)
        {
            exitingRecord = record;
            found = true;
            // Don't add this to updatedActiveParking
        }
        else
        {
            updatedActiveParking.push_back(record);
        }
    }

    if (!found)
    {
        std::cout << "Error: Vehicle " << vehicleNumberToExit << " not found in active parking." << std::endl;
        return;
    }

    exitingRecord.exitTime = DateTimeUtil::getCurrentTimestamp();
    exitingRecord.durationMinutes = DateTimeUtil::calculateDurationMinutes(exitingRecord.entryTime, exitingRecord.exitTime);

    if (exitingRecord.durationMinutes == -1)
    { // -1 from calculateDuration indicates parse error
        std::cerr << "Error: Could not parse timestamps to calculate duration for vehicle " << vehicleNumberToExit << "." << std::endl;
        std::cerr << "       Entry: " << exitingRecord.entryTime << ", Tentative Exit: " << exitingRecord.exitTime << std::endl;
        std::cerr << "       Vehicle remains in active parking. Please check data or try again." << std::endl;
        return;
    }
    else if (exitingRecord.durationMinutes == -2)
    { // -2 indicates end time before start time
        std::cerr << "Error: Exit time is before entry time for vehicle " << vehicleNumberToExit << "." << std::endl;
        std::cerr << "       Entry: " << exitingRecord.entryTime << ", Exit: " << exitingRecord.exitTime << std::endl;
        std::cerr << "       Vehicle remains in active parking." << std::endl;
        return;
    }

    double hours = static_cast<double>(exitingRecord.durationMinutes) / 60.0;
    if (exitingRecord.durationMinutes > 0 && hours < 1.0)
    {
        exitingRecord.fee = HOURLY_RATE; // Minimum 1 hour charge if actually parked
    }
    else
    {
        exitingRecord.fee = hours * HOURLY_RATE;
    }
    if (exitingRecord.fee < 0)
        exitingRecord.fee = 0; // Sanity check

    exitingRecord.status = ParkingStatus::EXITED;
    exitingRecord.historyRecordId = getNextHistoryRecordId(); // Assign unique history ID

    if (!saveRecords(activeParkingFilename, updatedActiveParking))
    {
        std::cerr << "CRITICAL ERROR: Failed to update active parking file for vehicle " << vehicleNumberToExit
                  << ". Vehicle may not have been removed from active list." << std::endl;
        // Consider rolling back or alerting admin. For now, we proceed to add to history.
    }

    if (appendRecord(parkingHistoryFilename, exitingRecord, true))
    {
        std::cout << "Vehicle " << vehicleNumberToExit << " exited at " << exitingRecord.exitTime << "." << std::endl;
        std::cout << "  History Record ID: " << exitingRecord.historyRecordId << std::endl;
        std::cout << "  Duration: " << exitingRecord.durationMinutes << " minutes." << std::endl;
        std::cout << "  Fee: $" << std::fixed << std::setprecision(2) << exitingRecord.fee << std::endl;
    }
    else
    {
        std::cerr << "CRITICAL ERROR: Failed to record exit in parking history for vehicle " << vehicleNumberToExit << "." << std::endl;
        std::cerr << "  Please manually verify and add record: " << exitingRecord << std::endl;
    }
}

void ParkingManager::listActiveCmd(const std::vector<std::string> &args)
{
    std::vector<ParkingRecord> activeRecords = loadRecords(activeParkingFilename, ParkingStatus::PARKED);
    std::sort(activeRecords.begin(), activeRecords.end(), [](const ParkingRecord &a, const ParkingRecord &b)
              {
        // Compare time_points for accurate sorting
        return DateTimeUtil::parseTimestamp(a.entryTime) < DateTimeUtil::parseTimestamp(b.entryTime); });
    displayParkingRecords(activeRecords, "Currently Parked Vehicles");
}

void ParkingManager::listHistoryCmd(const std::vector<std::string> &args)
{
    std::vector<ParkingRecord> historyRecords = loadRecords(parkingHistoryFilename, ParkingStatus::EXITED);
    std::string title = "Parking History (All)";

    if (!args.empty() && toLowerCase(args[0]) == "date")
    {
        if (args.size() < 2 || args[1].length() != 10)
        { // YYYY-MM-DD
            std::cout << "Error: Date missing or invalid format for history filter." << std::endl;
            std::cout << "Usage: list history date <YYYY-MM-DD>" << std::endl;
            return;
        }
        std::string filterDate = args[1];

        std::vector<ParkingRecord> filteredHistory;
        for (const auto &record : historyRecords)
        {
            // Check if record.entryTime or record.exitTime starts with filterDate
            // Using string::rfind(substr, 0) == 0 is equivalent to string::starts_with in C++20
            if ((!record.entryTime.empty() && record.entryTime.rfind(filterDate, 0) == 0) ||
                (!record.exitTime.empty() && record.exitTime.rfind(filterDate, 0) == 0))
            {
                filteredHistory.push_back(record);
            }
        }
        historyRecords = filteredHistory;
        title = "Parking History for Date: " + filterDate;
    }

    std::sort(historyRecords.begin(), historyRecords.end(), [](const ParkingRecord &a, const ParkingRecord &b)
              { return DateTimeUtil::parseTimestamp(a.exitTime) > DateTimeUtil::parseTimestamp(b.exitTime); });

    displayParkingRecords(historyRecords, title);
}

void ParkingManager::searchActiveCmd(const std::vector<std::string> &args)
{
    if (args.empty() || args[0].empty())
    {
        std::cout << "Error: Vehicle number missing for search." << std::endl;
        std::cout << "Usage: search active <vehicle_number>" << std::endl;
        return;
    }
    std::string vehicleNumber = args[0];
    std::vector<ParkingRecord> activeRecords = loadRecords(activeParkingFilename, ParkingStatus::PARKED);
    std::vector<ParkingRecord> foundRecords;

    for (const auto &record : activeRecords)
    {
        // Case-insensitive vehicle number search might be better
        if (toLowerCase(record.vehicleNumber) == toLowerCase(vehicleNumber))
        {
            foundRecords.push_back(record);
        }
    }
    displayParkingRecords(foundRecords, "Search Results in Active Parking for: " + vehicleNumber);
}

void ParkingManager::searchHistoryCmd(const std::vector<std::string> &args)
{
    if (args.empty() || args[0].empty())
    {
        std::cout << "Error: Vehicle number missing for search." << std::endl;
        std::cout << "Usage: search history <vehicle_number>" << std::endl;
        return;
    }
    std::string vehicleNumber = args[0];
    std::vector<ParkingRecord> historyRecords = loadRecords(parkingHistoryFilename, ParkingStatus::EXITED);
    std::vector<ParkingRecord> foundRecords;

    for (const auto &record : historyRecords)
    {
        if (toLowerCase(record.vehicleNumber) == toLowerCase(vehicleNumber))
        {
            foundRecords.push_back(record);
        }
    }
    std::sort(foundRecords.begin(), foundRecords.end(), [](const ParkingRecord &a, const ParkingRecord &b)
              { return DateTimeUtil::parseTimestamp(a.exitTime) > DateTimeUtil::parseTimestamp(b.exitTime); });
    displayParkingRecords(foundRecords, "Search Results in Parking History for: " + vehicleNumber);
}

void ParkingManager::deleteHistoryRecordCmd(const std::vector<std::string> &args)
{
    if (args.empty())
    {
        std::cout << "Error: History Record ID missing." << std::endl;
        std::cout << "Usage: delete history <history_record_id>" << std::endl;
        return;
    }

    int idToDelete;
    try
    {
        idToDelete = std::stoi(args[0]);
        if (idToDelete <= 0)
            throw std::invalid_argument("ID must be positive");
    }
    catch (const std::exception &e)
    {
        std::cout << "Error: Invalid History Record ID '" << args[0] << "'. Must be a positive number." << std::endl;
        // std::cout << "       Details: " << e.what() << std::endl; // For debugging
        return;
    }

    std::vector<ParkingRecord> historyRecords = loadRecords(parkingHistoryFilename, ParkingStatus::EXITED);
    if (historyRecords.empty())
    {
        std::cout << "Parking history is empty. Nothing to delete." << std::endl;
        return;
    }

    std::vector<ParkingRecord> updatedHistory;
    bool foundAndDeleted = false;

    for (const auto &record : historyRecords)
    {
        if (record.historyRecordId == idToDelete)
        {
            foundAndDeleted = true;
            std::cout << "Confirm: Deleting record -> " << record << std::endl; // Show what's being deleted
        }
        else
        {
            updatedHistory.push_back(record);
        }
    }

    if (foundAndDeleted)
    {
        // Add a confirmation step here if desired
        // std::string confirmation;
        // std::cout << "Are you sure you want to delete this record? (yes/no): ";
        // std::getline(std::cin, confirmation);
        // if (toLowerCase(confirmation) != "yes") {
        //     std::cout << "Deletion cancelled." << std::endl;
        //     return;
        // }

        if (saveRecords(parkingHistoryFilename, updatedHistory))
        {
            std::cout << "History Record ID " << idToDelete << " deleted successfully." << std::endl;
        }
        else
        {
            std::cerr << "CRITICAL ERROR: Failed to save updated parking history after deleting record ID " << idToDelete << "." << std::endl;
        }
    }
    else
    {
        std::cout << "History Record ID " << idToDelete << " not found." << std::endl;
    }
}

void ParkingManager::displayHelpCmd() const
{
    std::cout << "\n================ Parking Management System Commands ================" << std::endl;
    std::cout << std::left; // Ensure text is left-aligned for help
    std::cout << std::setw(50) << "park <vehicle_number>" << "- Records vehicle entry." << std::endl;
    std::cout << std::setw(50) << "  Example: park ABC-123" << std::endl;
    std::cout << std::setw(50) << "exit <vehicle_number>" << "- Records vehicle exit and calculates fee." << std::endl;
    std::cout << std::setw(50) << "  Example: exit ABC-123" << std::endl;
    std::cout << std::setw(50) << "list active" << "- Lists currently parked vehicles." << std::endl;
    std::cout << std::setw(50) << "list history [all | date <YYYY-MM-DD>]" << "- Lists parking history (all or by date)." << std::endl;
    std::cout << std::setw(50) << "  Example: list history" << std::endl;
    std::cout << std::setw(50) << "  Example: list history date 2023-10-27" << std::endl;
    std::cout << std::setw(50) << "search active <vehicle_number>" << "- Searches active parking." << std::endl;
    std::cout << std::setw(50) << "search history <vehicle_number>" << "- Searches parking history." << std::endl;
    std::cout << std::setw(50) << "delete history <history_record_id>" << "- Deletes a record from history by its ID." << std::endl;
    std::cout << std::setw(50) << "  Example: delete history 123" << std::endl;
    std::cout << std::setw(50) << "help" << "- Displays this help message." << std::endl;
    std::cout << std::setw(50) << "exit" << "- Exits the application." << std::endl;
    std::cout << "====================================================================" << std::endl;
}

void ParkingManager::run()
{
    std::string full_line_input;
    std::cout << "Parking Management System Initialized." << std::endl;
    displayHelpCmd();

    while (true)
    {
        std::cout << "\nparking> ";
        if (!std::getline(std::cin, full_line_input))
        {
            if (std::cin.eof())
            {
                std::cout << "Exiting (EOF)." << std::endl;
                break;
            }
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cerr << "Warning: Input error. Please try again." << std::endl;
            continue;
        }

        if (full_line_input.empty() || full_line_input.find_first_not_of(" \t") == std::string::npos)
        {
            continue; // Skip empty or whitespace-only input
        }

        std::stringstream command_stream(full_line_input);
        std::string command_token;
        command_stream >> command_token; // Extract the first word as the command
        command_token = toLowerCase(command_token);

        // Use our improved argument parser for the rest of the line
        std::vector<std::string> args = parseArguments(command_stream);

        if (command_token == "park")
        {
            parkVehicleCmd(args);
        }
        else if (command_token == "exit")
        {
            exitVehicleCmd(args);
        }
        else if (command_token == "list")
        {
            if (args.empty())
            {
                std::cout << "Usage: list <active|history> [options]" << std::endl;
                continue;
            }
            std::string listType = toLowerCase(args[0]);
            std::vector<std::string> subArgs(args.begin() + 1, args.end());
            if (listType == "active")
                listActiveCmd(subArgs);
            else if (listType == "history")
                listHistoryCmd(subArgs);
            else
                std::cout << "Error: Unknown list type. Use 'active' or 'history'." << std::endl;
        }
        else if (command_token == "search")
        {
            if (args.size() < 2)
            {
                std::cout << "Usage: search <active|history> <vehicle_number>" << std::endl;
                continue;
            }
            std::string searchType = toLowerCase(args[0]);
            std::vector<std::string> subArgs(args.begin() + 1, args.end());
            if (searchType == "active")
                searchActiveCmd(subArgs);
            else if (searchType == "history")
                searchHistoryCmd(subArgs);
            else
                std::cout << "Error: Unknown search type. Use 'active' or 'history'." << std::endl;
        }
        else if (command_token == "delete")
        {
            if (args.empty())
            {
                std::cout << "Usage: delete history <history_record_id>" << std::endl;
                continue;
            }
            std::string deleteType = toLowerCase(args[0]);
            std::vector<std::string> subArgs(args.begin() + 1, args.end());
            if (deleteType == "history")
                deleteHistoryRecordCmd(subArgs);
            else
                std::cout << "Error: Can only delete from 'history'." << std::endl;
        }
        else if (command_token == "help")
        {
            displayHelpCmd();
        }
        else if (command_token == "exit")
        {
            std::cout << "Exiting system. Goodbye!" << std::endl;
            break;
        }
        else
        {
            std::cout << "Error: Unknown command '" << command_token << "'. Type 'help' for commands." << std::endl;
        }
    }
}