# BTP 2024-25
Ridhiman Kaur Dhindsa 

## Overview
The aim of this BTP project was to **detect jamming attacks in 5G UAV scenarios**, using the SOTA framework given in the paper: [Deep Attention Recognition for Attack Identification in 5G UAV scenarios: Novel Architecture and End-to-End Evaluation](https://arxiv.org/abs/2303.12947). This repo contains the code replicating their Multi-headed Deep Neural Network (MHDNN) called DAtR, as well as code for dataset generation. It also contains code for added novelty over the SOTA technique, by introducing **attacker localization** after detection of jamming attack. 

## Dataset Generation
Simulation files include:  
* `uav_automated.cc`: Generates attack and non-attack scenarios involving 1 eNB node and n UEs using the NS3 simulation software. It takes arguments like number of UEs, initial coordinates and velocities of UEs. 5G communication is modelled using the `mmwave` module. It logs parameters such as SINR and RSSI for each UE, using TraceSinks, and also generates NetAnimation files to visualize movement and positioning of eNBs, attackers and UEs.  
* `uav_localization.cc`: Generates a sample attack scenario with coordinates and parameter logging for localizing the position of attacker.  
* `Rx_parser_generalized.cc`: Obtains `RxPacketTrace.txt` for a simulation scenario and extracts SINR values for each UE, based on RNTI.
* `attack_siml.cc`: Processes SINR and RSSI files to simulate jamming attack on a UE, based on value of noise power and attacker transmission power.

Animation files include:  
* `mmwave-animation_basic.xml`: Depicts a simple non-attack scenario with 2 UEs and 1 eNB, and constant velocity mobility model.
* `mmwave-animation_automated.xml`: Depicts an attack scenario with 6 UEs  and 1 eNB, and constant velocity mobility model. UE 1, 3 are attacked.
* `mmwave-animation_localization.xml`: Depicts an attack scenario with 1 eNB, 11 UEs, 1 attacker node, at different coordinates, for localization purposes.

Automation files:  
* `nonattack_generator.cc`: Used for generating 550 non-attack testcases. Generates values of n, ue_posn, ue_vel based on limiting conditions and stores them in `simulation_parameters.csv`. Runs simulation files in appropriate order for each iteration to create the dataset `dataset/nonattack/TCi` (i = testcase number). Each TCi folder contains `RxPacketTrace.txt`, and `UEj_sinr.csv`, `UEj_rssi.csv` files (j = UE number ranging from 1-n).  
> NOTE: Running this file consumes significant time (5-6 hours) and storage resources (20-21 GB) as it generates a very large dataset.
* `attack_generator.cc`: Used for generating 550 attack testcases. Generates value of ue_list (target UEs for jamming attack), noise_db, tx_power_db, start_time, end_time of attack duration, and stores them in `attack_parameters.csv`. Modifies SINR, RSSI values appropriately.

### How to run
For simulation and dataset generation:
```sh
g++ Rx_parser_generalized.cc -o Rx_parser_generalized  
chmod +x ~/ns3-mmwave/nonattack_generator.sh  
./nonattack_generator.sh  
```

```sh
g++ attack_siml.cc -o attack_siml
chmod +x ~/ns3-mmwave/attack_generator.sh
./attack_generator.sh
```

```sh
~/ns3-mmwave$: ./ns3 run uav_localization.cc
```
```sh
~/ns3-mmwave$: ./NetAnim
"Then open the desired xml file in the GUI and press play."
```

## Model Training


### How to run

## Attacker Localization

### How to run