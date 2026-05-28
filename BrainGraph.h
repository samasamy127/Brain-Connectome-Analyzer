#pragma once

#include <vector>
#include <list>
#include <unordered_map>
#include <string>
#include <utility>
#include <limits>

using namespace std;

class BrainGraph {

public:

    static const int N = 90;
    static constexpr double INF = numeric_limits<double>::infinity();

    struct Edge {
        int    target;
        double weight;
    };

private:

    vector<list<Edge>> adj;
    vector<list<Edge>> origAdj;

    unordered_map<string, int> nameToIdx;
    unordered_map<int, string> idxToName;

    bool dead[N];

    vector<double> weightedindegree;
    vector<double> weightedoutdegree;
    vector<double> betweennessCentrality;
    bool           centrComputed;

    vector<int>          tracePath(int source, int destination, const vector<int>& prev) const;
    string               fmtDouble(double value, int precision = 6) const;
    pair<double, double> computePathStats(const vector<int>& path) const;
    void                 printRankTable(const string& title, const string& metric,
                             const vector<int>& sortedNodes, const vector<double>& scores,
                             int topN) const;
    int                  promptNode(const string& prompt) const;

public:

    BrainGraph();

    bool loadLabels(const string& labelsFile);
    bool loadMatrix(const string& matrixFile);

    string name(int nodeIndex)            const;
    int    idx (const string& regionName) const;
    bool   isAlive(int nodeIndex)         const { return !dead[nodeIndex]; }

    pair<vector<double>, vector<int>> dijkstra(int sourceNode) const;
    pair<vector<int>, int>            bfs(int sourceNode, int destNode) const;

    void buildCentrality();
    void hubReport(int topN) const;

    void lesionNode   (int nodeIndex);
    void partialLesion(int nodeIndex, double factor);
    void restore      (int nodeIndex);
    void restoreAll   ();

    void compareSignalPaths(int sourceNode, int destNode) const;
    void reachabilityReport(int sourceNode)               const;
    void writeDot(const string& filename, const vector<int>& highlightPath,
                  const string& title)                    const;

    void printOrigAdjList() const;

    void shell();
};
