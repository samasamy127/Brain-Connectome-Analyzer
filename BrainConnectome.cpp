#include "BrainGraph.h"
#include "LinkedQueue.h"
#include "LinkedStack.h"
#include "priorityQueue.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <numeric>

using namespace std;

constexpr double BrainGraph::INF;

BrainGraph::BrainGraph()
    : adj(N), origAdj(N),
      weightedindegree(N, 0.0), weightedoutdegree(N, 0.0), betweennessCentrality(N, 0.0),
      centrComputed(false)
{
    for (int i = 0; i < N; i++)
        dead[i] = false;
}

string BrainGraph::fmtDouble(double v, int prec) const {
    ostringstream oss;
    oss << fixed << setprecision(prec) << v;
    return oss.str();
}

vector<int> BrainGraph::tracePath(int source, int destination, const vector<int>& prev) const {
    LinkedStack<int> stack1;
    vector<bool> visited(N, false);
    int current = destination;

    while (current != -1 && current >= 0 && current < N && !visited[current]) {
        visited[current] = true;
        stack1.push(current);
        if (current == source)
            break;
        current = prev[current];
    }

    if (stack1.isEmpty())
        return {};

    int top = stack1.peek();
    if (top != source)
        return {};

    vector<int> path;
    int value;
    while (!stack1.isEmpty()) {
        path.push_back(stack1.peek());
        stack1.pop(value);
    }
    return path;
}

bool BrainGraph::loadLabels(const string& path) {
    ifstream f(path);
    if (!f.is_open()) {
        cerr << "Cannot open labels file: " << path << endl;
        return false;
    }
    string line;
    getline(f, line);
    while (getline(f, line)) {
        if (line.empty()) continue;
        size_t sep = line.find(';');
        if (sep == string::npos) continue;
        int roiNum = stoi(line.substr(0, sep));
        string roiName = line.substr(sep + 1);
        while (!roiName.empty() &&
            (roiName.back() == '\r' || roiName.back() == '\n' || roiName.back() == ' '))
            roiName.pop_back();
        int i = roiNum - 1;
        nameToIdx[roiName] = i;
        idxToName[i] = roiName;
    }
    cout << "Loaded " << nameToIdx.size() << " brain region labels" << endl;
    return true;
}

bool BrainGraph::loadMatrix(const string& path) {
    ifstream f(path);
    int row = 0;
    int edgeCount = 0;
    string line;
    while (getline(f, line) && row < N) {
        if (line.empty()) continue;
        istringstream ss(line);
        string token;
        int col = 0;
        while (getline(ss, token, ',') && col < N) {
            double w = stod(token);
            if (w > 0.0) {
                adj[row].push_back({ col, w });
                edgeCount++;
            }
            col++;
        }
        row++;
    }
    origAdj = adj;
    cout << "connectivity matrix: " << edgeCount << " directed edges" << endl;
    return true;
}

string BrainGraph::name(int i) const {
    unordered_map<int, string>::const_iterator it = idxToName.find(i);
    if (it != idxToName.end())
        return it->second;
    return "Unknown";
}

int BrainGraph::idx(const string& regionName) const {
    unordered_map<string, int>::const_iterator it = nameToIdx.find(regionName);
    if (it != nameToIdx.end())
        return it->second;
    return -1;
}

pair<vector<double>, vector<int>> BrainGraph::dijkstra(int source) const {
    vector<double> dist(N, INF);
    vector<int> prev(N, -1);

    if (source < 0 || source >= N || dead[source])
        return { dist, prev };

    using P = pair<double, int>;
    priorityQueue<P> pq;
    dist[source] = 0.0;
    pq.push({ 0.0, source });

    while (!pq.empty()) {
        P top = pq.peek();
        double d = top.first;
        int u = top.second;
        pq.pop();

        if (d > dist[u]) continue;

        for (const Edge& e : adj[u]) {
            if (dead[e.target] || e.weight <= 0.0) continue;
            double cost = 1.0 / e.weight;
            double newDist = dist[u] + cost;
            if (newDist < dist[e.target]) {
                dist[e.target] = newDist;
                prev[e.target] = u;
                pq.push({ newDist, e.target });
            }
        }
    }
    return { dist, prev };
}

