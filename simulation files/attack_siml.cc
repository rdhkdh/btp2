#include <cstdio>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
using namespace std;

struct DataPoint
{
    double time;
    double value;
};

void
process_sinr_file(int ue,
                  const string& base_path,
                  double noise_db,
                  double start_time,
                  double end_time)
{
    string filename = base_path + "/UE" + to_string(ue) + "_sinr.csv";
    ifstream in_file(filename);
    if (!in_file.good())
        return; // Skip if file doesn't exist

    // Read entire file into memory
    vector<string> lines;
    string line;
    while (getline(in_file, line))
    {
        lines.push_back(line);
    }
    in_file.close();

    // Process and overwrite file
    ofstream out_file(filename);
    out_file << lines[0] << "\n"; // Write header

    for (size_t i = 1; i < lines.size(); i++)
    {
        DataPoint dp;
        if (sscanf(lines[i].c_str(), "%lf,%lf", &dp.time, &dp.value) == 2)
        {
            if (dp.time >= start_time && dp.time <= end_time)
            {
                dp.value -= noise_db;
            }
            out_file << dp.time << "," << dp.value << "\n";
        }
    }
}

void
process_rssi_file(int ue,
                  const string& base_path,
                  double tx_power_db,
                  double start_time,
                  double end_time)
{
    string filename = base_path + "/UE" + to_string(ue) + "_rssi.csv";
    ifstream in_file(filename);
    if (!in_file.good())
        return; // Skip if file doesn't exist

    vector<string> lines;
    string line;
    while (getline(in_file, line))
    {
        lines.push_back(line);
    }
    in_file.close();

    ofstream out_file(filename);
    out_file << lines[0] << "\n"; // Write header

    for (size_t i = 1; i < lines.size(); i++)
    {
        DataPoint dp;
        if (sscanf(lines[i].c_str(), "%lf,%lf", &dp.time, &dp.value) == 2)
        {
            if (dp.time >= start_time && dp.time <= end_time)
            {
                dp.value += tx_power_db;  // Apply attack
            }
            out_file << dp.time << "," << dp.value << "\n";
        }
    }
}

int
main(int argc, char* argv[])
{
    if (argc < 7)
    {
        cerr << "Usage: " << argv[0]
             << " <testcase_num> <ue_list> <noise_db> <tx_power_db> <start_time> <end_time>\n"
             << "Example: ./attack_siml 5 \"1,3\" 10 20 0.0 0.004\n";
        return 1;
    }

    int testcase_num = stoi(argv[1]);
    vector<int> ue_list;
    stringstream ss(argv[2]);
    string item;
    while (getline(ss, item, ','))
    {
        ue_list.push_back(stoi(item));
    }

    double noise_db = stod(argv[3]);
    double tx_power_db = stod(argv[4]);
    double start_time = stod(argv[5]);
    double end_time = stod(argv[6]);

    string base_path = string(getenv("HOME")) + "/dataset/attack/TC" + to_string(testcase_num);

    for (int ue : ue_list)
    {
        process_sinr_file(ue, base_path, noise_db, start_time, end_time);
        process_rssi_file(ue, base_path, tx_power_db, start_time, end_time);
    }

    return 0;
}