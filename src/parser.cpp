#include "parser.h"
using namespace std;
    

void parseNode(const string& node_file) {
    /*
    std::cout << "start parse node..." ;

    //loading_file parameter
    ifstream fin;
    stringstream ss;
    string in_line;
    string in_trash;

    //opening file and pass file discription
    fin.open(node_file);
    if (!fin) {
        cerr << "Error opening file: " << node_file << endl;
    }
    for(int i = 0;i<4;i++) getline(fin, in_line);
    ss.clear(); getline(fin, in_line); ss.str(in_line);
    ss >> in_trash >> in_trash >> num_node;
    ss.clear(); getline(fin, in_line); ss.str(in_line);
    ss >> in_trash >> in_trash >> num_terminal;



    //parsering node info
    while (getline(fin, in_line)) {
        string node_name,termi_condi;
        prec wid, height;

        if (in_line.empty() || in_line[0] == '#') continue;
        ss.clear(); ss.str(in_line); 
       
        ss >> node_name;
        ss >> wid;
        ss >> height;

        if (ss >> termi_condi) {
            PAD* in_pad = new PAD;
            in_pad->name = node_name;
            in_pad->W_H.x = wid;
            in_pad->W_H.y = height;
            if (termi_condi == "terminal") {
                //-------------------------------------------------------
                // don't know how to set pad to fixed
                //-------------------------------------------------------
            }
            else if (termi_condi == "terminal_NI")
            {
                in_pad->is_cover = true;
            }
            pad_map[node_name] = in_pad;
        }
        else {
            if (PARSER_NODE_MODE == 1) continue;
            MODULE* in_module = new MODULE;
            in_module->name = node_name;
            in_module->ori_W_H.x = wid;
            in_module->ori_W_H.y = height;
            in_module->ori_area = wid * height;  // Calculate ori_area
            in_module->is_std = true;
            module_map[node_name] = in_module;
        }
    }
    cout << "finished" << endl;
    return;
    */


    cout << "[Node]Start parse node..." << endl;
    ifstream infile(node_file);
    if (!infile.is_open())
    {
        cerr << "Error opening file.\n";
    }
    string line;
    getline(infile, line);
    // unsigned int numSoft = 0, numHard = 0, numTerm = 0;
    unsigned int numNodes = 0, numTerminals = 0;

    while (getline(infile, line))
    {
        if (line.empty() || line[0] == '#')
            continue;
        if (line.find("NumNodes") != string::npos)
        {
            istringstream iss(line);
            string tmp;
            iss >> tmp >> tmp >> numNodes;
            // cout << numNodes << endl;
            continue;
        }
        else if (line.find("NumTerminals") != string::npos)
        {
            istringstream iss(line);
            string tmp;
            iss >> tmp >> tmp >> numTerminals;
            // cout << numTerminals << endl;
            continue;
        } // break;  // done with header
        istringstream iss(line);
        string tmp, name;
        double w, h;
        // if (!(iss >> tmp >> tmp >> tmp)) continue;
        iss >> name >> w >> h >> tmp;
        // cout << tmp << endl;
        // cout << name;
        // if (name[0] == 'o') {
        MODULE *module = new MODULE;
        module_map[name] = module;
        module_map[name]->name = name;
        module_map[name]->W_H.x = w;
        module_map[name]->W_H.y = h;
        if (tmp == "terminal" || tmp == "terminal_NI")
        {
            module_map[name]->fixed = true;
        }
        // MODULES.push_back(cell);
        // cout << "name: " << module_map[name]->name << " width: " << module_map[name]->W_H.x << " Height: " << module_map[name]->W_H.y  << "Terimal: " << module_map[name]->fixed << endl;

        //}
    }
    cout << "Total Modules: " << module_map.size() << endl;
    cout << "[Node]Finished parse node." << endl;
    return;
}