pair<vector<int>, int> BrainGraph::bfs(int source, int destination) const {
    if (source < 0 || source >= N || destination < 0 || destination >= N)
        return { {}, -1 };
    if (dead[source] || dead[destination])
        return { {}, -1 };

    vector<bool> visited(N, false);
    vector<int> prev(N, -1);
    LinkedQueue<int> queue;

    visited[source] = true;
    queue.enqueue(source);

    while (!queue.isEmpty()) {
        int u;
        queue.dequeue(u);
        if (u == destination) break;
        for (const Edge& e : adj[u]) {
            if (dead[e.target] || visited[e.target]) continue;
            visited[e.target] = true;
            prev[e.target] = u;
            queue.enqueue(e.target);
        }
    }

    if (!visited[destination])
        return { {}, -1 };

    vector<int> path = tracePath(source, destination, prev);
    return { path, (int)(path.size() - 1) };
}

pair<double, double> BrainGraph::computePathStats(const vector<int>& path) const {
    double totalWeight = 0.0;
    double totalCost = 0.0;
    for (int i = 0; i + 1 < (int)path.size(); i++) {
        int from = path[i];
        int to = path[i + 1];
        for (const Edge& e : adj[from]) {
            if (e.target == to) {
                totalWeight += e.weight;
                totalCost += 1.0 / e.weight;
                break;
            }
        }
    }
    return { totalWeight, totalCost };
}

void BrainGraph::buildCentrality() {
    weightedindegree.assign(N, 0.0);
    weightedoutdegree.assign(N, 0.0);
    betweennessCentrality.assign(N, 0.0);

    for (int i = 0; i < N; i++) {
        if (dead[i]) continue;
        for (const Edge& e : adj[i]) {
            if (dead[e.target]) continue;
            weightedoutdegree[i] += e.weight;
            weightedindegree[e.target] += e.weight;
        }
    }

    cout << "Computing betweenness centrality..." << endl;
    for (int s = 0; s < N; s++) {
        if (dead[s]) continue;
        vector<double> dist;
        vector<int> prev;
        tie(dist, prev) = dijkstra(s);
        for (int t = 0; t < N; t++) {
            if (t == s || dead[t] || dist[t] == INF) continue;
            int cur = prev[t];
            while (cur != -1 && cur != s) {
                betweennessCentrality[cur] += 1.0;
                cur = prev[cur];
            }
        }
    }
    centrComputed = true;
    cout << "Centrality computation complete" << endl;
}

void BrainGraph::printRankTable(const string& title, const string& metric,
    const vector<int>& sortedNodes, const vector<double>& scores, int topN) const {
    cout << endl << "Top " << topN << " by " << title << endl;
    cout << left << setw(5) << "Rank" << setw(36) << "Region" << metric << endl;
    cout << string(60, '-') << endl;
    int rank = 1;
    for (int i = 0; i < (int)sortedNodes.size(); i++) {
        int node = sortedNodes[i];
        if (dead[node]) continue;
        cout << left << setw(5) << rank << setw(36) << name(node)
            << fixed << setprecision(4) << scores[node] << endl;
        rank++;
        if (rank > topN) break;
    }
}

void BrainGraph::hubReport(int topN) const {
    if (!centrComputed) {
        cout << " Run option 4 (Build centrality) first" << endl;
        return;
    }

    vector<int> order(N);
    iota(order.begin(), order.end(), 0);

    cout << "      HUB REPORT      " << endl;

    vector<int> byBetweenness = order;
    sort(byBetweenness.begin(), byBetweenness.end(),
        [&](int a, int b) { return betweennessCentrality[a] > betweennessCentrality[b]; });
    printRankTable("BETWEENNESS CENTRALITY", "Betweenness", byBetweenness, betweennessCentrality, topN);

    vector<int> byOut = order;
    sort(byOut.begin(), byOut.end(),
        [&](int a, int b) { return weightedoutdegree[a] > weightedoutdegree[b]; });
    printRankTable("WEIGHTED OUT-DEGREE (strong senders)", "Out-Weight", byOut, weightedoutdegree, topN);

    vector<int> byIn = order;
    sort(byIn.begin(), byIn.end(),
        [&](int a, int b) { return weightedindegree[a] > weightedindegree[b]; });
    printRankTable("WEIGHTED IN-DEGREE (strong receivers)", "In-Weight", byIn, weightedindegree, topN);
}

