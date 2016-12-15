#include <cstdio>
#include <iomanip>
#include <cstring>
#include <map>
#include <string>
#include <vector>
#include <cmath>
#include <set>
#include <queue>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <iostream>
using namespace std;

#define foreach(x, y) \
    for(__typeof((y).begin()) x = (y).begin(); x != (y).end(); ++x)
#define INF (10001)

struct Response {
    vector<pair<string, vector<string> > > path;
    int money, cost_time;
    double distance;
    vector<int> time_between_station;

    Response() {
        money = cost_time = 0, distance = 0.;
    }
};

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
    string pre_station;
    int cost_time, cost_money, interchange;
    double distance, real_distance;

    State(
            const string &pre_station = "",
            const int &cost_time = INF,
            const int &cost_money = INF,
            const int &interchange = INF,
            const double &distance = INF
        ):
        pre_station(pre_station),
        cost_time(cost_time),
        cost_money(cost_money),
        interchange(interchange),
        distance(distance) {
        real_distance = 0.;
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

    Comp(const char *dominate):dominate(dominate) {}

    bool operator ()(const State &a, const State &b) {
        if(dominate == "Distance" || dominate == "distance")
            return a.distance < b.distance;
        if(dominate == "Money" || dominate == "money")
            return a.get_cost() < b.get_cost();
        return a.cost_time < b.cost_time;
    }
};


class Metro {
private :
    map<string, int> station_name_index;
    map<int, string> station_index_name;
    map<string, set<string> > subway_stations, station_belong;
    map<string, set<Edge> > graph;
    int tot_station;

    int get_station_index(string &name) {
        if(station_name_index.find(name) == station_name_index.end()) {
            station_name_index[name] = ++tot_station;
            station_index_name[tot_station] = name;
        }
        return station_name_index[name];
    }

    string get_subway_on(const Edge &e) {
        string a = e.start, b = e.end;
        foreach(it, subway_stations) {
            if(it->second.find(a) == it->second.end()) continue;
            if(it->second.find(b) == it->second.end()) continue;
            return it->first;
        }
        return "";
    }

    string get_subway_on(string &a, string &b) {
        foreach(it, subway_stations) {
            if(it->second.find(a) == it->second.end()) continue;
            if(it->second.find(b) == it->second.end()) continue;
            return it->first;
        }
        return "";
    }

    Response parse_response(
            map<string, State> &dist, 
            string &start, 
            string &end
            ) {
        Response ret;
        int &money = ret.money, &cost_time = ret.cost_time;
        double &distance = ret.distance;
        vector<pair<string, vector<string> > > &path = ret.path;
        vector<int> &time_between_station = ret.time_between_station;
        if(dist.find(end) == dist.end()) {
            money = cost_time = INF, distance = INF;
            path.clear(), time_between_station.clear();
        } else {
            money = dist[end].get_cost();
            cost_time = dist[end].cost_time;
            distance = dist[end].distance + dist[end].real_distance;

            string pre_subway = "", pre_station = end;
            vector<string> pass;
            pass.push_back(end);
            while(pre_station != start) {
                State pre = dist[pre_station];
                string now_station = pre.pre_station;
                State now = dist[now_station];
                string now_subway = get_subway_on(now_station, pre_station);
                if(now_subway != pre_subway && pre_subway != "") {
                    if(pass.size()) {
                        reverse(pass.begin(), pass.end());
                        path.push_back(make_pair(pre_subway, pass));
                        pass.clear();
                    }
                }

                pass.push_back(now_station);
                time_between_station.push_back(
                        pre.cost_time - now.cost_time
                    );

                pre_station = now_station;

                if(pre_station == start && pass.size()) {
                    reverse(pass.begin(), pass.end());
                    path.push_back(make_pair(now_subway, pass));
                    pass.clear();
                }
            }
            reverse(time_between_station.begin(), time_between_station.end());
        }
        return ret;
    }

public :
    static const char* SUBWAY_NAME[];

    void read_data(
            const string &filename,
            vector<pair<string, int> > &station_time,
            vector<pair<string, double> > &station_distance
        ) {
        station_time.clear(), station_distance.clear();

        ifstream in(filename.c_str(), ios::in);
        // ofstream out((filename + ".out").c_str(), ios::out);
        string line_data;
        while(getline(in, line_data)) {
            if((int) line_data.size() <= 0) break;
            int pos = line_data.find(' ');
            string name = line_data.substr(0, pos);
            string time_number = line_data.substr(pos + 1, line_data.size() - pos);
            int time;
            sscanf(time_number.c_str(), "%d", &time);
            station_time.push_back(make_pair(name, time));
            // foreach(c, name) out << hex << ((int) *c) << ' ';
            // out << endl;
            // out << name << ' ' << time << endl;
        }
        // out << flush;
        while(getline(in, line_data)) {
            if((int) line_data.size() <= 0) break;
            double distance = 0.;
            string name;
            if(line_data.find(' ') == string::npos)
                name = line_data;
            else {
                int pos = line_data.find(' ');
                name = line_data.substr(0, pos);
                string distance_number = line_data.substr(pos + 1, line_data.size() - pos);
                sscanf(distance_number.c_str(), "%lf", &distance);
            }
            station_distance.push_back(make_pair(name, distance));
            // foreach(c, name) out << hex << ((int) *c) << ' ';
            // out << endl;
            // out << name << ' ' << distance << endl;
        }
        in.close();
    }

