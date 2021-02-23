#include <queue>
#include "../include/Processing.h"

Processing::Processing(double fr, double p0, int res_tol, double pw, double tat) {
	flux_ratio = fr;
	phi0 = p0;
	resolve_tol = res_tol;
	pulse_width = pw;
	trans_analysis_time = tat;
}
Processing::Processing(std::string param_path) {
    std::ifstream ifs(param_path);
    rapidjson::IStreamWrapper isw(ifs);
    rapidjson::Document d;
    d.ParseStream(isw);

    flux_ratio = d["flux_ratio"].GetDouble();
    phi0 = d["phi0"].GetDouble();
    resolve_tol = d["resolve_tolerance"].GetDouble();
    pulse_width = d["pulse_width"].GetDouble();
    trans_analysis_time = d["trans_analysis_time"].GetDouble();
    integrator_width = d["integrator_width"].GetDouble();
    clock_period = d["clock_period"].GetDouble();
    input_delay = d["input_delay"].GetDouble();
    wavefile_timestep = d["wavefile_timestep"].GetDouble();
    high_amplitude = d["high_amplitude"].GetString();
    phase_ratio = d["phase_ratio"].GetDouble();
}

std::vector<std::vector<int>> Processing::get_test_pattern() {
    return test_pattern;
}

float Processing::getTimeStep() {
    return data[0][1] - data[0][0];
}

void Processing::adjust_cw() {
    int in_cw = 0;
    bool first = true;
    std::vector<std::string> keys;
    for (auto kv : cw_delay) {
        keys.push_back(kv.first);
        if (first) {
            in_cw = kv.second;
            first = false;
        }
        else if (kv.second < in_cw) in_cw = kv.second;
    }
    for (std::string key : keys) {
        cw_delay[key] -= in_cw;
    }
}

int Processing::get_clock_window(float pulse_event) {
    int size = clock_pulses.size();
    float prev_pulse = 0;

    for (int i = 0; i < size; i++) {
        float this_pulse = ((double)clock_pulses[i]) * getTimeStep() / 1e-12;
        if (pulse_event <= this_pulse && pulse_event > prev_pulse) {
            return i;
        }
        prev_pulse = this_pulse;
    };

    return -1;
}

std::vector<int> Processing::phase_pulse_detect(int signal_index) {
    std::vector<float> signal = data[signal_index];
    //float time = data[0][1] - data[0][0];
    std::vector<int> pulses;

    double two_pi = 2 * (double)pi;
    int psize = 0;
    double threshold = phase_ratio * two_pi;
    
    for (int i = 0; i < signal.size(); i++) {
        if (signal[i] >= psize * two_pi + threshold) {
            pulses.push_back(i);
            psize++;
        }
    }
    
    return pulses;
}

void Processing::set_cw_for_node(std::string node, float pulse_event) {
    cw_delay.insert_or_assign(node, get_clock_window(pulse_event));
}

std::vector<int> Processing::pulse_detect(int signal_index, CliOptions& cliOptions) {
    if (cliOptions.analysis == 1) return phase_pulse_detect(signal_index);
    std::vector<int> detect;
    std::vector<float> signal = data[signal_index];
    float time = data[0][1] - data[0][0];
    int width = (int)(integrator_width / time);
    clock_width = (int)(clock_period / time);
    detect.reserve(width + signal.size());
    for (int i = 0; i < width; i++) {
        detect.push_back(0);
    }
    for (int i = 0; i < width; i++) {
        signal.insert(signal.begin(), 0);
    }
    for (int i = 0; i < signal.size() - width; i++) {
        float total = 0;
        for (int j = i+1; j < i + width - 1; j++) {
            total += signal[j];
        }
        total += ((double)signal[i] + signal[i + width - 1]) / 2.0;
        float flux = total * time;
        if (flux / phi0 >= flux_ratio) {
            detect.push_back(1);
        }
        else {
            detect.push_back(0);
        }
    }
    std::vector<int> temp_pulses;
    std::vector<int> pulses;
    for (int i = 0; i < detect.size() - 1; i++) {
        if (detect[i + 1] - detect[i] == 1) {
            temp_pulses.push_back(i + 1 - width);
        }
    }
    if (temp_pulses.size() > 0) {
        pulses.push_back(temp_pulses[0]);
        for (int i = 1; i < temp_pulses.size() - 1; i++) {
            if (temp_pulses[i] - temp_pulses[i - 1] > integrator_width) {
                pulses.push_back(temp_pulses[i]);
            }
        }
    }
    return pulses;
}
void Processing::process_params(CliOptions cli_options) {
    std::ifstream is(cli_options.testPatternPath);

    if (is.good()) {
        std::string line;
        std::getline(is, line);
        std::stringstream ss(line);
        std::string item;
        while (std::getline(ss, item, ',')) {
            tp_inputs++;
            tp_nodes.push_back(item);
        }
    }

    while (is.good()) {
        std::string line;
        std::getline(is, line);
        std::stringstream ss(line);
        std::string item;
        std::vector<int> row;
        while (std::getline(ss, item, ',')) {
            row.push_back(std::stoi(item.c_str()));
        }
        if (row.size() > 0) test_pattern.push_back(row);
    }

    is.close();
    
    /*for (std::vector<int> row : test_pattern) {
        for (char input : row) {
            std::cout << input << ',';
        }
        std::cout << std::endl;
    }*/
    if (cli_options.analysis == 0) {
        josimCommand = "josim -a 0 -o data.csv ";
    }
    else {
        josimCommand = "josim -a 1 -o data.csv ";
    }
    josimCommand.append(" ");
    std::string new_netlist = cli_options.inputNetlistPath;
    josimCommand += new_netlist.insert(new_netlist.find("."),"_gen");
    josimCommand += " > nul";
}
int Processing::get_data_length() {
    return data.size();
}

