#ifndef ALGO_H
#define ALGO_H

#include "datatype.h"
#include <list>
#include <vector>
#include <algorithm>
#include <iostream>

using namespace std;

struct Cluster
{
    // vector<MODULE *> modules;
    int start_index; // 在 row->modules 中的起始 index
    int end_index;   // 在 row->modules 中的結束 index

    double wc; // total width of cluster
    double qc; // total empty space in cluster
    double ec; // total extra space in cluster
    double xc; // x coordinate of cluster
    int n_first, n_last;
};
class Abacus
{
public:
    Abacus();
    ~Abacus();

    // void sort();
    void algo();
    void PlaceRow(ROW *row);
    // void AddCell(cluster &C int idx);
    void Collapse(Cluster &C, Cluster &C_before, vector<Cluster> &clusters, double row_min, double row_max);
    void slice_row();

private:
    // int n; // number of nodes
};
#endif