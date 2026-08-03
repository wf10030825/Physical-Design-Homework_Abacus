#include "output.h"

//linux
#include <sys/stat.h>
#include <sys/types.h>

//windows
//#include <direct.h>

using namespace std;

void ensure_parent_dir(const string& filepath) {   
    string dir_path;
    size_t pos = filepath.find_last_of("/");
    if (pos == std::string::npos) {
        cerr << "format error: the filapath should include directory" << endl;
        return;
    }
    dir_path = filepath.substr(0, pos);
    
    size_t start = 0;
    while (true) {
        pos = dir_path.find_first_of("/", start);
        string mother_dir = dir_path.substr(0, pos);

        if (!mother_dir.empty()) mkdir(mother_dir.c_str(),0755);
        if (pos == std::string::npos) {
            mkdir(mother_dir.c_str(), 0755);
            break; 
        }
        start = pos + 1;
    }
}


void writeOutFile() {
    // writePlOut("./benchmarks/" + benchname + "/" + benchname + ".pl_out");
    if(WRITE_PLT){
        writLegalPl("./legal/" + benchname + "/" + benchname + ".legal.pl");
        // writePlt("./plt/" + benchname + "/" + benchname + ".plt");
    }
}

void writLegalPl(const string& fname){
    ensure_parent_dir(fname);
    ofstream fout(fname);
    if (!fout.is_open())
    {
        cerr << "Error: cannot write " << fname << "\n";
        return;
    }

    for (const auto &kv : module_map)
    {
        MODULE *m = kv.second;
        fout << m->name << " " << m->lb.x << " " << m->lb.y << " : ";
        // orientation
        switch (m->orientation)
        {
        case 0:
            fout << "N";
            break;
        case 1:
            fout << "S";
            break;
        case 2:
            fout << "W";
            break;
        case 3:
            fout << "E";
            break;
        case 4:
            fout << "FN";
            break;
        case 5:
            fout << "FW";
            break;
        case 6:
            fout << "FS";
            break;
        case 7:
            fout << "FE";
            break;
        default:
            fout << "N";
            break; // unknown, default to N
        }
        if (m->fixed)
        {
            fout << " /FIXED";
        }
        fout << "\n";
    }

    fout.close();
    cout << "[INFO] Wrote legal placement to " << fname << endl;
}