void Processing::run_josim(CliOptions& clioptions) {
    data.clear();
    std::cout << "Simulating circuit with JoSIM" << std::endl;
    system(josimCommand.c_str());
    std::cout << "Analyzing simulation results" << std::endl << std::endl;
    csv2matrix("data.csv");
    clock_pulses = pulse_detect(1, clioptions);
}

std::vector<std::vector<float>> Processing::csv2matrix(std::string file_path) {
    std::ifstream is;
    is = std::ifstream(file_path);
    std::stringstream buffer;
    buffer << is.rdbuf();
    std::string data_content = buffer.str();
    is.close();

    bool parseable = false;
    char to_parse[32] = { 0 };
    int tp_index = 0;
    std::vector<float> row;

    //much more efficient csv reader
    for (char c : data_content) {
        if (parseable) {
            if (c == '\n') {
                if (!to_parse[0] == 0) {
                    float num = atof(to_parse);
                    row.push_back(num);
                    memset(to_parse, 0, (tp_index + 1) * sizeof(char));
                    tp_index = 0;
                }
                data.push_back(row);
                row.clear();
            }
            else if (c == ',') {
                if (!to_parse[0] == 0) {
                    float num = atof(to_parse);
                    row.push_back(num);
                    memset(to_parse, 0, (tp_index + 1) * sizeof(char));
                    tp_index = 0;
                }
            }
            else to_parse[tp_index++] = c;
        }
        else if (c == '\n' && !parseable) parseable = true;
    }

    data = transpose(data);
    return data;
}

std::vector<std::string> Processing::get_tp_nodes() {
    return tp_nodes;
}

void Processing::printInputWaves(std::vector<std::vector<int>> test_pattern, int num_tests) {
    std::ofstream output;
    //char input[10] = "001101000";
    double max_time = trans_analysis_time;
    double delay = input_delay;
    double time_step = wavefile_timestep;
    
    std::queue<int> high_times;
    int end = (int)(max_time / time_step);

    for (int i = 0; i < tp_inputs; i++) {
        for (int j = 0; j < num_tests; j++) {
            if (test_pattern[j][i]) high_times.push((int)((delay + j*clock_period) / time_step));
        }
        std::stringstream ss;
        ss << "IN" << i;
        output.open(ss.str());
        for (int k = 0; k < end; k++) {
            if (high_times.empty()) {
                output << "0 ";
            }
            else if (k >= high_times.front()) {
                output << high_amplitude << " "; 
                high_times.pop();
            }
            else {
                output << "0 ";
            }
        }
        output.close();
    }
}
std::vector<std::vector<float>> Processing::transpose(std::vector<std::vector<float>> a) {
    int row_size = a[0].size();
    int col_size = a.size(); 
    std::vector<std::vector<float>> t(row_size);
    std::vector<float> temp(col_size);
    for (int i = 0; i < row_size; i++) {
        for (int j = 0; j < col_size; j++) {
            temp[j] = a[j][i];
        }
        t[i] = temp;
    }
    return t;
}

