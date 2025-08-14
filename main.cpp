#include <bits/stdc++.h>
#include <mysql.h>
#pragma comment(lib, "libmysql.lib")
using namespace std;

struct Course { int id; string name; };
struct Slot { int course_id; string dow; int start; int end; };

bool conflict(const Slot& a, const Slot& b){
    if (a.dow != b.dow) return false;
    return !(a.end <= b.start || b.end <= a.start);
}

MYSQL* connect_db(const char* host, const char* user, const char* pass, const char* db, unsigned int port){
    MYSQL* conn = mysql_init(nullptr);
    if(!mysql_real_connect(conn, host, user, pass, db, port, NULL, 0)){
        cerr << "MySQL connect error: " << mysql_error(conn) << "\n";
        exit(1);
    }
    return conn;
}

void run_sql(MYSQL* conn, const string& q){
    if(mysql_query(conn, q.c_str())){
        cerr << "SQL error: " << mysql_error(conn) << "\n";
    }
}

vector<Course> loadCourses(MYSQL* conn){
    vector<Course> out;
    run_sql(conn, "SELECT course_id, name FROM courses ORDER BY course_id;");
    MYSQL_RES* res = mysql_store_result(conn);
    MYSQL_ROW row;
    while((row = mysql_fetch_row(res))){
        out.push_back({stoi(row[0]), row[1]});
    }
    mysql_free_result(res);
    return out;
}

unordered_map<int, vector<int>> loadPrereqs(MYSQL* conn){
    unordered_map<int, vector<int>> mp;
    run_sql(conn, "SELECT course_id, prereq_id FROM prerequisites;");
    MYSQL_RES* res = mysql_store_result(conn);
    MYSQL_ROW row;
    while((row = mysql_fetch_row(res))){
        int c = stoi(row[0]), p = stoi(row[1]);
        mp[c].push_back(p);
    }
    mysql_free_result(res);
    return mp;
}

vector<Slot> loadSlots(MYSQL* conn){
    vector<Slot> out;
    run_sql(conn, "SELECT course_id, day_of_week, start_time, end_time FROM timeslots;");
    MYSQL_RES* res = mysql_store_result(conn);
    MYSQL_ROW row;
    while((row = mysql_fetch_row(res))){
        out.push_back({stoi(row[0]), row[1], stoi(row[2]), stoi(row[3])});
    }
    mysql_free_result(res);
    return out;
}

void addCourse(MYSQL* conn){
    int id; string name;
    cout << "Course ID: "; cin >> id;
    cout << "Course Name: "; cin.ignore(); getline(cin, name);
    string q = "INSERT INTO courses(course_id,name) VALUES(" + to_string(id) + ", '" + name + "')";
    run_sql(conn, q);
    cout << "Added.\n";
}

void addPrereq(MYSQL* conn){
    int c, p;
    cout << "Course ID: "; cin >> c;
    cout << "Prerequisite ID: "; cin >> p;
    string q = "INSERT INTO prerequisites(course_id,prereq_id) VALUES(" + to_string(c) + "," + to_string(p) + ")";
    run_sql(conn, q);
    cout << "Added.\n";
}

void addSlot(MYSQL* conn){
    int c, s, e; string d;
    cout << "Course ID: "; cin >> c;
    cout << "Day (Mon/Tue/Wed/Thu/Fri): "; cin >> d;
    cout << "Start (e.g., 900): "; cin >> s;
    cout << "End (e.g., 1030): "; cin >> e;
    string q = "INSERT INTO timeslots(course_id,day_of_week,start_time,end_time) VALUES("
        + to_string(c) + ",'" + d + "'," + to_string(s) + "," + to_string(e) + ")";
    run_sql(conn, q);
    cout << "Added.\n";
}

void generateSchedule(MYSQL* conn){
    auto courses = loadCourses(conn);
    auto prereq = loadPrereqs(conn);
    auto slots = loadSlots(conn);

    // Build graph & indegree from prereqs
    unordered_map<int, vector<int>> graph;
    unordered_map<int, int> indeg;
    for(const auto& c : courses) indeg[c.id] = 0;
    for(auto& [course, pres] : prereq){
        for(int p : pres){
            graph[p].push_back(course);
            indeg[course]++;
        }
    }

    // Kahn's topological sort
    queue<int> q;
    for(auto& [id, d] : indeg) if(d==0) q.push(id);
    vector<int> topo;
    while(!q.empty()){
        int u = q.front(); q.pop();
        topo.push_back(u);
        for(int v : graph[u]){
            if(--indeg[v]==0) q.push(v);
        }
    }
    if(topo.size() != courses.size()){
        cout << "❌ Cycle detected in prerequisites. Fix prereqs and retry.\n";
        return;
    }

    // Pick non-conflicting slots following topo order
    vector<Slot> chosen;
    auto findSlot=[&](int cid)->optional<Slot>{
        for(const auto& s: slots) if(s.course_id==cid){
            bool ok=true;
            for(const auto& c: chosen) if(conflict(s,c)){ ok=false; break; }
            if(ok) return s;
        }
        return nullopt;
    };

    cout << "\n✅ Recommended Schedule:\n";
    for(int id : topo){
        auto s = findSlot(id);
        string name = "";
        for(const auto& c: courses) if(c.id==id){ name=c.name; break; }
        if(s.has_value()){
            cout << "- " << name << " [" << s->dow << " " << s->start << "-" << s->end << "]\n";
        } else {
            cout << "- " << name << " [No available non-conflicting slot]\n";
        }
    }
}

int main(){
    // Adjust credentials as needed
    const char* host="127.0.0.1";
    const char* user="root";
    const char* pass="password"; // TODO: change
    const char* db  ="course_scheduler";
    unsigned int port = 3306;

    MYSQL* bootstrap = connect_db(host, user, pass, NULL, port);
    // Ensure DB exists
    if(mysql_query(bootstrap, "CREATE DATABASE IF NOT EXISTS course_scheduler;")){
        cerr << "Create DB error: " << mysql_error(bootstrap) << "\n";
    }
    mysql_close(bootstrap);

    MYSQL* conn = connect_db(host, user, pass, db, port);

    int ch;
    while(true){
        cout << "\n=== Course Scheduler (MySQL) ===\n";
        cout << "1. Add Course\n2. Add Prerequisite\n3. Add Timeslot\n4. Generate Schedule\n5. Exit\n> ";
        if(!(cin>>ch)) break;
        if(ch==1) addCourse(conn);
        else if(ch==2) addPrereq(conn);
        else if(ch==3) addSlot(conn);
        else if(ch==4) generateSchedule(conn);
        else if(ch==5) break;
        else cout << "Invalid.\n";
    }

    mysql_close(conn);
    return 0;
}
