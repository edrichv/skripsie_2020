#include "../include/Netlist.h"

Netlist::Netlist(std::vector<std::string> lines) {
	for (int i = 0; i < lines.size(); i++) {
		std::string line = lines[i];
		if (line.find(".TRAN") != std::string::npos) tran_index = i;
		netlist_lines.push_back(line);
	}
}
Netlist::Netlist() {}
std::vector<std::string> Netlist::get_lines() {
	return netlist_lines;
}
void Netlist::parse_file(const char* netlist_path, const char* params_path, LogicTree& lt) {
	std::ifstream ifs(params_path);
	rapidjson::IStreamWrapper isw(ifs);
	rapidjson::Document d;
	d.ParseStream(isw);
	dutname = d["DUTname"].GetString();
	path = netlist_path;

	std::ifstream is(netlist_path);

	bool in_subckt = false;
	std::vector<std::string> component;

LOOP:while (is.good()) {
		std::string line;
		std::getline(is, line);
		if (line.compare("") == 0) goto LOOP;
		reformat_line(line);
		Netlist::netlist_lines.push_back(line);
		std::stringstream ss(line);
		std::string item;
		component.clear();
		while (std::getline(ss, item, ' ')) {
			if (strcmp(item.c_str(), "*") == 0) goto LOOP;
			component.push_back(item);
		}
		if (in_subckt) {
			if (line.find(".ENDS " + dutname) != std::string::npos) in_subckt = false;
			if (component[0].at(0) == 'X') {
				if (component[0].find("CLK") != std::string::npos) continue;
				if (d.HasMember(component[1].c_str())) {
					std::vector<std::string> in;
					std::vector<std::string> out;
					for (int i = 2; i < component.size(); i++) {
						std::string thisparam = d[component[1].c_str()]["params"][i - 2].GetString();
						if (thisparam.compare("in") == 0) in.push_back(component[i]);
						else if (thisparam.compare("out") == 0) out.push_back(component[i]);
					}
					char type = Cell::type_from_name(d[component[1].c_str()]["type"].GetString());
					if (!(type == Cell::type_from_name("SPLIT") || type == Cell::type_from_name("T"))) {
						//nodes_of_interest.insert(nodes_of_interest.end(), in.begin(), in.end());
						nodes_of_interest.insert(nodes_of_interest.end(), out.begin(), out.end());
					}
					lt.add_cell(type, in, out);
				}
				else {
					continue;
				}
			}
			else if (component[0].at(0) == 'T') {
				lt.add_cell(Cell::type_from_name("T"), { component[1] }, { component[3]});
			}
		}
		else if (line.find(".SUBCKT " + dutname) != std::string::npos) {
			std::stringstream ss(line);
			std::string item;
			std::vector<std::string> start;
			std::vector<std::string> end;
			component.clear();
			while (std::getline(ss, item, ' ')) {
				component.push_back(item);
			}
			for (int i = 2; i < component.size(); i++) {
				std::string thisparam = d["DUTparams"][i - 2].GetString();
				if (thisparam.compare("in") == 0) start.push_back(component[i]);
				else if (thisparam.compare("out") == 0) end.push_back(component[i]);
				else if (thisparam.compare("clk") == 0) clknode = component[i];
			}
			nodes_of_interest.insert(nodes_of_interest.end(), clknode);
			nodes_of_interest.insert(nodes_of_interest.end(), end.begin(), end.end());
			nodes_of_interest.insert(nodes_of_interest.end(), start.begin(), start.end());
			lt = LogicTree(start, end);
			in_subckt = true;
		}
		else {
			if (component.size() > 1) {
				if (strcmp(component.at(1).c_str(), dutname.c_str()) == 0) {
					map_subckt_to_main(component, lt, d);
				}
			}
		}
	}
}
void Netlist::reformat_line(std::string& line) {
	trim_spaces(line);
	to_upper_case(line);
}
void Netlist::trim_spaces(std::string& line) {
	std::stringstream ss;
	char prevchar = ' ';
	for (int i = 0; i < line.length(); i++) {
		char thischar = line.at(i);
		if (thischar == '\t') {
			thischar = ' ';
			ss << thischar;
		}
		else if (prevchar == ' ' && thischar == ' ');
		else if (thischar != '\n' && thischar != '\r') ss << thischar;
		prevchar = thischar;
	}
	line = ss.str();
}
void Netlist::to_upper_case(std::string& line) {
	std::transform(line.begin(), line.end(), line.begin(), ::toupper);
}
std::string Netlist::get_print_statement(std::string node) {
	std::string ps = ".PRINT ";
	if (subckt_to_main.find(node) != subckt_to_main.end()) { //exists in map
		ps += "NODEP " + subckt_to_main.at(node);
	}
	else {
		ps += "P(" + node + "." + "X" + dutname + ")";
	}
	return ps;
}
std::unordered_map<std::string, std::string> Netlist::gets2m() {
	return subckt_to_main;
}
std::vector<std::string> Netlist::get_noi() {
	return nodes_of_interest;
}
void Netlist::map_subckt_to_main(std::vector<std::string>& dutcomponent, LogicTree& lt, rapidjson::Document& d) {
	int j = 0;
	int k = 0;
	std::vector<std::string> start_nodes = lt.get_start_nodes();
	std::vector<std::string> end_nodes = lt.get_stop_nodes();
	for (int i = 2; i < dutcomponent.size(); i++) {
		std::string thisparam = d["DUTparams"][i - 2].GetString();
		if (thisparam.compare("in") == 0) subckt_to_main.insert_or_assign(start_nodes[j++], dutcomponent[i]);
		else if (thisparam.compare("out") == 0)  subckt_to_main.insert_or_assign(end_nodes[k++], dutcomponent[i]);
		else if (thisparam.compare("clk") == 0) subckt_to_main.insert_or_assign(clknode, dutcomponent[i]);
	}
}
void Netlist::add_line(std::string line) {
	netlist_lines.push_back(line);
}
void Netlist::write_to_file(std::string path) {
	std::ofstream out(path);
	for (std::string line : netlist_lines) {
		out << line << std::endl;
	}
}
void Netlist::set_tran(int len, float step) {
	std::stringstream ss(".TRAN ");
	ss << step << "p " << len << "p";
	netlist_lines[tran_index] = ss.str();
}