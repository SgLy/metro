#include <stdio.h>
#include <string.h>
#include <map>
#include <string>
#include <vector>
#include <set>
#include <queue>
#include <algorithm>

using std::map;
using std::vector;
using std::string;
using std::pair;
using std::make_pair;
using std::set;
using std::queue;

class Metro {
#define forea
public:
    Metro(vector<string> file)
    {
        char file_address[100], station_name[100];
        int time, last_time, last_station;
        station_count = 0;
        for (auto filename: file) {
            sprintf(file_address, "data/%s.txt", filename.c_str());
            FILE * f = fopen(file_address, "r");
            last_time = last_station = -1;
            while (fscanf(f, "%s%d", station_name, &time) != EOF) {
                auto it = station_index.find(station_name);
                int this_station;
                if (it == station_index.end()) {
                    station_index[station_name] = this_station = station_count++;
                    stations.push_back(station_name);
                    edges.push_back(vector<pair<int, int> >());
                    station_line.push_back(set<string>{filename});
                } else {
                    this_station = it->second;
                    station_line[this_station].insert(filename);
                }
                if (last_station != -1) {
                    edges[last_station].push_back(make_pair(this_station, time - last_time));
                    edges[this_station].push_back(make_pair(last_station, time - last_time));
                }
                last_time = time;
                last_station = this_station;
            }
            fclose(f);
        }
    }

    void listAll()
    {
        for (size_t i = 0; i < stations.size(); ++i) {
            printf("%3lu %s\n", i, stations[i].c_str());
            printf("    Line ");
            for (auto l: station_line[i])
                printf("%s ", l.c_str());
            printf("\n");
            for (auto s: edges[i])
                printf("      %2d mins to %s\n", s.second, stations[s.first].c_str());
        }
    }

    int queryCost(string src, string dest)
    {
        int s = station_index[src];
        int t = station_index[dest];
        return query(s, t).first;
    }

    vector<string> queryRoute(string src, string dest)
    {
        int s = station_index[src];
        int t = station_index[dest];
        vector<string> result;
        for (auto i: query(s, t).second)
            result.push_back(stations[i]);
        return result;
    }

    pair<int, vector<int> > query(int s, int t)
    {
        bool * inqueue = new bool[station_count];
        int * distance = new int[station_count];
        int * from_station = new int[station_count];
        memset(inqueue, 0, sizeof(bool) * station_count);
        memset(distance, 127, sizeof(int) * station_count);
        memset(from_station, 255, sizeof(int) * station_count);
        queue<int> q;
        inqueue[s] = true;
        distance[s] = 0;
        q.push(s);
        while (!q.empty()) {
            int current_station = q.front();
            q.pop();
            for (auto edge: edges[current_station]) {
                int next_station = edge.first;
                int cost = edge.second;
                if (from_station[current_station] != -1)
                    if (!shareSameLine(from_station[current_station], next_station))
                        cost += change_line_cost;
                if (distance[next_station] > distance[current_station] + cost) {
                    distance[next_station] = distance[current_station] + cost;
                    from_station[next_station] = current_station;
                    if (!inqueue[next_station]) {
                        inqueue[next_station] = true;
                        q.push(next_station);
                    }
                }
            }
        }
        int cost = distance[t];
        vector<int> route;
        int station = t;
        while (station != -1) {
            route.push_back(station);
            station = from_station[station];
        }
        std::reverse(route.begin(), route.end());
        delete[] inqueue;
        delete[] distance;
        delete[] from_station;
        return make_pair(cost, route);
    }

private:
    const int change_line_cost = 5;
    int station_count;
    map<string, int> station_index;
    vector<string> stations;
    vector<set<string> > station_line;
    vector<vector<pair<int, int> > > edges;

    bool shareSameLine(int station_A, int station_B)
    {
        auto lines_A = station_line[station_A];
        auto lines_B = station_line[station_B];
        set<string> common_lines;
        std::set_intersection(lines_A.begin(), lines_A.end(),
                              lines_B.begin(), lines_B.end(),
                              std::inserter(common_lines, common_lines.begin()));
        return !common_lines.empty();
    }
};

int main()
{
    Metro metro({"1", "2", "3", "3_North", "4", "5", "6", "8", "GuangFo", "APM"});
    metro.listAll();
    printf("%d\n", metro.queryCost("大学城北", "机场南"));
    for (auto s: metro.queryRoute("大学城北", "机场南"))
        printf("%s ", s.c_str());
    return 0;
}

