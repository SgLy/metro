#include <cstdio>
#include <cstring>
#include <map>
#include <string>
#include <vector>
#include <set>
#include <queue>
#include <fstream>
#include <algorithm>
#include <iostream>
#include <cmath>
using std::map;
using std::string;
using std::ifstream;
using std::set;
using std::queue;
using std::vector;
using std::pair;
using std::cout;
using std::endl;
using std::min;

#define INF (10001)
/**
 * For metro usage, 10001 is large enough.
 * */

struct Response {
    /**
     * A response structure for each query.
     * Path: vector<pair<subway_name, vector<each_station_of_the_subway> > >
     * Money: Money consumed.
     * Cost_time: Time Consumed.
     * Distance: Total Distance started from the start station.
     * Time_between_station: Time delta between each station and next station.
     * */
    vector<pair<string, vector<string> > > path;
    int money, cost_time;
    double distance;
    vector<int> time_between_station;

    Response() {
        money = cost_time = 0, distance = 0.;
    }
};

struct Edge {
    /**
     * A structure store information of each pair of adjacent edge.
     * Start, End: start station and end station name.
     * Cost_time: The time consume on the edge.
     * Distance: The distance of the edge.
     * */
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
        /**
         * Because adjacent edges of each station are stored in set,
         * it need a < operator.
         * */
        return end < t.end;
    }
};

struct State {
    /**
     * A structure that record each station information from the start startion.
     * The structure is used when working on Shortest Path.(Spfa)
     * Pre_station: The previous station of this station.
     *              With the fact that no two subway line intersect with adjacent
     *              two stations, pre_station and now_station can determine which
     *              subway_line is on.
     * Cost_time: The time consumed.
     * Cost_money: The money consumed uptil the previous charge segment.
     *             For example, if user use APM, the user should pay the subway
     *             before it get on APM.
     * Interchange: The number of interchange.
     * Distance: The distance start from previous charge segment.
     * Real_distance: The distance start from start station to previous charge
     *                segment.
     *                That is, the Total_distance is Real_distance plus Distance.
     * */
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
        /**
         * Get cost from start station to this station.
         * Computing by the rules on the formal website.
         * */
        int ret = cost_money + 2;
        const double EPS = 1e-5;
        if(distance > 4. + EPS)
            ret += ceil((min(distance, 12.) - 4. - EPS) / 4.);
        if(distance > 12. + EPS)
            ret += ceil((min(distance, 24.) - 12. - EPS) / 6.);
        if(distance > 24. + EPS)
            ret += ceil((distance - 24. - EPS) / 8.);
        return ret;
    }

    double get_distance() const {
        /**
         * Get total distance from start station to this station.
         * Computing by the distance and real_distance.
         * */
        double ret = distance + real_distance;
        return ret;
    }
};

struct Comp {
    /**
     * A self-define comparator. Set with a string with following:
     * "Money" or "money": To find a path with money consume least.
     * "Distance" or "distance": To find a path with distance least.
     * "Interchange" or "interchange": to find a path with least interchanges.
     * Defaultly: To find a path with time consumed least.
     * */
    string dominate;

    Comp(const char *dominate):dominate(dominate) {}

    bool operator ()(const State &a, const State &b) {
        if(dominate == "Distance" || dominate == "distance")
            return a.get_distance() < b.get_distance();
        if(dominate == "Money" || dominate == "money")
            return a.get_cost() < b.get_cost();
        if(dominate == "Interchange" || dominate == "interchange")
            return a.interchange < b.interchange;
        return a.cost_time < b.cost_time;
    }
};


class Metro {
    /**
     * Manage class ofr GuangZhou metro.
     * */
private :
    map<string, int> station_name_index;
    map<int, string> station_index_name;
    map<string, set<string> > subway_stations, station_belong;
    map<string, set<Edge> > graph;
    int tot_station;