void BrainGraph::lesionNode(int i) {
    if (i < 0 || i >= N) return;
    dead[i] = true;
    centrComputed = false;
    cout << "Lesioned node: [" << i + 1 << "] " << name(i) << endl;
}

void BrainGraph::partialLesion(int i, double factor) {
    if (i < 0 || i >= N || dead[i]) return;
    for (Edge& e : adj[i])
        e.weight *= factor;
    for (int j = 0; j < N; j++) {
        if (dead[j]) continue;
        for (Edge& e : adj[j])
            if (e.target == i) e.weight *= factor;
    }
    centrComputed = false;
    cout << "Partial lesion applied to [" << i + 1 << "] " << name(i)
        << "  (factor=" << factor << ")" << endl;
}

void BrainGraph::restore(int i) {
    if (i < 0 || i >= N) return;
    dead[i] = false;
    adj[i] = origAdj[i];
    for (int j = 0; j < N; j++) {
        for (Edge& e : adj[j]) {
            if (e.target == i) {
                for (const Edge& oe : origAdj[j]) {
                    if (oe.target == i) {
                        e.weight = oe.weight;
                        break;
                    }
                }
            }
        }
    }
    centrComputed = false;
    cout << "Restored: [" << i + 1 << "] " << name(i) << endl;
}

void BrainGraph::restoreAll() {
    for (int i = 0; i < N; i++)
        dead[i] = false;
    adj = origAdj;
    centrComputed = false;
    cout << "All nodes restored" << endl;
}

void BrainGraph::compareSignalPaths(int source, int destination) const {
    if (source < 0 || source >= N || destination < 0 || destination >= N) {
        cout << "Invalid node indices" << endl;
        return;
    }

    cout << "  SIGNAL PATH: " << left << setw(47)
        << (name(source) + " -> " + name(destination)) << endl;

    vector<double> dist;
    vector<int> prev;
    tie(dist, prev) = dijkstra(source);
    vector<int> dijPath = tracePath(source, destination, prev);

    cout << endl << "[DIJKSTRA]" << endl;
    if (dijPath.empty() || dist[destination] == INF) {
        cout << "  No path found (region unreachable after lesion)" << endl;
    }
    else {
        cout << "  Path: ";
        for (int i = 0; i < (int)dijPath.size(); i++) {
            if (i) cout << " -> ";
            cout << name(dijPath[i]);
        }
        pair<double, double> stats = computePathStats(dijPath);
        double dijTotalW = stats.first;
        cout << endl << "  Hops: " << (int)(dijPath.size() - 1)
            << "   Total cost (sum 1/w): " << fmtDouble(dist[destination])
            << "   Total strength (sum w): " << fmtDouble(dijTotalW) << endl;
    }

    vector<int> bfsPath;
    int hops;
    tie(bfsPath, hops) = bfs(source, destination);

    cout << endl << "[BFS]" << endl;
    if (bfsPath.empty() || hops < 0) {
        cout << "  No path found (region unreachable)." << endl;
    }
    else {
        cout << "  Path: ";
        for (int i = 0; i < (int)bfsPath.size(); i++) {
            if (i) cout << " -> ";
            cout << name(bfsPath[i]);
        }
        pair<double, double> bfsStats = computePathStats(bfsPath);
        double bfsW    = bfsStats.first;
        double bfsCost = bfsStats.second;
        cout << endl << "  Hops: " << hops
            << "   Total cost (sum 1/w): " << fmtDouble(bfsCost)
            << "   Total strength (sum w): " << fmtDouble(bfsW) << endl;
    }

    cout << "[COMPARISON TABLE]" << endl;
    cout << left << setw(28) << "Metric" << setw(20) << "Dijkstra" << setw(20) << "BFS" << endl;
    cout << string(68, '-') << endl;

    string dijHopStr = dijPath.empty() ? "N/A" : to_string((int)dijPath.size() - 1);
    string bfsHopStr = (hops < 0) ? "N/A" : to_string(hops);
    cout << setw(28) << "Hops" << setw(20) << dijHopStr << bfsHopStr << endl;

    string dijCostStr = (dist[destination] == INF) ? "N/A" : fmtDouble(dist[destination]);
    string bfsCostStr = "N/A";
    if (!bfsPath.empty() && hops >= 0) {
        pair<double, double> bfsStats2 = computePathStats(bfsPath);
        bfsCostStr = fmtDouble(bfsStats2.second);
    }
    cout << setw(28) << "Total cost (sum 1/w)" << setw(20) << dijCostStr << bfsCostStr << endl;

    string same = (dijPath == bfsPath) ? "Yes" : "No";
    cout << setw(28) << "Paths identical" << same << endl;
    cout << string(68, '-') << endl;
 }
