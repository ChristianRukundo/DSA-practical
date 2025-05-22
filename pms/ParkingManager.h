// ParkingManager.h
#pragma once

#include "ParkingRecord.h"
#include "DateTimeUtil.h"
#include <string>
#include <vector>

class ParkingManager
{
private:
    const std::string activeParkingFilename = "active_parking.csv";
    const std::string parkingHistoryFilename = "parking_history.csv";
    const double HOURLY_RATE = 2.0;

    // --- Helper Functions for File I/O ---
    std::vector<ParkingRecord> loadRecords(const std::string &filename, ParkingStatus expectedStatus) const;
    bool saveRecords(const std::string &filename, const std::vector<ParkingRecord> &records) const;
    bool appendRecord(const std::string &filename, const ParkingRecord &record, bool isHistoryRecord) const; // Added flag

    // --- Internal Logic Helpers ---
    ParkingRecord createParkingRecordForEntry(const std::string &vehicleNumber, const std::string &entryTime) const;
    bool parseRecordFromCsvLine(const std::string &line, ParkingRecord &record, ParkingStatus expectedStatus) const;
    std::string toLowerCase(std::string s) const;
    int getNextHistoryRecordId() const; // New helper for unique history IDs

    // Helper for robust argument parsing from a line
    std::vector<std::string> parseArguments(std::istream &stream) const;

public:
    ParkingManager();

    void run();

    // --- Command Handlers ---
    void parkVehicleCmd(const std::vector<std::string> &args);
    void exitVehicleCmd(const std::vector<std::string> &args);
    void listActiveCmd(const std::vector<std::string> &args);
    void listHistoryCmd(const std::vector<std::string> &args);
    void searchActiveCmd(const std::vector<std::string> &args);
    void searchHistoryCmd(const std::vector<std::string> &args);
    void deleteHistoryRecordCmd(const std::vector<std::string> &args);
    void displayHelpCmd() const;

    void displayParkingRecords(const std::vector<ParkingRecord> &records, const std::string &title) const;
};