void parseNet(const string& fname) { // plot small square boundary
    cout << "[Net]Start parse net..." << endl;

    //loading_file parameter
    ifstream fin;
    stringstream ss;
    string in_line;
    string in_trash;

    //opening file and pass file discription
    fin.open(fname);
    if (!fin) {
        cerr << "Error opening file: " << fname << endl;
    }
    for(int i =0;i<4;i++)getline(fin, in_line);
    getline(fin, in_line); 
    ss.clear(); ss.str(in_line); 
    ss >> in_trash >> in_trash >> num_net;
    for (int i = 0; i < 2; i++)getline(fin, in_line);

    prec net_min_x;
    prec net_min_y;
    prec net_max_x;
    prec net_max_y;
    int num_pin=0;
    string in_pin_node_name;
    string in_net_name;
    string pin_type;
    prec in_pin_bias_x;
    prec in_pin_bias_y;
    
    unsigned int net_id = 0;  // For assigning NET IDs

    //start parse net info
    while (getline(fin, in_line)) {
        if (in_line.empty() || in_line[0] == '#') continue;

        ss.clear(); ss.str(in_line);

        ss >> in_trash >> in_trash >> num_pin >> in_net_name;

        net_min_x = PREC_MAX;
        net_min_y = PREC_MAX;
        net_max_x = PREC_MIN;
        net_max_y = PREC_MIN;
        
        // Create NET object for this net
        NET* current_net = new NET();
        current_net->name = in_net_name;
        current_net->id = net_id++;

        for (int i = 0; i < num_pin; i++) {
            getline(fin, in_line); ss.clear(); ss.str(in_line);
            ss >> in_pin_node_name >> pin_type >> in_trash >> in_pin_bias_x >> in_pin_bias_y;

            auto itModule = module_map.find(in_pin_node_name);    // find out module
            auto itPad = pad_map.find(in_pin_node_name);  // find out pad

            prec in_pin_x;
            prec in_pin_y;

            if (itModule != module_map.end()) // in module map
            {
                MODULE* in_module = itModule->second;
                in_pin_x = in_module->center.x + in_pin_bias_x;
                in_pin_y = in_module->center.y + in_pin_bias_y;

                PIN* in_pin = new PIN();
                in_pin->name = pin_type;
                in_pin->offset = FPOS((prec)in_pin_bias_x, (prec)in_pin_bias_y);
                in_pin->module = in_module;
                in_module->pins.push_back(in_pin);
                PINS.push_back(in_pin);
                
                // Link PIN to NET
                current_net->pins.push_back(in_pin);
                
                // Link NET to MODULE (if module doesn't already have this net)
                bool has_net = false;
                for (NET* n : in_module->nets) {
                    if (n == current_net) { has_net = true; break; }
                }
                if (!has_net) {
                    in_module->nets.push_back(current_net);
                }
	    }	
            else if (itPad != pad_map.end()) // in pad map
            {
                PAD* in_pad = itPad->second;
                in_pin_x = in_pad->center.x + in_pin_bias_x;
                in_pin_y = in_pad->center.y + in_pin_bias_y;

            
                PIN* in_pin = new PIN();
                in_pin->name = pin_type;
                in_pin->offset = FPOS((prec)in_pin_bias_x, (prec)in_pin_bias_y);
                in_pin->pad = in_pad;
                in_pad->pins.push_back(in_pin);
                PINS.push_back(in_pin);
                
                // Link PIN to NET
                current_net->pins.push_back(in_pin);
            }
            else {
                if (PARSER_NODE_MODE == 0)
                    cerr << "can't find node in module_map or pad_map" << endl;
            }
            net_min_x = min(net_min_x, in_pin_x);
            net_min_y = min(net_min_y, in_pin_y);
            net_max_x = max(net_max_x, in_pin_x);
            net_max_y = max(net_max_y, in_pin_y);
        }
        
        // Store NET in global NETS vector
        NETS.push_back(current_net);
        
        wHPWL += (net_max_x - net_min_x) + (net_max_y - net_min_y);
        //cout << wHPWL << endl;
    }

    cout << "[Net]Finished parse net." << endl;
    // cout << "HPWL : " << wHPWL << endl;
    // cout << "Total NETs created: " << NETS.size() << endl;
}

