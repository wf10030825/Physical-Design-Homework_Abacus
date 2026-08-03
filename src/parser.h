#ifndef _PARSER_h_
#define _PARSER_h_

#define PARSER_NODE_MODE 0  //0: all nodes  1: only macro  2: only cell


#include <unordered_map>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <cmath>

#include "datatype.h"


extern int num_net;
extern unsigned int num_node;
extern unsigned int num_terminal;

void parseNode(const string& block_file);
void parseNet(const string& block_file);
void parsePl(const string& block_file);
void parseScl(const string& scl_file);
void parseAux(const string& aux_path);
//void parser_matlab(string matlab_file);

/*
void write_cell_dat(string outpifle);
void write_pad_dat(string outpifle);
void write_pad_pin_dat(string out_file); 
*/
#endif
