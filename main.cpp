#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "metro.cpp"

#ifdef WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif

void clearScreen()
{
    #ifdef WIN32
    system("cls");
    #else
    system("clear");
    #endif
}

void milliSleep(size_t ms) {
    #ifdef WIN32
    Sleep(ms);
    #else
    timespec ts;
    ts.tv_sec = ms / 1000;
    ts.tv_nsec = (ms % 1000) * 1000000L;
    nanosleep(&ts, NULL);
    #endif
}

int getInt()
{
    int res = 0;
    char c = getchar();
    for ( ; c < '0' || '9' < c; c = getchar());
    for ( ; '0' <= c && c <= '9'; c = getchar())
        res = res * 10 + c - '0';
    return res;
}

char * time()
{
    char * res = new char[100];
    const time_t t = time(NULL);
    struct tm * current = localtime(&t);
    sprintf(res, "%04d-%02d-%02d %02d:%02d:%02d", current->tm_year + 1900,
            current->tm_mon + 1, current->tm_mday, current->tm_hour,
            current->tm_min, current->tm_sec);
    return res;
}

void enterConfirm()
{
    for (char c = getchar(); c != '\n'; c = getchar());
}

void bigLine()
{
    for (int i = 0; i < 50; ++i)
        putchar('=');
    puts("");
}

void smallLine()
{
    for (int i = 0; i < 50; ++i)
        putchar('-');
    puts("");
}

void header()
{
    clearScreen();
    bigLine();
    printf("    Guangzhou Metro Querying System\n\n");
    printf("    time: %s\n", time());
    bigLine();
}

void action1(Metro * metro)
{
    vector< pair< string, vector<string> > > allLines =  metro->list_all_subway();
    for (size_t i = 0; i < allLines.size(); ++i) {
        header();
        printf("  Line %s:\n", allLines[i].first.c_str());
        vector<string> &allStations = allLines[i].second;
        for (size_t j = 0; j < allStations.size(); ++j)
            printf("    Station %2zu: %s\n", j + 1, allStations[j].c_str());
        if (i != allLines.size() - 1) {
            printf("\n  ENTER to next line...\n");
            enterConfirm();
        }
    }
    printf("\n\n  ENTER to continue..\n");
    enterConfirm();
}

int pickStation(const char msg[], Metro * metro)
{
    header();
    puts(msg);
    bigLine();
    vector< pair< string, vector<string> > > allLines =  metro->list_all_subway();
    for (size_t i = 0; i < allLines.size(); ++i)
        printf("  %zu. Line %s\n", i + 1, allLines[i].first.c_str());
    smallLine();
    int line;
    while (true) {
        printf("  Choose a line: ");
        line = getInt();
        if (0 < line && line <= (int)allLines.size())
            break;
        printf("\n  Invalid input!\n\n");
    }
    header();
    puts(msg);
    bigLine();
    printf("  (%2d) Line %s\n\n", line, allLines[line - 1].first.c_str());
    vector<string> &allStations = allLines[line - 1].second;
    for (size_t i = 0; i < allStations.size(); ++i)
        printf("  %zu. %s\n", i + 1, allStations[i].c_str());
    smallLine();
    int station;
    while (true) {
        printf("  Choose a station: ");
        station = getInt();
        if (0 < station && station <= (int)allStations.size())
            break;
        printf("\n  Invalid input!\n\n");
    }
    header();
    puts(msg);
    bigLine();
    printf("  Selected:\n");
    printf("  (%2d) Line    %s\n", line, allLines[line - 1].first.c_str());
    printf("  (%2d) Station %s\n", station, allStations[station - 1].c_str());
    printf("\n  ENTER to continue...\n");
    enterConfirm();
    return metro->query_station_index(allStations[station - 1]);
}

void printQueryResult(const Response &response, int query_type)
{
    vector< pair<string, vector<string> > > path = response.path;
    int money = response.money;
    int cost_time = response.cost_time;
    double distance = response.distance;
    vector<int> time_between_station = response.time_between_station;

    header();
    printf("  Query Result\n");
    bigLine();
    printf("  Departure: %s\n", path.front().second.front().c_str());
    printf("  Arrival:   %s\n", path.back().second.back().c_str());
    smallLine();
    char msg[100];
    sprintf(msg, "  Estimated duration : %6d minute", cost_time);
    printf("%.40s %s\n", msg, (query_type == 1 ? "[Min]" : ""));
    sprintf(msg, "  Travel Distance    : %6.2lf km    ", distance);
    printf("%.40s %s\n", msg, (query_type == 2 ? "[Min]" : ""));
    sprintf(msg, "  Ticket fee         : %6d RMB   ",    money);
    printf("%.40s %s\n", msg, (query_type == 3 ? "[Min]" : ""));
    smallLine();
    for (size_t i = 0; i < path.size(); ++i) {
        string& line = path[i].first;
        vector<string>& stations = path[i].second;
        if (i == 0)
            printf("  Depart from %s, Line %s\n", stations[0].c_str(), line.c_str());
        else
            printf("  Interchange to Line %s\n", line.c_str());
        for (size_t j = 0; j < stations.size(); ++j)
            printf("    %s\n", stations[j].c_str());
    }
    printf("  Arrived at %s, Line %s\n", path.back().second.back().c_str(), path.back().first.c_str());
    printf("\n\n  ENTER to continue...\n");
    enterConfirm();
}

void subMenu2(Metro * metro)
{
    int src_ind, dest_ind;
    while (true) {
        if ((src_ind = pickStation("  Select DEPARTURE station", metro)) != -1)
            break;
        printf("\n  Unable to find such station!\n");
        milliSleep(750);
    }
    puts("");
    while (true) {
        if ((dest_ind = pickStation("  Select ARRIVAL station", metro)) != -1)
            break;
        printf("\n  Unable to find such station!\n");
        milliSleep(750);
    }
    string src = metro->query_station_name(src_ind);
    string dest = metro->query_station_name(dest_ind);
    int query_type;
    Response response;
    while (true) {
        header();
        printf("  Departure: %s\n", src.c_str());
        printf("  Arrival:   %s\n", dest.c_str());
        smallLine();
        printf("  1. Query minimum time\n");
        printf("  2. Query minimum distance\n");
        printf("  3. Query minimum ticket fee\n");
        smallLine();
        printf("  Your choice:  ");
        switch (query_type = getInt()) {
            case 1:
                response = metro->query_time(src_ind, dest_ind);
                break;
            case 2:
                response = metro->query_distance(src_ind, dest_ind);
                break;
            case 3:
                response = metro->query_money(src_ind, dest_ind);
                break;
            default:
                printf("\n  Invalid input!\n");
                milliSleep(750);
                continue;
        }
        break;
    }
    printQueryResult(response, query_type);
}

bool mainMenu(Metro * metro)
{
    clearScreen();
    header();
    printf("  1. Show all stations\n");
    printf("  2. Perform a query\n");
    printf("  3. Exit\n");
    smallLine();
    printf("  Your choice: ");
    switch (getInt()) {
        case 1:
            action1(metro);
            return true;
        case 2:
            subMenu2(metro);
            return true;
        case 3:
            return false;
        default:
            printf("\n  Invalid input!\n");
            milliSleep(750);
    }
    return true;
}

void exitMessage()
{
    clearScreen();
    header();
    printf("    Thanks for your using!\n");
    milliSleep(1000);
}

int main()
{
    Metro *metro = new Metro(Metro::SUBWAY_NAME, 10);
    while (mainMenu(metro))
        ;
    exitMessage();
    return 0;
}

