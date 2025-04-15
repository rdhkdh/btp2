// logs rssi for each UE as csv files, sinr via RxPacketTrace.txt and RxParser_generalized

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/mobility-module.h"
#include "ns3/mmwave-module.h"
#include "ns3/netanim-module.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/config-store.h"
#include "ns3/log.h"
#include "ns3/mmwave-helper.h"
#include <fstream>
using namespace ns3;
using namespace mmwave;

std::ofstream rssiFiles[30];

// Callback function for handling path loss trace of UEs (rssi printed)
void PathLossTraceSink(Ptr<const SpectrumPhy> txPhy, Ptr<const SpectrumPhy> rxPhy, double lossDb)
{
    // Get the node ID of the receiver
    uint32_t rxNodeId = rxPhy->GetDevice()->GetNode()->GetId();
    std::string filename = "UE" + std::to_string(rxNodeId) + "_rssi.csv";

    if (!rssiFiles[rxNodeId].is_open())
    {
        rssiFiles[rxNodeId].open(filename, std::ios_base::out);
        rssiFiles[rxNodeId]<<"Time,RSSI"<<std::endl;
    }
    rssiFiles[rxNodeId]<<Simulator::Now().GetSeconds()<<","<<30-lossDb<<std::endl;
}

int
main(int argc, char* argv[])
{
    int n = 2;  // Default number of UEs
    std::vector<Vector> ue_posn;  // Vector of initial positions
    std::vector<Vector> ue_vel;   // Vector of velocities

    // Parse command-line arguments
    CommandLine cmd;
    cmd.AddValue("n", "Number of UEs", n);
    std::string uePosnStr, ueVelStr;
    cmd.AddValue("ue_posn", "Comma-separated list of initial positions (x,y,z) for each UE", uePosnStr);
    cmd.AddValue("ue_vel", "Comma-separated list of velocities (vx,vy,vz) for each UE", ueVelStr);
    cmd.Parse(argc, argv);

    // Parse ue_posn argument
    std::istringstream posnStream(uePosnStr);
    std::string posnValue;
    int c=0;
    while (std::getline(posnStream, posnValue, ',')) {
        std::istringstream ss(posnValue);
        double t, x, y, z;
        ss >> t;
        c++;
        if(c%3==1) {x=t;}
        else if(c%3==2) {y=t;}
        else if(c%3==0) 
        {
            z=t;
            ue_posn.push_back(Vector(x,y,z));
        }
    }

    // Parse ue_vel argument
    std::istringstream velStream(ueVelStr);
    std::string velValue;
    int count=0;
    while (std::getline(velStream, velValue, ',')) {
        std::istringstream ss(velValue);
        double temp, vx, vy, vz;
        ss >> temp;
        count++;
        if(count%3==1) {vx=temp;}
        else if(count%3==2) {vy=temp;}
        else if(count%3==0) 
        {
            vz=temp;
            ue_vel.push_back(Vector(vx,vy,vz));
        }
    }

    //create nodes
    Ptr<MmWaveHelper> ptr_mmWave = CreateObject<MmWaveHelper>();

    NodeContainer enbNodes;
    NodeContainer ueNodes;
    enbNodes.Create(1);
    ueNodes.Create(n);

    // set ENB position and mobility
    Ptr<ListPositionAllocator> enbPositionAlloc = CreateObject<ListPositionAllocator>();
    enbPositionAlloc->Add(Vector(0.0, 0.0, 0.0));
    MobilityHelper enbmobility;
    enbmobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    enbmobility.SetPositionAllocator(enbPositionAlloc);
    enbmobility.Install(enbNodes);

    // Set UE mobility
    MobilityHelper uemobility;
    uemobility.SetMobilityModel("ns3::ConstantVelocityMobilityModel");
    uemobility.Install(ueNodes);

    for (int i = 0; i < n; ++i) {
        ueNodes.Get(i)->GetObject<MobilityModel>()->SetPosition(ue_posn[i]);
        ueNodes.Get(i)->GetObject<ConstantVelocityMobilityModel>()->SetVelocity(ue_vel[i]);
    }

    NetDeviceContainer enbNetDev = ptr_mmWave->InstallEnbDevice(enbNodes);
    NetDeviceContainer ueNetDev = ptr_mmWave->InstallUeDevice(ueNodes);

    ptr_mmWave->AttachToClosestEnb(ueNetDev, enbNetDev);
    ptr_mmWave->EnableTraces();

    // Activate a data radio bearer
    enum EpsBearer::Qci q = EpsBearer::GBR_CONV_VOICE;
    EpsBearer bearer(q);
    ptr_mmWave->ActivateDataRadioBearer(ueNetDev, bearer);

    //Enable NetAnim
    AnimationInterface anim("mmwave-animation_automated.xml");
    anim.EnablePacketMetadata(true);
    anim.UpdateNodeColor(enbNodes.Get(0), 0, 255, 0); // eNB in green
    for (uint32_t i = 0; i < ueNodes.GetN(); i++)
    {
        anim.UpdateNodeColor(ueNodes.Get(i), 0, 0, 255);  // UEs in blue
    }

    // Connect the PathLoss trace source to the callback function
    Config::ConnectWithoutContext(
        "/ChannelList/*/$ns3::SpectrumChannel/PathLoss",
        MakeCallback(&PathLossTraceSink)
    );

    Simulator::Stop(Seconds(2));
    Simulator::Run();
    Simulator::Destroy();

    // Close the file pointers and delete them
    for (int i = 0; i < n; ++i)
    {
        if (rssiFiles[i].is_open())
        {
            rssiFiles[i].close();
        }
    }

    return 0;
}