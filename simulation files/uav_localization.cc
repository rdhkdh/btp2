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

NS_LOG_COMPONENT_DEFINE("UAVLocalization");

int main(int argc, char* argv[])
{
    CommandLine cmd;
    cmd.Parse(argc, argv);

    // Create MmWaveHelper
    Ptr<MmWaveHelper> ptr_mmWave = CreateObject<MmWaveHelper>();
    
    // Create nodes
    NodeContainer enbNodes;
    NodeContainer ueNodes;
    NodeContainer attackerNodes;
    enbNodes.Create(1);
    ueNodes.Create(11);
    attackerNodes.Create(1);

    // Setup mobility for eNB
    Ptr<ListPositionAllocator> enbPositionAlloc = CreateObject<ListPositionAllocator>();
    enbPositionAlloc->Add(Vector(0.0, 0.0, 0.0));
    MobilityHelper enbmobility;
    enbmobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    enbmobility.SetPositionAllocator(enbPositionAlloc);
    enbmobility.Install(enbNodes);

    // Setup mobility for UEs
    MobilityHelper uemobility;
    uemobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    
    // Create position allocator for UEs
    Ptr<ListPositionAllocator> uePositionAlloc = CreateObject<ListPositionAllocator>();
    uePositionAlloc->Add(Vector(10, 40, 0));   //UE1
    uePositionAlloc->Add(Vector(50, 20, 0));   //UE2
    uePositionAlloc->Add(Vector(115, 30, 0));  //UE3
    uePositionAlloc->Add(Vector(30, 70, 0));   //UE4
    uePositionAlloc->Add(Vector(115, 80, 0));  //UE5
    uePositionAlloc->Add(Vector(130, 150, 0)); //UE6
    uePositionAlloc->Add(Vector(40, 100, 0));  //UE7
    uePositionAlloc->Add(Vector(60, 90, 0));   //UE8
    uePositionAlloc->Add(Vector(80, 100, 0));  //UE9
    uePositionAlloc->Add(Vector(60, 130, 0));  //UE10
    uePositionAlloc->Add(Vector(80, 120, 0));  //UE11
    
    uemobility.SetPositionAllocator(uePositionAlloc);
    uemobility.Install(ueNodes);

    // Setup mobility for attacker
    Ptr<ListPositionAllocator> attackerPositionAlloc = CreateObject<ListPositionAllocator>();
    attackerPositionAlloc->Add(Vector(70.0, 110.0, 0.0));
    MobilityHelper attackermobility;
    attackermobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    attackermobility.SetPositionAllocator(attackerPositionAlloc);
    attackermobility.Install(attackerNodes);

    // Install network devices
    NetDeviceContainer enbNetDev = ptr_mmWave->InstallEnbDevice(enbNodes);
    NetDeviceContainer ueNetDev = ptr_mmWave->InstallUeDevice(ueNodes);
    NetDeviceContainer attackerNetDev = ptr_mmWave->InstallUeDevice(attackerNodes);

    // Attach UEs to eNB
    ptr_mmWave->AttachToClosestEnb(ueNetDev, enbNetDev);
    ptr_mmWave->AttachToClosestEnb(attackerNetDev, enbNetDev);
    
    // Activate data radio bearer
    enum EpsBearer::Qci q = EpsBearer::GBR_CONV_VOICE;
    EpsBearer bearer(q);
    ptr_mmWave->ActivateDataRadioBearer(ueNetDev, bearer);
    ptr_mmWave->ActivateDataRadioBearer(attackerNetDev, bearer);

    // Enable NetAnim
    AnimationInterface anim("mmwave-animation_8_localization.xml");
    anim.EnablePacketMetadata(true);
    anim.UpdateNodeColor(enbNodes.Get(0), 0, 255, 0);     // eNB in green
    anim.UpdateNodeColor(attackerNodes.Get(0), 255, 0, 0); // attacker in red
    
    for (uint32_t i = 0; i < ueNodes.GetN(); i++)
    {
        anim.UpdateNodeColor(ueNodes.Get(i), 0, 0, 255);  // UEs in blue
    }

    // Enable traces
    ptr_mmWave->EnableTraces();

    Simulator::Stop(Seconds(0.5));
    Simulator::Run();
    Simulator::Destroy();

    return 0;
}