void BrainGraph::reachabilityReport(int source) const {
    vector<double> dist;
    vector<int> prev;
    tie(dist, prev) = dijkstra(source);

    int reachable = 0;
    int unreachable = 0;
    vector<string> unreachableNames;

    for (int i = 0; i < N; i++) {
        if (i == source || dead[i]) continue;
        if (dist[i] < INF)
            reachable++;
        else {
            unreachable++;
            unreachableNames.push_back(name(i));
        }
    }

    cout << endl << " REACHABILITY FROM: " << name(source) << endl;
    cout << "Reachable  : " << reachable << endl;
    cout << "Unreachable: " << unreachable << endl;
    if (!unreachableNames.empty()) {
        cout << "Disconnected regions:" << endl;
        for (int i = 0; i < (int)unreachableNames.size(); i++)
            cout << "  - " << unreachableNames[i] << endl;
    }
}

void BrainGraph::writeDot(const string& file,
    const vector<int>& highlight, const string& title) const {
    ofstream f(file);
    if (!f.is_open()) {
        cerr << "Cannot write: " << file << endl;
        return;
    }

    unordered_map<int, int> highlightPositions;
    for (int i = 0; i < (int)highlight.size(); i++)
        highlightPositions[highlight[i]] = i;

    f << "digraph \"" << title << "\" {\n"
        << "  layout=neato\n"
        << "  overlap=false\n"
        << "  node [shape=ellipse, style=filled, fontsize=11, width=1.2]\n"
        << "  edge [penwidth=0.3, color=gray80]\n";

    for (int i = 0; i < N; i++) {
        string color = "lightblue";
        if (dead[i]) {
            color = "tomato";
        }
        else if (highlightPositions.count(i)) {
            int pos = highlightPositions[i];
            if (pos == 0)
                color = "limegreen";
            else if (pos == (int)highlight.size() - 1)
                color = "orange";
            else
                color = "yellow";
        }
        f << "  " << i << " [label=\"" << name(i)
            << "\", fillcolor=" << color << "];\n";
    }
    f << "\n";

    const double threshold = 0.05;
    for (int i = 0; i < N; i++) {
        if (dead[i]) continue;
        for (const Edge& e : adj[i]) {
            if (dead[e.target]) continue;

            bool hl = false;
            unordered_map<int, int>::const_iterator it = highlightPositions.find(i);
            if (it != highlightPositions.end()) {
                int pos = it->second;
                if (pos + 1 < (int)highlight.size() && highlight[pos + 1] == e.target)
                    hl = true;
            }

            if (!hl && e.weight < threshold) continue;
            f << "  " << i << " -> " << e.target
                << " [weight=" << fixed << setprecision(4) << e.weight;
            if (hl) f << ", color=blue, penwidth=2.5";
            f << "];\n";
        }
    }
    f << "}\n";
    cout << "Exported: " << file << endl;
    cout << "  Render: dot -Tpng " << file << " -o connectome.png" << endl;
}

void BrainGraph::printOrigAdjList() const {
    cout << "   ORIGINAL ADJACENCY LIST  (90 nodes)     " << endl;
    int totalEdges = 0;
    for (int i = 0; i < N; i++) {
        if (origAdj[i].empty()) continue;
        cout << "\n[" << setw(2) << (i + 1) << "] " << name(i) << "\n";
        for (const Edge& e : origAdj[i]) {
            cout << "      -> [" << setw(2) << (e.target + 1) << "] "
                << left << setw(36) << name(e.target)
                << "w=" << fixed << setprecision(6) << e.weight << "\n";
            totalEdges++;
        }
    }
    cout << "\nTotal: " << totalEdges << " directed edges.\n";
}

