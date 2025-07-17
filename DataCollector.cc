#include "DataCollector.h"
#include "LogicNode.h"
#include "NodeGroup.h"
#include "DataStruct.h"
#include "Channel.h"
#include <cmath>
#include <iostream>

const double DataCollector::BLUE_NODE_SPEED = 30.0; // 30节速度


// 构造函数
DataCollector::DataCollector() {
    // 初始化数据收集器
}

// 析构函数
DataCollector::~DataCollector() {
    // 清理资源
}

// 获取节点位置
PositionMessage DataCollector::getNodePosition(LogicNode node) {
    Position nodePos = node.getPosition();
    
    std::map<std::string, Position> poses;
    poses[node.nodeId] = nodePos;
    
    PositionMessage position(poses);
       
    
    return position;
}

// 获取节点群位置
PositionMessage DataCollector::getNodeGroupPosition(NodeGroup nodes) {
    
    std::vector<Position> nodePositions = nodes.getNodeGroupPosition();

    std::map<std::string, Position> poses;
    //遍历
    for (uint32_t i = 0; i < nodePositions.size(); i++) {
        
        std::string nodeId = nodes.nodeGroupId + "_" + std::to_string(i);
        poses[nodeId] = nodePositions[i];
    }
    
    PositionMessage groupPosition(poses);

    return groupPosition;
}

// 检查信道是否激活?
bool DataCollector::isChannelActive(Channel channel) {

    return false;
}

// 检查信道是否可用?
bool DataCollector::isChannelAvailable(Channel channel) {

    return false;
}

// 检查两个节点是否连接？
bool DataCollector::isConnected(LogicNode node1, LogicNode node2) {
   
    return false; 
}