    void check_reverse(
            vector<pair<string, int> > &station_time,
            vector<pair<string, double> > &station_distance) {

        if((station_distance.begin())->first != 
                (station_time.begin())->first) {
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

    Metro(const char **a, int station_number) {
        tot_station = 0;

        // debug
        cout << station_number << endl;
        for(int i = 0; i < station_number; ++i) {
            const string subway_name(SUBWAY_NAME[i]);
            vector<pair<string, int> > station_time;
            vector<pair<string, double> > station_distance;
            read_data("data\\" + subway_name + ".txt", station_time, station_distance);

            if(station_time.size() != station_distance.size()) {
                cout << subway_name << " data format is not valid." << endl;
                continue;
            }

            check_reverse(station_time, station_distance);
            int num_station = station_time.size();

            set<string> &subway_contain = subway_stations[subway_name];
            subway_contain.clear();
            foreach(it, station_time)
                subway_contain.insert(it->first),
                station_belong[it->first].insert(subway_name);
            for(int i = 0; i < num_station - 1; ++i) {
                string start = station_time[i].first,
                       end = station_time[i + 1].first;
                int time_delta =
                    station_time[i + 1].second - station_time[i].second;
                double distance = station_distance[i].second;
                graph[start].insert(Edge(start, end, time_delta, distance));
                graph[end].insert(Edge(end, start, time_delta, distance));

                get_station_index(start);
                get_station_index(end);
            }
        }
    }

    vector<pair<int, string> > list_all_stations() {
        vector<pair<int, string> > ret;
        foreach(it, station_index_name)
            ret.push_back(make_pair(it->first, it->second));
        return ret;
    }

    vector<pair<string, vector<string> > > list_all_subway() {
        vector<pair<string, vector<string> > > ret;
        foreach(it, subway_stations) {
            string subway_name = it->first;
            int num_stations = it->second.size();
            vector<string> stations(num_stations);
            copy(it->second.begin(), it->second.end(), stations.begin());
            ret.push_back(make_pair(subway_name, stations));
        }
        return ret;
    }

    Response spfa(string &start, string &end, Comp &cmp) {
        if(start == end) {
            Response res;
            return res;
        }

        map<string, State> dist;
        queue<string> que;
        map<string, bool> inque;

        dist[start] = State("", 0, 0, 0, 0.);
        que.push(start), inque[start] = true;
        while(!que.empty()) {
            string u = que.front();
            que.pop(), inque[u] = false;
            State pre_state = dist[u];
            string pre_subway = get_subway_on(pre_state.pre_station, u);
            foreach(sub, graph[u]) {
                Edge e = *sub;
                string now_subway = get_subway_on(e);
                State now(u);
                int &cost_time = now.cost_time,
                    &cost_money = now.cost_money,
                    &interchange = now.interchange;
                double &distance = now.distance;
                cost_time = pre_state.cost_time + e.cost_time;
                distance = pre_state.distance + e.distance;
                cost_money = pre_state.cost_money;
                now.real_distance = pre_state.real_distance;
                if(now_subway != pre_subway)
                    ++interchange, cost_time += 2;
                if(now_subway != "APM") distance += e.distance;
                if(now_subway != pre_subway && now_subway == "APM") {
                    cost_money = now.get_cost() + 2;
                    now.real_distance += now.distance;
                    now.distance = 0.;
                }

                State nex = dist[e.end];
                if(cmp(now, nex)) {
                    dist[e.end] = now;
                    if(!inque[e.end])
                        inque[e.end] = true, que.push(e.end);
                }
            }
        }

        Response path = parse_response(dist, start, end);
        return path;
    }

    Response query_money(const int &start, const int &end) {
        Comp comp("Money");
        string start_name = query_station_name(start),
               end_name = query_station_name(end);
        Response ret = spfa(start_name, end_name, comp);
        return ret;
    }

    Response query_distance(const int &start, const int &end) {
        Comp comp("Distance");
        string start_name = query_station_name(start),
               end_name = query_station_name(end);
        Response ret = spfa(start_name, end_name, comp);
        return ret;
    }

    Response query_time(const int &start, const int &end) {
        Comp comp("Time");
        string start_name = query_station_name(start),
               end_name = query_station_name(end);
        cout << start_name << ' ' << end_name << endl;
        Response ret = spfa(start_name, end_name, comp);
        return ret;
    }

    int query_station_index(const string &name) {
        if(station_name_index.find(name) != station_name_index.end())
            return station_name_index[name];
        return -1;
    }

    string query_station_name(const int &index) {
        if(station_index_name.find(index) != station_index_name.end())
            return station_index_name[index];
        return "";
    }

};
#undef foreach
#undef INF
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
    Metro *metro = new Metro(Metro::SUBWAY_NAME, 10);
    Response res = metro->query_time(1, 2);
    cout << res.money << ' ' << res.cost_time << ' ' << res.distance << endl;
    return 0;
}