    int get_station_index(string &name) {
        /**
         * Different from query_station_index, it will automatically add the name
         * to database if this name not exists.
         * */
        if(station_name_index.find(name) == station_name_index.end()) {
            station_name_index[name] = ++tot_station;
            station_index_name[tot_station] = name;
        }
        return station_name_index[name];
    }

    string get_subway_on(const Edge &e) {
        /**
         * Determine which subway line is on.
         * */
        string a = e.start, b = e.end;
        for(map<string, set<string> >::iterator
                it = subway_stations.begin();
                it != subway_stations.end(); ++it) {
            if(it->second.find(a) == it->second.end()) continue;
            if(it->second.find(b) == it->second.end()) continue;
            return it->first;
        }
        return "";
    }

    string get_subway_on(string &a, string &b) {
        /**
         * Determine which subway line is on.
         * */
        for(map<string, set<string> >::iterator
                it = subway_stations.begin();
                it != subway_stations.end(); ++it) {
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
        /**
         * Parse a response structure to response query using the information
         * recorded by shortest-path algorithm.
         * */

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
            distance = dist[end].get_distance();

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
                    pass.push_back(pre_station);
                }

                pass.push_back(now_station);
                time_between_station.push_back(
                        pre.cost_time - now.cost_time
                    );

                pre_station = now_station, pre_subway = now_subway;

                if(pre_station == start && pass.size()) {
                    reverse(pass.begin(), pass.end());
                    path.push_back(make_pair(now_subway, pass));
                    pass.clear();
                }
            }
            reverse(time_between_station.begin(), time_between_station.end());
            reverse(path.begin(), path.end());
        }
        return ret;
    }

    Response spfa(string &start, string &end, Comp &cmp) {
        /**
         * Shortest path algorithm.
         * */
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
            for(set<Edge>::iterator
                    sub = graph[u].begin();
                    sub != graph[u].end(); ++sub) {
                Edge e = *sub;
                string now_subway = get_subway_on(e);
                State now(u);
                int &cost_time = now.cost_time,
                    &cost_money = now.cost_money,
                    &interchange = now.interchange;
                double &distance = now.distance, &real_distance = now.real_distance;
                cost_time = pre_state.cost_time + e.cost_time;
                distance = pre_state.distance;
                cost_money = pre_state.cost_money;
                real_distance = pre_state.real_distance;
                interchange = pre_state.interchange;
                if(now_subway != pre_subway && pre_subway != "")
                    ++interchange, cost_time += 2;
                if(now_subway != "APM") distance += e.distance;
                else real_distance += e.distance;
                if(now_subway != pre_subway && now_subway == "APM") {
                    cost_money = now.get_cost() - 2;
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

public :
    static const char* SUBWAY_NAME[];

    void read_data(
            const string &filename,
            vector<pair<string, int> > &station_time,
            vector<pair<string, double> > &station_distance
        ) {
        /**
         * Read data from "filename".
         * Parse the data into two vectors: station_time, station_distance.
         * Station_time: Time delta between each station(Represent by time
         *               consume start from one end).
         * Station_distance: Distance between each adjacent station.
         * */
        station_time.clear(), station_distance.clear();

        ifstream in(filename.c_str());
        string line_data;
        while(getline(in, line_data)) {
            if((int) line_data.size() <= 0) break;
            int pos = line_data.find(' ');
            string name = line_data.substr(0, pos);
            string time_number = line_data.substr(pos + 1, line_data.size() - pos);
            int time;
            sscanf(time_number.c_str(), "%d", &time);
            station_time.push_back(make_pair(name, time));
        }
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
        }
        in.close();
    }

    void check_reverse(
            vector<pair<string, int> > &station_time,
            vector<pair<string, double> > &station_distance) {
        /**
         * Fix data format.
         * */

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

    Metro(const char **subway_name_list, int station_number) {
        tot_station = 0;
        /**
         * Be careful: pass a data file name list in a array, with
         *             station_number indicates the number of data files
         *             array. Pass subway_name_list with SUBWAY_NAME is
         *             suggested.
         * */

        for(int i = 0; i < station_number; ++i) {
            const string subway_name(subway_name_list[i]);
            vector<pair<string, int> > station_time;
            vector<pair<string, double> > station_distance;
            #ifdef WIN32
            read_data("data\\" + subway_name + ".txt", station_time, station_distance);
            #else
            read_data("data/" + subway_name + ".txt", station_time, station_distance);
            #endif

            if(station_time.size() != station_distance.size()) {
                cout << subway_name << " data format is not valid." << endl;
                continue;
            }

            check_reverse(station_time, station_distance);
            int num_station = station_time.size();

            set<string> &subway_contain = subway_stations[subway_name];
            subway_contain.clear();
            for(vector<pair<string, int> >::iterator it = station_time.begin();
                    it != station_time.end();
                    ++it)
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
        /**
         * Return a vector contains all the stations supported with index and its name.
         * */
        vector<pair<int, string> > ret;
        for(map<int, string>::iterator it = station_index_name.begin();
                it != station_index_name.end();
                ++it)
            ret.push_back(make_pair(it->first, it->second));
        return ret;
    }

    vector<pair<string, vector<string> > > list_all_subway() {
        /**
         * Return a vector contains all the subway with its stations.
         * */
        vector<pair<string, vector<string> > > ret;
        for(map<string, set<string> >::iterator it = subway_stations.begin(); it != subway_stations.end(); ++it) {
            string subway_name = it->first;
            int num_stations = it->second.size();
            vector<string> stations(num_stations);
            copy(it->second.begin(), it->second.end(), stations.begin());
            ret.push_back(make_pair(subway_name, stations));
        }
        return ret;
    }

    Response query(const int &start, const int &end, const string &dominate) {
        /**
         * Query start station to end station with "dominate" considering first.
         * Dominate: "Time", "Distance", "Interchange", "Money"
         * */
        Comp comp(dominate.c_str());
        string start_name = query_station_name(start),
               end_name = query_station_name(end);

        Response ret = spfa(start_name, end_name, comp);
        return ret;
    }

    Response query_money(const int &start, const int &end) {
        /**
         * A old interface, not suggested.
         * Please use query instead.
         * */
        return query(start, end, "Money");
    }

    Response query_distance(const int &start, const int &end) {
        /**
         * A old interface, not suggested.
         * Please use query instead.
         * */
        return query(start, end, "Distance");
    }

    Response query_time(const int &start, const int &end) {
        /**
         * A old interface, not suggested.
         * Please use query instead.
         * */
        return query(start, end, "Time");
    }

    int query_station_index(const string &name) {
        /**
         * Query the corresponding index of the name.
         * If the name is not considered as a station's name, it return -1.
         * */
        if(station_name_index.find(name) != station_name_index.end())
            return station_name_index[name];
        return -1;
    }

    string query_station_name(const int &index) {
        /**
         * Query the corresponding name of the index.
         * If the index is not a station index, it return empty string "".
         * */
        if(station_index_name.find(index) != station_index_name.end())
            return station_index_name[index];
        return "";
    }

};
const char* Metro::SUBWAY_NAME[] = {
    "一号线",
    "二号线",
    "三号线",
    "三号线北延段",
    "四号线",
    "五号线",
    "六号线",
    "八号线",
    "广佛线",
    "APM列车"
};

/*
using std::cin;
int main() {
    Metro *metro = new Metro(Metro::SUBWAY_NAME, 10);
    vector<pair<int, string> > stations = metro->list_all_stations();
    for(int i = 0, len = stations.size(); i < len; ++i)
        cout << stations[i].first << ' ' << stations[i].second << endl;
    int a, b;
    cin >> a >> b;
    Response res = metro->query_time(a, b);
    cout << res.money << ' ' << res.cost_time << ' ' << res.distance << endl;
    foreach(it, res.path) {
        cout << it->first;
        cout << ": ";
        foreach(i, it->second)
            cout << *i << ' ';
        cout << endl;
    }
    return 0;
}
*/