void parsePl(const string &fname){
    cout << "[Pl]Start parse pl..." << endl;

    //file parameter
    ifstream fin;
    stringstream ss;
    string in_line;
    string in_trash;

    //opening file and pass the file discription
    fin.open(fname);
    if (!fin) {
        cerr << "Error opening file: " << fname << endl;
    }
    for(int i =0;i<3;i++) getline(fin,in_line);
    

    die_ur_x = chip_ur.x;
    die_ur_y = chip_ur.y;



    // start parsing place info
    string in_node_name, in_orient, in_fixed;
    prec lb_x, lb_y;

    while (getline(fin,in_line)) {
        if (in_line.empty()|| in_line[0]=='#') continue;
        ss.clear(); ss.str(in_line);

        in_fixed = "";
        
        ss >> in_node_name >> lb_x >> lb_y >> in_trash >> in_orient >> in_fixed;
        int in_orientation = -1;
        if (in_orient == "N") in_orientation = 0;
        else if (in_orient == "W") in_orientation = 1;
        else if (in_orient == "S") in_orientation = 2;
        else if (in_orient == "E") in_orientation = 3;
        else if (in_orient == "FN") in_orientation = 4;
        else if (in_orient == "FW") in_orientation = 5;
        else if (in_orient == "FS") in_orientation = 6;
        else if (in_orient == "FE") in_orientation = 7;

        //finding the node and updating
        auto itModule = module_map.find(in_node_name);
        auto itPad = pad_map.find(in_node_name);

        if(itModule != module_map.end()) // in module map
        {
            MODULE* in_module = itModule->second;

            //update module info
            in_module->lb = FPOS((prec)lb_x, (prec)lb_y);
            in_module->orientation = in_orientation;
            in_module->ur.update_ur(in_module->lb, in_module->ori_W_H);
            in_module->center.ur_Center(in_module->lb, in_module->ur);
            if(in_fixed == "/FIXED") in_module->fixed = true;

            die_ur_x = max(die_ur_x, in_module->ur.x);
            die_ur_y = max(die_ur_y, in_module->ur.y);
            die_lb_x = min(die_lb_x, in_module->lb.x);
            die_lb_y = min(die_lb_x, in_module->lb.y);

        }
        else if(itPad != pad_map.end()) // in pad map
        {
            PAD* in_pad = itPad->second;

            //update pad info
            //if(in_fixed == "/FIXED"){
                in_pad->lb = FPOS((prec)lb_x, (prec)lb_y);
                in_pad->orientation = in_orientation;
                in_pad->ur.update_ur(in_pad->lb, in_pad->W_H);
                in_pad->center.ur_Center(in_pad->lb,in_pad->ur);


                die_ur_x = max(die_ur_x, in_pad->ur.x);
                die_ur_y = max(die_ur_y, in_pad->ur.y);
                die_lb_x = min(die_lb_x, in_pad->lb.x);
                die_lb_y = min(die_lb_x, in_pad->lb.y);
            //}
            /*
            else{
                //turn pad to module
                MODULE* to_module = new MODULE;
                to_module->name = in_pad->name;
                to_module->ori_W_H.x = in_pad->W_H.x;
                to_module->ori_W_H.y = in_pad->W_H.y;
                to_module->ori_area = in_pad->W_H.x * in_pad->W_H.y;  // Calculate ori_area
                to_module->lb = FPOS((prec)lb_x, (prec)lb_y);
                to_module->orientation = in_orientation;
                to_module->ur.update_ur(in_pad->lb, in_pad->W_H);
                to_module->center.ur_Center(in_pad->lb, in_pad->ur);
                to_module->is_std = false;

                module_map[in_pad->name] = to_module;
                delete itPad->second;  // delete pad object
                pad_map.erase(itPad);  // delete map entry


                die_ur_x = max(die_ur_x, to_module->ur.x);
                die_ur_y = max(die_ur_y, to_module->ur.y);
                die_lb_x = min(die_lb_x, to_module->lb.x);
                die_lb_y = min(die_lb_x, to_module->lb.y);
            }
            */


        }
        else {   // not in pad_map or moudle_map => incorrect
            if(PARSER_NODE_MODE == 0)
                cerr << "can't find node" <<  in_node_name  <<" in module_map or pad_map" << endl;
        }
    }
    cout << "[Node]Finished parse node." << endl;
}

