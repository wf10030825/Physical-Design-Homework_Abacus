#pragma once

#include <unordered_map>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <cmath>


#define WRITE_PLT 1

#include "datatype.h"
#include "parser.h"
#include "abacus.h"

void writeOutFile();
void writePlOut(const string& fname);
void writLegalPl(const string& fname);

void ensure_parent_dir(const string& filepath);   
   


