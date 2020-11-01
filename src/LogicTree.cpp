#include "../include/LogicTree.h"

static const std::string types_array[7] = { "AND","OR","NOT","XOR","SPLIT","DFF","T"};
const std::string* Cell::types = types_array;
Cell::Cell(std::vector<std::string> in, std::vector<std::string> out, char t) {
	in_nodes = in;
	out_nodes = out;
	type = t;
}
Cell::Cell() { Cell::type = 0; }
char Cell::type_from_name(std::string name) {
	for (int i = 0; i < 7; i++) {
		if (Cell::types[i].compare(name) == 0) return i;
	}
	return 0;
}
char Cell::getType() {
	return type;
}
std::vector<std::string> Cell::getInNodes() {
	return in_nodes;
}
std::vector<std::string> Cell::getOutNodes() {
	return out_nodes;
}
char LogicTree::get_node_val(std::string node) {
	return node_vals.at(node);
}

LogicTree::LogicTree(std::vector<std::string> start, std::vector<std::string> end) {
	start_nodes = start;
	end_nodes = end;
}
LogicTree::LogicTree() {}

void LogicTree::add_cell(char type, std::vector<std::string> in_nodes, std::vector<std::string> out_nodes) {
	int num_in = in_nodes.size();
	int num_out = out_nodes.size();
	//if ptl
	if (type == 6) {
		trans_list.push_back(std::make_pair(in_nodes[0], out_nodes[0]));
		node_vals.insert_or_assign(in_nodes[0], 2);
		node_vals.insert_or_assign(out_nodes[0], 2);
	}
	else {
		for (std::string node : in_nodes) {
			data_structure.insert({ node, Cell(in_nodes,out_nodes,type) });
		}
		for (std::string node : out_nodes) {
			node_vals.insert_or_assign(node, 2);
		}
	}
}
void LogicTree::set_ss_nodes(std::vector<std::string> start, std::vector<std::string> end) {
	start_nodes = start;
	end_nodes = end;
}
std::vector<std::string> LogicTree::get_start_nodes() {
	return start_nodes;
}
std::vector<std::string> LogicTree::get_stop_nodes() {
	return end_nodes;
}
void LogicTree::print_cells(std::string node) {
	std::cout << "Cell conncted to node " << node << std::endl;
	Cell cell = data_structure.at(node);
	std::cout << "Type: " << Cell::types[cell.getType()] << std::endl;
	std::cout << "Input nodes: ";
	for (std::string node : cell.getInNodes()) {
		std::cout << node << ",";
	}
	std::cout << std::endl << "Output nodes: ";
	for (std::string node : cell.getOutNodes()) {
		std::cout << node << ",";
	}
	std::cout << std::endl << "Output vals: ";
	for (std::string node : cell.getOutNodes()) {
		std::cout << int(node_vals.at(node)) << ",";
	}
	std::cout << std::endl << std::endl;
}
void LogicTree::propagate_inputs(std::vector<std::string> nodes, std::vector<int> inputs) {
	for (int i = 0; i < nodes.size(); i++) {
		node_vals.insert_or_assign(nodes[i], inputs[i]);
	}
	for (std::string node : start_nodes) {
		std::string new_node = node;
		for (std::pair<std::string, std::string> pair : trans_list) {
			if (node.compare(pair.first) == 0) { new_node = pair.second; break; }
			else if (node.compare(pair.second) == 0) { new_node = pair.first; break; }
		}
		node_vals.at(new_node) = node_vals.at(node);
		bfs(data_structure.at(new_node));
	}
}
char LogicTree::contains(std::vector<std::string> a, std::string b) {
	for (std::string c : a) {
		if (c == b) return 1;
	}
	return 0;
}
void LogicTree::bfs(Cell cell) {
	char out = 2;
	std::vector<std::string> in_nodes = cell.getInNodes();
	char in0 = node_vals.at(in_nodes[0]);
	char in1 = 2;
	if (in_nodes.size() > 1) in1 = node_vals.at(in_nodes[1]);
	if (in0 == 2) return;
	switch (cell.getType()) {
	case 0:
		if (in1 == 2) return;
		out = in0 && in1;
		break;
	case 1:
		if (in1 == 2) return;
		out = in0 || in1;
		break;
	case 2:
		if (in0 == 0) out = 1;
		else out = 0;
		break;
	case 3:
		if (in1 == 2) return;
		out = in0 != in1;
		break;
	case 4:
		out = in0;
		break;
	case 5:
		out = in0;
	case 6:
		out = in0;
	default:
		break;
	}
	for (std::string node : cell.getOutNodes()) {
		node_vals.at(node) = out;
		std::string new_node = node;
		for (std::pair<std::string, std::string> pair : trans_list) {
			if (node.compare(pair.first) == 0) { new_node = pair.second; break; }
			else if (node.compare(pair.second) == 0) { new_node = pair.first; break; }
		}
		node_vals.at(new_node) = out;
		if (!contains(end_nodes, new_node)) {
			try {
				bfs(data_structure.at(new_node));
			}
			catch (std::exception e) {

			}
		}
	}
}

void LogicTree::reset_nodes() {
	for (auto kv : data_structure) {
		node_vals.insert_or_assign(kv.first, 2);
	}
	for (auto kv : trans_list) {
		node_vals.insert_or_assign(kv.first, 2);
		node_vals.insert_or_assign(kv.second, 2);
	}
}
std::vector<int> LogicTree::simulate(std::vector<int> inputs, std::vector<std::string> tp_nodes) {
	reset_nodes();
	propagate_inputs(tp_nodes, inputs);
	std::vector<int> output;
	for (std::string node : end_nodes) {
		output.push_back(node_vals.at(node));
	}
	return output;
}
