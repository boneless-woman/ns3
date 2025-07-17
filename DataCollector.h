#ifndef DATA_COLLECTOR_H
#define DATA_COLLECTOR_H

#include <vector>
#include <string>
#include "ns3/core-module.h"
#include "ns3/mobility-module.h"
#include "ns3/network-module.h"
#include <LogicNode>
#include <NodeGroup>
#include <Channel>


using namespace ns3;


// 基础数据收集器
class DataCollector {
public:
    DataCollector();
    ~DataCollector();

    PositionMessage getNodePosition(LogicNode node);
    
    PositionMessage getNodeGroupPosition(NodeGroup nodes);
    
    bool isChannelActive(Channel channel);

    bool isChannelAvailable(Channel channel);

    bool isConnected(LogicNode node1, LogicNode node2);

    // double getDistance(LogicNode node1, LogicNode node2);

    // Position coordinateTrans(Position position, std::string);

    Scene initScene;


};

#endif // DATA_COLLECTOR_H