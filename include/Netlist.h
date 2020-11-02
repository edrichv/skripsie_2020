#pragma once
#ifndef NETLIST_H
#define NETLIST_H

#include <vector>
#include <unordered_map>
#include <string>
#include <sstream>
#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>
#include "rapidjson/pointer.h"
#include "LogicTree.h"
#include <algorithm>
#include <iostream>
#include <fstream>

class Netlist
{
private:
	std::vector<std::string> netlist_lines;
	std::vector<std::string> nodes_of_interest;
	std::unordered_map<std::string, std::string> subckt_to_main;
	void map_subckt_to_main(std::vector<std::string>& component, LogicTree& lt, rapidjson::Document& d);
	std::string dutname;
	std::string path;
	std::string clknode;
	int tran_index = 0;
public:
	Netlist(std::vector<std::string> lines);
	Netlist();
	void parse_file(const char* netlist_path,const char* params_path, LogicTree& lt);
	void reformat_line(std::string& line);
	void trim_spaces(std::string& line);
	void to_upper_case(std::string& line);
	std::string get_print_statement(std::string node, char analysis);
	std::unordered_map<std::string, std::string> gets2m();
	std::vector<std::string> get_noi();
	void add_line(std::string line);
	std::vector<std::string> get_lines();
	void write_to_file(std::string path);
	void set_tran(int len, float step);
};

#endif

