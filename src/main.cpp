#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <filesystem>
#include "../include/Netlist.h"
#include "../include/LogicTree.h"
#include "../include/CliOptions.h"
#include "../include/Processing.h"



int main(int argc, char** argv) {

    std::string param_path = "./parse_params.json";

    std::vector<std::string> files_to_remove;
    LogicTree lt;
    Netlist netlist_in, netlist_out;
    CliOptions cliOptions(argc, argv);
    Processing processing("./processing_params.json");

    netlist_in.parse_file(cliOptions.inputNetlistPath, param_path.c_str(), lt);
    netlist_out = Netlist(netlist_in.get_lines());

    for (std::string node : netlist_in.get_noi()) {
        netlist_out.add_line(netlist_in.get_print_statement(node, cliOptions.analysis));
    }
    std::string out_path = cliOptions.inputNetlistPath;
    out_path.insert(out_path.find("."), "_gen");
    processing.process_params(cliOptions);
    std::vector<std::vector<int>> tp = processing.get_test_pattern();
    netlist_out.write_to_file(out_path);
    std::vector<std::string> noi = netlist_in.get_noi();
    std::vector<int> pulseless_node_indices;
    pulseless_node_indices.reserve(noi.size()-1);
    bool all_nodes_found = false;
    bool in_cw_set = false;
    for (int i = 1; i < noi.size(); i++) pulseless_node_indices.push_back(i);

    files_to_remove.push_back(out_path);
    files_to_remove.push_back("data.csv");
    for (int i = 0; i < tp[0].size(); i++) {
        std::stringstream ss;
        ss << "IN" << i;
        files_to_remove.push_back(ss.str());
    }

    for (int i = 0; i < tp.size(); i++) {
        if (all_nodes_found) break;
        else {
            processing.printInputWaves({ tp[i] }, 1);
        }
        std::cout << "Testing pattern ";
        for (int input : tp[i]) {
            std::cout << input << " ";
        }
        std::cout << std::endl << std::endl;
        processing.run_josim(cliOptions);
        lt.simulate(tp[i],processing.get_tp_nodes());

        printf("%-8s%-3s%-6s%-11s", "NODE", "LT", "JOSIM", "EVENT (ps)");
        std::cout << std::endl << std::endl;
        int cnts[2] = {0};
        
        for (int j = 2; j < processing.get_data_length(); j++) {
            std::vector<int> pulses = processing.pulse_detect(j, cliOptions);
            int size = pulses.size();
            int josim_node_state = size;
            int node_val = int(lt.get_node_val(noi[j - 1]));
            //if (node_val == 2) node_val = 0;
            bool passed;
            if (josim_node_state != node_val) {
                cnts[0]++;
                passed = false;
            }
            else {
                passed = true;
                cnts[1]++;
            }
            float pulse_event;
            if (size > 0) {
                pulse_event = ((double)pulses[0]) * processing.getTimeStep() / 1e-12;
                processing.set_cw_for_node(noi[j - 1], pulse_event);
                if (!pulseless_node_indices.empty()) {
                    for (int k = 0; k < pulseless_node_indices.size(); k++) {
                        if (pulseless_node_indices[k] == j - 1) {
                            pulseless_node_indices.erase(pulseless_node_indices.begin() + k);
                            k = pulseless_node_indices.size();
                        }
                    }
                }
            }
            bool should_write = !(passed && cliOptions.verbosity == 0);
            if (should_write) {
                if (size > 0){
                    printf("%-8s%-3d%-6d%-11.2f", noi[j - 1].c_str(), node_val, size, pulse_event);
                }
                else {
                    printf("%-8s%-3d%-6d%-11s", noi[j - 1].c_str(), node_val, size, "NO PULSE");
                }
            }
            if (!passed) {
                std::cout << "FAIL";
            }
            else if (cliOptions.verbosity > 0) {
                std::cout << "PASS";
            }
            if (lt.contains(lt.get_stop_nodes(), noi[j - 1]) && should_write) {
                std::cout << "\tPRIMARY";
            }
            if(should_write) std::cout << std::endl;
        }
        std::cout << std::endl << cnts[1] << "/" << cnts[1] + cnts[0] << " Passed - " << cnts[1] / ((float)(cnts[0] + cnts[1])) * 100 << "%" << std::endl << std::endl;
        if (pulseless_node_indices.empty()) {
            all_nodes_found = true;
            processing.adjust_cw();
        }
    }

    if (cliOptions.verbosity < 2) {
        for (std::string path : files_to_remove) {
            std::filesystem::remove(path);
        }
    }

    return 0;
}