int BrainGraph::promptNode(const string& prompt) const {
    cout << prompt;
    string input;
    getline(cin, input);

    while (!input.empty() && (input.back() == ' ' || input.back() == '\r'))
        input.pop_back();
    while (!input.empty() && input.front() == ' ')
        input.erase(input.begin());

    try {
        int i = stoi(input) - 1;
        if (i >= 0 && i < N) return i;
    }
    catch (...) {}

    int i = idx(input);
    if (i != -1) return i;
    cout << " Region not found: \"" << input << "\"" << endl;
    return -1;
}

void BrainGraph::shell() {
    int persistsource = -1;
    int persistdestination = -1;

    cout << "  Brain Connectome Analyzer  " << endl;
    cout << " enter region by name or number (1-90)" << endl;

    persistsource      = promptNode("Set default SOURCE region (name or 1-90): ");
    persistdestination = promptNode("Set default DESTINATION region (name or 1-90): ");
    cout << "  Default path: " << name(persistsource)
        << " -> " << name(persistdestination) << "\n";

    while (true) {
        cout << " MENU:" << endl;
        cout << "  1. Find signal path (Dijkstra)" << endl;
        cout << "  2. Find BFS path" << endl;
        cout << "  3. Compare Dijkstra vs BFS" << endl;
        cout << "  4. Build & show hub report" << endl;
        cout << "  5. Simulate full lesion" << endl;
        cout << "  6. Simulate partial lesion" << endl;
        cout << "  7. Restore a node" << endl;
        cout << "  8. Restore all nodes" << endl;
        cout << "  9. Reachability check after lesion" << endl;
        cout << " 10. Export .dot visualization" << endl;
        cout << " 11. Show original adjacency list" << endl;
        cout << " 12. Visualize Dijkstra path (full graph)" << endl;
        cout << " 13. Change default source/destination" << endl;
        cout << "  0. Quit" << endl;
        cout << "Choice: ";

        int choice;
        if (!(cin >> choice))
            break;
        cin.ignore();

        switch (choice) {

        case 0:
            cout << "Goodbye." << endl;
            return;

        case 1: {
            int s = persistsource;
            int d = persistdestination;
            if (s < 0 || d < 0) {
                cout << "No source/destination set." << endl;
                break;
            }
            vector<double> dist;
            vector<int> prev;
            tie(dist, prev) = dijkstra(s);
            vector<int> path = tracePath(s, d, prev);
            if (path.empty() || dist[d] == INF) {
                cout << "No path found." << endl;
            }
            else {
                cout << "Path: ";
                for (int i = 0; i < (int)path.size(); i++) {
                    if (i) cout << " -> ";
                    cout << name(path[i]);
                }
                cout << endl << "Hops: " << (int)path.size() - 1
                    << "  Cost: " << fmtDouble(dist[d]) << endl;
            }
            break;
        }

        case 2: {
            int s = persistsource;
            int d = persistdestination;
            if (s < 0 || d < 0) {
                cout << "No source/destination set" << endl;
                break;
            }
            vector<int> path;
            int hops;
            tie(path, hops) = bfs(s, d);
            if (path.empty() || hops < 0) {
                cout << "No path found" << endl;
            }
            else {
                cout << "Path: ";
                for (int i = 0; i < (int)path.size(); i++) {
                    if (i) cout << " -> ";
                    cout << name(path[i]);
                }
                cout << endl << "Hops: " << hops << endl;
            }
            break;
        }

        case 3: {
            int s = persistsource;
            int d = persistdestination;
            if (s < 0 || d < 0) {
                cout << "No source/destination set" << endl;
                break;
            }
            compareSignalPaths(s, d);
            break;
        }

        case 4:
            buildCentrality();
            hubReport(10);
            break;

        case 5: {
            int n = promptNode("Region to lesion (name or 1-90): ");
            if (n < 0) break;
            lesionNode(n);
            break;
        }

        case 6: {
            int n = promptNode("Region to partially lesion (name or 1-90): ");
            if (n < 0) break;
            cout << "Reduction factor (e.g. 0.3 keeps 30% of strength): ";
            double factor;
            cin >> factor;
            cin.ignore();
            partialLesion(n, factor);
            break;
        }

        case 7: {
            int n = promptNode("Region to restore (name or 1-90): ");
            if (n < 0) break;
            restore(n);
            break;
        }

        case 8:
            restoreAll();
            break;

        case 9: {
            int s = persistsource;
            int d = persistdestination;
            if (s < 0 || d < 0) {
                cout << "No source/destination set" << endl;
                break;
            }
            vector<double> dist;
            vector<int> prev;
            tie(dist, prev) = dijkstra(s);
            vector<int> path = tracePath(s, d, prev);

            if (path.empty()) {
                cout << endl << "REACHABILITY: " << name(s) << " -> " << name(d) << endl;
                cout << "  Destination is already UNREACHABLE (no path exists)" << endl;
                break;
            }

            cout << endl << " REACHABILITY ALONG PATH: "
                << name(s) << " -> " << name(d) << endl;
            cout << "Path has " << (int)(path.size() - 1) << " hops through "
                << (int)path.size() << " regions" << endl;

            int reachable = 0;
            int unreachable = 0;
            for (int i = 0; i < (int)path.size(); i++) {
                int node = path[i];
                string status;
                string role;

                if (i == 0)
                    role = "[SOURCE]      ";
                else if (i == (int)(path.size() - 1))
                    role = "[DESTINATION] ";
                else
                    role = "[INTERMEDIATE]";

                if (dead[node]) {
                    status = "DEAD (lesioned)";
                    unreachable++;
                }
                else if (dist[node] < INF) {
                    status = "REACHABLE";
                    reachable++;
                }
                else {
                    status = "UNREACHABLE";
                    unreachable++;
                }

                cout << "  " << role << " [" << setw(2) << node + 1 << "] "
                    << left << setw(34) << name(node) << status << endl;

                if (i + 1 < (int)path.size()) {
                    int next = path[i + 1];
                    double w = 0.0;
                    for (const Edge& e : adj[node])
                        if (e.target == next) { w = e.weight; break; }
                    cout << "        ->  connection strength: "
                        << fixed << setprecision(4) << w << endl;
                }
            }

            cout << endl << "Summary:" << endl;
            cout << "  Reachable nodes on path  : " << reachable << endl;
            cout << "  Unreachable/dead on path : " << unreachable << endl;
            if (unreachable > 0)
                cout << "  Path is BROKEN - signal cannot complete the journey" << endl;
            else
                cout << "  Path is INTACT - signal can travel end to end." << endl;
            break;
        }

        case 10: {
            int s = persistsource;
            vector<int> hlPath;
            if (s >= 0 && persistdestination >= 0) {
                vector<double> dist;
                vector<int> prev;
                tie(dist, prev) = dijkstra(s);
                hlPath = tracePath(s, persistdestination, prev);
                if (hlPath.empty())
                    cout << "  (no path found - exporting graph without highlight)" << endl;
            }
            cout << "Output filename [connectome.dot]: ";
            string fname;
            getline(cin, fname);
            if (fname.empty())
                fname = "connectome.dot";
            writeDot(fname, hlPath, "Brain Connectome S001");
            break;
        }

        case 11:
            printOrigAdjList();
            break;

        case 12: {
            int s = persistsource;
            int d = persistdestination;
            if (s < 0 || d < 0) {
                cout << "Invalid regions." << endl;
                break;
            }
            vector<double> dist;
            vector<int> prev;
            tie(dist, prev) = dijkstra(s);
            vector<int> path = tracePath(s, d, prev);
            if (path.empty())
                cout << "  No path found between regions" << endl;
            else
                cout << "  Path found with " << (int)(path.size() - 1) << " hops" << endl;
            writeDot("dijkstra_full.dot", path, "Dijkstra Path - Full Connectome");
            break;
        }

        case 13: {
            persistsource      = promptNode("New SOURCE region (name or 1-90): ");
            persistdestination = promptNode("New DESTINATION region (name or 1-90): ");
            cout << "  Updated path: " << name(persistsource)
                << " -> " << name(persistdestination) << endl;
            break;
        }

        default:
            cout << "Invalid choice" << endl;
        }
    }
}

int main() {
    BrainGraph bg;

    cout << "    Brain Connectome Analysis System (AAL-90)    " << endl;

    if (!bg.loadLabels("AAL_regions.csv")) {
        cerr << "Failed to load labels. Exiting." << endl;
        return 1;
    }

    if (!bg.loadMatrix("S001.csv")) {
        cerr << "Failed to load connectivity matrix. Exiting." << endl;
        return 1;
    }

    cout << "\nData loaded successfully. Entering interactive shell" << endl;
    bg.shell();

    return 0;
}
