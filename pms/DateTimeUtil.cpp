#include "DateTimeUtil.h"
#include <ctime>

using namespace std;

string DateTimeUtil::getCurrentTimestamp()
{
    auto now = chrono::system_clock::now();
    return formatTimestamp(now);
}

chrono::system_clock::time_point DateTimeUtil::parseTimestamp(const string &timestampStr)
{
    tm tm = {};
    istringstream ss(timestampStr);
    ss >> get_time(&tm, "%Y-%m-%d %H:%M:%S");

    if (ss.fail())
    {
        return chrono::system_clock::time_point{}; // Return epoch on failure
    }
    return chrono::system_clock::from_time_t(mktime(&tm));
}

long long DateTimeUtil::calculateDurationMinutes(const string &startTimeStr, const string &endTimeStr)
{
    auto startTime = parseTimestamp(startTimeStr);
    auto endTime = parseTimestamp(endTimeStr);

    if (startTime == chrono::system_clock::time_point{} || endTime == chrono::system_clock::time_point{})
    {
        return -1;
    }
    if (endTime < startTime)
    {
        return -2; // Indicate end time before start time specifically
    }
    auto duration = chrono::duration_cast<chrono::minutes>(endTime - startTime);
    return duration.count();
}

string DateTimeUtil::formatTimestamp(const chrono::system_clock::time_point &tp)
{
    time_t time = chrono::system_clock::to_time_t(tp);
    tm buf_tm = {};
#ifdef _WIN32
    localtime_s(&buf_tm, &time);
#else
    localtime_r(&time, &buf_tm);
#endif
    ostringstream oss;
    oss << put_time(&buf_tm, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}