#include <iostream>
#include <string>
using namespace std;

#define TOTAL_APOCALYPSE 900000000 // Datum der totalen Apokalypse

struct Date
{
    int day, month;
    long long year;
    bool isGregorian;

    Date( const int& _d, const int& _m, const long long& _y, const bool& _g = true) : day(_d), month(_m), year(_y), isGregorian(_g) {};

    bool isLeapYear() {
        // Erweiterte Schaltregeln des gregorianischen Kalendars
        if ( year % 400 == 0 && isGregorian ) return true;
        if ( year % 100 == 0 && isGregorian ) return false;

        // "default" julian leap year rule
        if ( year % 4 == 0 ) return true; 

        // sonst: kein Schaltjahr
        return false;
    }

    int getMonthLength( const int _m ) {
        switch (_m) {
            case 1: return 31;
            case 2: return isLeapYear() ? 29 : 28;
            case 3: return 31;
            case 4: return 30;
            case 5: return 31;
            case 6: return 30;
            case 7: return 31;
            case 8: return 31;
            case 9: return 30;
            case 10: return 31;
            case 11: return 30;
            case 12: return 31;
            default: return 0;
        }
    }

    void nextDay(void)
    { // Inkrementieren des Datums 
        if ( day == getMonthLength(month) ) { // Monatsgrenze überschritten
            day = 1;
            if ( month == 12 ) { // Jahresgrenze überschritten
                month = 1;
                year++;
            }else{ month++; }
        }else{ day++; }
    }
    Date& operator++() { nextDay(); return *this; }

    void previousDay(void)
    { // Dekrementieren des Datums 
        if ( day == 1 ) { // Monatsgrenze überschritten
            if ( month == 1 ) { // Jahresgrenze überschritten
                day = 31;
                month = 12;
                year--;
            } else{ day = getMonthLength(month - 1); month--; }
        }else{ day--; }
    }
    Date& operator--() { previousDay(); return *this; }

    string to_str(void) { return to_string(day) + "." + to_string(month) + "." + to_string(year); }

    bool operator==(const Date& b) {
        return ( year == b.year && month == b.month && day == b.day && isGregorian == b.isGregorian );
    }

};

int getTimeDiff( const Date& d )
{ // Zeitdifferenz des julianischen und gregorianischen Datums zum Zeitpunkt 'd' zur gegenseitigen Umrechnung
    // Formel: siehe https://de.wikipedia.org/wiki/Umrechnung_zwischen_julianischem_und_gregorianischem_Kalender
    int h = int( ( d.month <= 2 ? d.year - 1 : d.year ) / 100 );
    int a = int( h / 4 );
    int b = h % 4;
    int dT = 3*a+b-2;

    return dT;
}

Date greg2jul( const Date& d )
{
    Date tempDate = d;
    tempDate.isGregorian = false;
    int dT = getTimeDiff(tempDate);
    for( int i = 1; i <= dT; i++ ) --tempDate;
    return tempDate;
}

Date jul2greg( const Date& d )
{
    Date tempDate = d;
    tempDate.isGregorian = true;
    int dT = getTimeDiff( tempDate);
    for( int i = 1; i <= dT; i++ ) ++tempDate;
    return tempDate;
}

Date getEasterDate ( const long long year, const bool isGregorian )
{ // Errechnet das Datum des Ostersonntag im Jahr 'year' mit der Gauß'schen Osterformel
    // Siehe https://de.wikipedia.org/wiki/Gau%C3%9Fsche_Osterformel#Eine_erg.C3.A4nzte_Osterformel
    int K = year / 100;

    int M = !isGregorian ? 15   : (15 + (3*K + 3) / 4 - (8*K + 13) / 25);
    int S = !isGregorian ? 0    : (2 - (3*K + 3) / 4);

    int A = year % 19;
    int D = (19*A + M) % 30;
    int R = (D + A / 11) / 29;
    int OG = 21 + D - R;
    int SZ = 7 - (year + year / 4 + S) % 7;
    int OE = 7 - (OG - SZ) % 7;
    int OS = OG + OE;

    int month   = (OS <= 31) ? 3: 4;
    int day     = (OS <= 31) ? OS : (OS - 31);
    
    return Date( day, month, year );
}

int main(void)
{
    
    // Gregorianisches Ostern fällt auf julianisches Weihnachten
    for( long long int y = 2000; y <= TOTAL_APOCALYPSE; y++ )
    {
        Date easterDate_greg = getEasterDate(y, true); // Osterdatum im Jahr 'y'
        Date easterDateInJul = greg2jul(easterDate_greg); // Umrechnen in den julianischen Kalender

        if ( easterDateInJul.day == 25 && easterDateInJul.month == 12 ) // fällt dieses im julianischen Kalender auf den 25.12., dann Erfolg
        {
            cout << endl << "ERFOLG 1" << endl << "Ostern, " << easterDate_greg.to_str() << " [greg] = Weihnachten, " << easterDateInJul.to_str() << " [jul]";
            cout << endl << "Zeitunterschied: " << getTimeDiff(easterDate_greg);
            break;
        }
    }

    // Julianisches Ostern fällt auf gregorianisches Weihnachten
    for( long long int y = 2000; y <= TOTAL_APOCALYPSE; y++ )
    {
        Date easterDate_jul = getEasterDate( y, false );
        Date easterDateInGreg = jul2greg(easterDate_jul);

        if ( easterDateInGreg.day == 25 && easterDateInGreg.month == 12 )
        {
            cout << endl << "ERFOLG 2" << endl << "Ostern, " << easterDate_jul.to_str() << " [jul] = Weihnachten, " << easterDateInGreg.to_str() << " [greg]";
            cout << endl << "Zeitunterschied: " << getTimeDiff( easterDateInGreg);
            break;
        }
    }

    cout << endl;
    return 1;
}
