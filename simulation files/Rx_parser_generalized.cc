#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <unordered_map>

int main(int argc, char* argv[]) {
    // Check for correct number of arguments
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <number_of_UEs>" << std::endl;
        return 1;
    }

    // Get number of UEs from command line
    int numUEs;
    try {
        numUEs = std::stoi(argv[1]);
        if (numUEs <= 0) {
            throw std::invalid_argument("Number of UEs must be positive");
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: Invalid number of UEs. " << e.what() << std::endl;
        return 1;
    }

    // Input file
    std::ifstream inputFile("RxPacketTrace.txt");
    if (!inputFile.is_open()) {
        std::cerr << "Error: Could not open RxPacketTrace.txt" << std::endl;
        return 1;
    }

    // Map to store output file streams for each UE
    std::unordered_map<std::string, std::ofstream> ueFiles;

    // Open output files for each UE and write headers
    for (int i = 1; i <= numUEs; ++i) {
        std::string filename = "UE" + std::to_string(i) + "_sinr.csv";
        ueFiles[std::to_string(i)].open(filename);
        if (!ueFiles[std::to_string(i)].is_open()) {
            std::cerr << "Error: Could not create output file " << filename << std::endl;
            return 1;
        }
        ueFiles[std::to_string(i)] << "Time,SINR\n";
    }

    // Variables to store data
    std::string line;
    std::string dlUl, time, frame, subF, slot, sym, symbol, cellId, rnti, ccId, tbSize, mcs, rv, sinr, corrupt, tbler;

    // Read the input file line by line
    while (std::getline(inputFile, line)) {
        std::istringstream iss(line);
        iss >> dlUl >> time >> frame >> subF >> slot >> sym >> symbol >> cellId >> rnti >> ccId >> tbSize >> mcs >> rv >> sinr >> corrupt >> tbler;

        // Check if the RNTI corresponds to one of our UEs
        if (ueFiles.find(rnti) != ueFiles.end()) {
            ueFiles[rnti] << time << "," << sinr << "\n";
        }
    }

    // Close all files
    inputFile.close();
    for (auto& pair : ueFiles) {
        pair.second.close();
    }

    std::cout << "SINR values have been separated into " << numUEs << " UE files (UE1_sinr.csv to UE" 
              << numUEs << "_sinr.csv)" << std::endl;

    return 0;
}