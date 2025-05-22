// DateTimeUtil.h
#pragma once

#include <string>
#include <chrono>  // For time operations
#include <iomanip> // For put_time, get_time
#include <sstream> // For string stream conversions

using namespace std;

class DateTimeUtil
{
public:
    // Gets the current timestamp as a string in "YYYY-MM-DD HH:MM:SS" format
    static string getCurrentTimestamp();

    // Parses a timestamp string ("YYYY-MM-DD HH:MM:SS") into a time_point
    static chrono::system_clock::time_point parseTimestamp(const string &timestampStr);

    // Calculates the duration in minutes between two timestamps
    // Returns -1 if parsing fails or endTime is before startTime
    static long long calculateDurationMinutes(const string &startTimeStr, const string &endTimeStr);

    // Formats a time_point into a string
    static string formatTimestamp(const chrono::system_clock::time_point &tp);
};