
hashtable gDaysPerMonth;
int gDaysPerMonth[1] = 31;
int gDaysPerMonth[2] = 28;
int gDaysPerMonth[3] = 31;
int gDaysPerMonth[4] = 30;
int gDaysPerMonth[5] = 31;
int gDaysPerMonth[6] = 30;
int gDaysPerMonth[7] = 31;
int gDaysPerMonth[8] = 30;
int gDaysPerMonth[9] = 30;
int gDaysPerMonth[10] = 31;
int gDaysPerMonth[11] = 30;
int gDaysPerMonth[12] = 31;
int gMaxMonth = 12;

hashtable gMonthNames;
string gMonthNames[1] = "Jan";
string gMonthNames[2] = "Feb";
string gMonthNames[3] = "Mar";
string gMonthNames[4] = "Apr";
string gMonthNames[5] = "May";
string gMonthNames[6] = "Jun";
string gMonthNames[7] = "Jul";
string gMonthNames[8] = "Aug";
string gMonthNames[9] = "Sep";
string gMonthNames[10] = "Oct";
string gMonthNames[11] = "Nov";
string gMonthNames[12] = "Dec";

int GetNumDays(int month, int year) {
    // -- get the number of days in the month
    int max_day = gDaysPerMonth[month];
    
    // -- see if we need to adjust for a leap year
    if(month == 2 && ((year % 4 ) == 0)) {
        max_day = max_day + 1;
    }
    
    return max_day;
}

bool ValidDate(int day, int month, int year) {
    // -- sanity check
    if(month <= 0 || month > gMaxMonth) {
        Print("Error - invalid month");
        return false;
    }

    // -- find out what the max day is for the month (including leap years)
    int max_day = GetNumDays(month, year);
    
    // -- see if the day is valid
    if(day <= 0 || day > max_day) {
        Print("Error - invalid day");
        return false;
    }
    
    // -- date is valid
    return true;
}

void PrintDate(int day, int month, int year) {
    if(!ValidDate(day, month, year)) {
        Print("Error - invalid date");
        return;
    }
    
    Print(gMonthNames[month]);
    Print(day);
    Print(year);
}

// -- calculate the date, "num_days" from now
bool FutureDate(int day, int month, int year, int num_days) {
    // -- ensure we start with a valid date
    if(!ValidDate(day, month, year)) {
        Print("Error - invalid date");
        return false;
    }
    
    // -- loop until we have no more days to add
    while(num_days > 0) {
        // -- get the number of days left in the current month
        int max_days = GetNumDays(month, year);
        
        // -- see how many days are left at the end of the month
        int days_left = max_days - day;
        
        // -- if there are enough days left in the current month,
        // -- advance the day, and we're done
        if(days_left >= num_days) {
            day = day + num_days;
            PrintDate(day, month, year);
            return true;
        }
        
        // -- otherwise, advance to the next month (wrap around back
        // -- to January, and advance the year if needed
        day = 1;
        month = month + 1;
        if(month > gMaxMonth) {
            month = 1;
            year = year + 1;
        }
        num_days = num_days - days_left - 1;
    }
    
    // -- success
    PrintDate(day, month, year);
    return true;
}

// -- calculate the previous date "num_days" before now
bool PastDate(int day, int month, int year, int num_days) {
    // -- ensure we start with a valid date
    if(!ValidDate(day, month, year)) {
        Print("Error - invalid date");
        return false;
    }
    
    // -- loop until we have no more days to add
    while(num_days > 0) {
        if(num_days < day) {
            day = day - num_days;
            PrintDate(day, month, year);
            return true;
        }
        
        // -- previous month
        month = month - 1;
        if(month <= 0) {
            month = 12;
            year = year - 1;
        }
        
        // -- get the max day for the prev month
        int max_days = GetNumDays(month, year);
        
        // -- subtract the current day from the num_days
        num_days = num_days - day;
        
        // -- set the day to be the last day of the new month
        day = max_days;
    }
        
    // -- success
    PrintDate(day, month, year);
    return true;
}








