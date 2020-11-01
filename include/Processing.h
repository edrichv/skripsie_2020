#pragma once
#ifndef PROCESSING_H
#define PROCESSING_H

#include <vector>
#include <string>
#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>
#include <fstream>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <unordered_map>
#include "CliOptions.h"

constexpr float pi = 3.14159265358979;

class Processing
{
private:
	double flux_ratio = 0.4;
	double phi0 = 2.07e-15;
	int resolve_tol = 5;
	double pulse_width = 10e-12;
	double trans_analysis_time = 1e-9;
	double input_delay = 25e-12;
	double wavefile_timestep = 5e-12;
	std::string high_amplitude = "600";
	double integrator_width = pulse_width;
	std::vector<std::vector<float>> data;
	std::vector<int> clock_pulses;
	std::vector<std::vector<int>> test_pattern;
	int tp_inputs = 0;
	double clock_period = 0;
	int clock_width = 0;
	std::unordered_map<std::string, int> cw_delay;
	std::string josimCommand = "";
	std::vector<std::string> tp_nodes;
public:
	std::vector<std::vector<int>> get_test_pattern();
	Processing(double fr, double p0, int res_tol, double pw, double mod);
	Processing(std::string param_path);
	std::vector<int> pulse_detect(int signal_index);
	void printInputWaves(std::vector<std::vector<int>> test_pattern, int num_tests);
	std::vector<std::vector<float>> transpose(std::vector<std::vector<float>> a);
	void process_params(CliOptions cli_options);
	int get_data_length();
	float getTimeStep();
	void run_josim();
	std::vector<std::vector<float>> csv2matrix(std::string file_path);
	std::vector<std::string> get_tp_nodes();
	int get_clock_window(float pulse_event);
	void set_cw_for_node(std::string node, float pulse_event);
	std::vector<int> phase_pulse_detect(int signal_index);
	void adjust_cw();
};

#endif

