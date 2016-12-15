#include <stdio.h>
#include <string.h>
#include <map>
#include <string>
#include <vector>
#include <set>
#include <queue>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <iostream>
using namespace std;

struct Response {
    vector<pair<int, vector<string> > > path;
    int money, cost_time;
    double distance;
    vector<int> time_between_station;
};

class Metro {
#define foreach(x, y) \
    for(__typeof((y).begin()) x = (y).begin(); x != (y).end(); ++x)
#define INF (10001)
private :
    static const char* SUBWAY_NAME[];

    struct Edge {
        string start, end;
        int cost_time;
        double distance;

        Edge(
                const string &start, 
                const string &end, 
                const int &cost_time,
                const double &distance
            ) :  
            start(start), 
            end(end), 
            cost_time(cost_time),
            distance(distance) {
        }

        bool operator <(const Edge &t) const {
            return end < t.end;
        }
    };

    struct State {
        string pre_station, pre_subway, on_subway;
        int cost_time, cost_money, interchange;
        double distance;

        State() {
            cost_time = cost_money = interchange = INF;
            pre_station = pre_subway = on_subway = "";
        }

        State(
                const string &pre, const string &on_subway
                const int &cost_time,
                const int &cost_money,
                const int &interchange,
                const double &distance
            ):
            pre(pre), on_subway(on_subway),
            cost_time(cost_time),
            cost_money(cost_money),
            interchange(interchange),
            distance(distance) {
        }

        int get_cost() const {
            int ret = cost_money + 2;
            const double EPS = 1e-5;
            if(distance > 4. + EPS)
                ret += ceil((min(distance, 12.) - EPS) / 4.);
            if(distance > 12. + EPS)
                ret += ceil((min(distance, 24.) - EPS) / 6.);
            if(distance > 24. + EPS)
                ret += ceil((distance - EPS) / 8.);
            return ret;
        }
    };

    struct Comp {
        string dominate;

        Comp(dominate):dominate(dominate) {}

        bool operator ()(const State &a, const State &b) {
            if(dominate == "Distance" || dominate == "distance")
                return a.distance < b.distance;
            if(dominate == "Money" || dominate == "money")
                return a.get_cost() < b.get_cost();
            return a.cost_time < b.cost_time;
        }
    };

    map<string, int> station_name_index;
    map<int, string> station_index_name;
    map<string, set<string> > subway_stations, station_belong;
    map<string, set<Edge> > graph;
    int tot_station;

    int get_station_index(const string &name) {
        if(station_name_index.find(name) == station_name_index.end())
            station_name_index[name] = ++tot_station;
        return station_name_index[name];
    }

public :
    void read_data(
            const string &filename,
            vector<pair<string, int> > &station_time,
            vector<pair<string, double> > &station_distance
        ) {
        station_time.clear(), station_distance.clear();

        ifstream in(filename.c_str());
        string line_data;
        while(in >> line_data) {
            if((int) line_data.size() <= 0) break;
            string name;
            int time;
            stringstream line_in(line_data);
            line_in >> name >> time;
            station_time.push_back(make_pair(name, time));
        }
        while(in >> line_data) {
            if((int) line_data.size() <= 0) break;
            string name;
            double distance;
            stringstream line_in(line_data);
            if(line_data.find(' ') == string::npos)
                line_in >> name, distance = 0.;
            else line_in >> name >> distance;
            station_time.push_back(make_pair(name, distance));
        }
        in.close();
    }

    void check_reverse(
            vector<pair<string, int> > &station_time,
            vector<pair<string, double> > &station_distance

        if(station_distance.begin()->first != 
                station_time.begin()->first) {
            vector<pair<string, double> > temp;
            for(int i = station_time.size() - 1; i >= 0; --i)
                if(i) temp.push_back(make_pair(
                            station_distance[i].first,
                            station_distance[i - 1].second
                        ));
                else temp.push_back(make_pair(
                            station_distance[i].first,
                            0.
                        ));

            station_distance = temp;
        }
    }

    Metro(const char **a = SUBWAY_NAME) {
        tot_station = 0;

        int station_number = sizeof(a) / 4;
        // debug
        cout << station_number << endl;
        for(int i = 0; i < station_number; ++i) {
            const string subway_name(SUBWAY_NAME[i]);
            vector<pair<string, int> > station_time;
            vector<pair<string, double> > station_distance;
            read_data(subway_name, station_time, station_distance);

            if(station_time.size() != station_distance.size()) {
                cout << filename << " data format is not valid." << endl;
                continue;
            }

            check_reverse(station_time, station_distance);
            int num_station = station_time.size();

            set<string> &subway_contain = subway_stations[subway_name];
            subway_contain.clear();
            foreach(it, station_time)
                subway_contian.insert(it->first),
                station_belong[it->first].insert(subway_name);
            for(int i = 0; i < num_station - 1; ++i) {
                string start = station_time[i].first,
                       end = station_time[i + 1].first;
                int time_delta =
                    station_time[i + 1].second - station_time[i].time;
                double distance = station_distance[i];
                graph[start].insert(Edge(start, end, time_delta, distance));
            }
        }
    }

    vector<pair<int, string> > list_all_stations() const {
        vector<pair<int, string> > ret;
        foreach(it, station_index_name)
            ret.push_back(make_pair(it->first, it->second));
        return ret;
    }

    Response spfa(const string &start, const string &end, const Comp &cmp) {
        map<string, State> dist;
        queue<string> que;
        map<string, bool> inque;

        dist[start] = State("", 0, 0, 0, 0.);
        que.push(start), inque[start] = true;
        while(!que.empty()) {
            string u = que.front();
            que.pop(), inque[u] = false;
        }
    }

    Response query_money(const int &start, const int &end) const {
        Comp comp("Money");
        string start_name = query_station_name(start),
               end_name = query_station_name(end);
        Reponse ret = spfa(start_name, end_name, comp);
        return ret;
    }

    Response query_distance(const int &start, const int &end) const {
        Comp comp("Distance");
        string start_name = query_station_name(start),
               end_name = query_station_name(end);
        Reponse ret = spfa(start_name, end_name, comp);
        return ret;
    }

    Response query_time(const int &start, const int &end) const {
        Comp comp("Time");
        string start_name = query_station_name(start),
               end_name = query_station_name(end);
        Response ret = spfa(start_name, end_name, comp);
        return ret;
    }

    int query_station_index(const string &name) const {
        if(station_name_index.find(name) != station_name_index.end())
            return station_name_index[name];
        return -1;
    }

    string query_station_name(const int &index) const {
        if(station_index_name.find(index) != station_index_name.end())
            return station_index_name[index];
        return "";
    }

#undef foreach
#undef INF
};
const char* Metro::SUBWAY_NAME[] = {
    "1",
    "2",
    "3",
    "3_North",
    "4",
    "5",
    "6",
    "8",
    "GuangFo",
    "APM"
};

int main() {
    Metro *metro = new Metro();
    return 0;
}