void parseScl(const string &scl_file){ // plot small square boundary
    cout << "[Scl]Start parse scl..." << endl;

    //loading_file parameter
    ifstream fin;
    stringstream ss;
    string in_line;
    string in_trash;

    fin.open(scl_file);
    if (!fin) {
        cerr << "Error opening file: " << scl_file << endl;
    }

    for(int i =0;i<5;i++){
        getline(fin, in_line);
    }
 
    chip_lb = FPOS(PREC_MAX, PREC_MAX);
    chip_ur = FPOS(PREC_MIN, PREC_MIN);

    string status;
    prec in_coordinate = -1;  //y-coordinate
    prec in_height = -1;    
    prec in_site_width = -1;
    prec in_subrow_origin = -1;    //x-coordinate
    prec in_num_sites = -1;

    //start parse row info
    while (getline(fin, in_line)) {
        if (in_line.empty() || in_line[0] == '#') continue;

        ss.clear(); ss.str(in_line); ss >> status;


        if (status == "CoreRow") {
            getline(fin, in_line); ss.clear(); ss.str(in_line);
            ss >> in_trash >> in_trash >> in_coordinate;
            getline(fin, in_line); ss.clear(); ss.str(in_line);
            ss >> in_trash >> in_trash >> in_height;
            getline(fin, in_line); ss.clear(); ss.str(in_line);
            ss >> in_trash >> in_trash >> in_site_width;
            for (int i = 0; i < 3; i++) getline(fin, in_line);
            getline(fin, in_line); ss.clear(); ss.str(in_line);
            ss >> in_trash >> in_trash >> in_subrow_origin >> in_trash >> in_trash >> in_num_sites;
            getline(fin, in_line); ss.clear(); ss.str(in_line); ss >> status;
            if (status != "End") cerr << "scl format error" << endl;
            
            //-------------------------
            // the part to add row object
            ROW *row = new ROW;
            row->lb.x = in_subrow_origin;
            row->lb.y = in_coordinate;
            row->W_H.x = in_num_sites * in_site_width;
            row->W_H.y = in_height;
            row->NumSites = in_num_sites;
            row->Sitespacing = in_site_width;
            ROWS.push_back(row);

            //-------------------------

            //start update chip range
            if (in_subrow_origin < chip_lb.x) chip_lb.x = in_subrow_origin;
            prec right_edge_tmp = in_subrow_origin + (in_site_width * in_num_sites);
            if (right_edge_tmp > chip_ur.x) chip_ur.x = right_edge_tmp;
            if (in_coordinate < chip_lb.y) chip_lb.y = in_coordinate;
            prec up_edge_tmp = in_coordinate + in_height;
            if (up_edge_tmp > chip_ur.y) chip_ur.y = up_edge_tmp;
        }
        else {
            cerr << "scl format error" << endl;
        }
    }
    //update chip info
    chip_W_H.update_W_H(chip_lb, chip_ur);
    chip_area = chip_W_H.Cal_area();

    cout << "[Scl]Finished parse scl." << endl;
}

void parseAux(const string& aux_path) {

    cout << "[Aux]Parser node mode:" << PARSER_NODE_MODE << endl;

    //loading_file parameter
    ifstream fin;
    stringstream ss;
    string in_line;
    string in_trash;

    //reference files
    string node_file;
    string net_file;
    string pl_file;
    string scl_file;

    //geting prefix & file_name of path
    size_t slash_pos = aux_path.find_last_of('/');
    if (slash_pos == string::npos) {
        cerr << "Invalid path format" << endl;
    }
    size_t dot_pos = aux_path.find_last_of('.');
    if (dot_pos == string::npos) {
        cerr << "Invalid path format" << endl;
    }

    benchmark_dir = aux_path.substr(0, slash_pos + 1);
    benchname = aux_path.substr((slash_pos + 1), (dot_pos - slash_pos-1));

    //parsering the file including in aux
    fin.open(aux_path);
    if (!fin) {
        cerr << "Error: cannot open aux file:" << aux_path << endl;
    }
    ss.clear(); getline(fin, in_line); ss.str(in_line);
    //ss >> in_trash >> in_trash >> node_file >> net_file >> in_trash >> pl_file >> scl_file;
    ss >> in_trash >> in_trash >> node_file >> net_file >> in_trash >> in_trash >> scl_file;
    pl_file = (benchname + ".gp.pl");

    // cout << "[Aux]Input aux:" << aux_path <<
    //     "\ninput_node_file:" << (benchmark_dir + node_file) <<
    //     "\ninput_net_file" << (benchmark_dir + net_file) <<
    //     "\ninput_pl_file" << (benchmark_dir + pl_file) <<
    //     "\ninput_scl_file" << (benchmark_dir + scl_file) << endl;

    parseScl(benchmark_dir + scl_file);
    parseNode(benchmark_dir + node_file);
    parsePl(benchmark_dir + pl_file);
    parseNet(benchmark_dir + net_file);

}


//parse info
int num_net = -1;
unsigned int num_node = 0;
unsigned int num_terminal = 0;
