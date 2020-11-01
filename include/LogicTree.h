#pragma once
#ifndef LOGICTREE_H
#define LOGICTREE_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <unordered_map>
#include <string>
#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>
#include "rapidjson/pointer.h"
#include <algorithm>

class Cell {
private:
	std::vector<std::string> in_nodes, out_nodes;
	char type;
public:
	static const std::string* types;
	Cell(std::vector<std::string> in, std::vector<std::string> out, char t);
	Cell();
	static char type_from_name(std::string name);
	char getType();
	std::vector<std::string> getInNodes();
	std::vector<std::string> getOutNodes();
};

class LogicTree {
private:
	std::unordered_map<std::string, Cell> data_structure;
	std::unordered_map<std::string, char> node_vals;
	std::vector<std::string> start_nodes, end_nodes;
	std::vector<std::pair<std::string, std::string>> trans_list;
public:
	LogicTree(std::vector<std::string> start, std::vector<std::string> end);
	LogicTree();
	void add_cell(char type, std::vector<std::string> in_nodes, std::vector<std::string> out_nodes);
	void set_ss_nodes(std::vector<std::string> start, std::vector<std::string> end);
	std::vector<std::string> get_start_nodes();
	std::vector<std::string> get_stop_nodes();
	void print_cells(std::string node);
	void propagate_inputs(std::vector<std::string> nodes, std::vector<int> inputs);
	char contains(std::vector<std::string> a, std::string b);
	void bfs(Cell cell);
	void reset_nodes();
	char get_node_val(std::string node);
	std::vector<int> simulate(std::vector<int> inputs, std::vector<std::string> tp_nodes);
};

#endif

