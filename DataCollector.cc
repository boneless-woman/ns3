#include "DataCollector.h"
#include "LogicNode.h"
#include "NodeGroup.h"
#include "Channel.h"
#include <cmath>
#include <iostream>

const double DataCollector::BLUE_NODE_SPEED = 30.0; // 30节速度

DataCollector::DataCollector() {
    // 初始化
}

DataCollector::~DataCollector() {
    // 清理
}

Position DataCollector::getNodePosition(LogicNode* node) {
    if (!node) {
        std::cerr << "Error: Node is null" << std::endl;
        return Position();
    }
    
    // 从逻辑层节点获取移动模型
    Ptr<MobilityModel> mobility = node->getObject<MobilityModel>();
    if (!mobility) {
        std::cerr << "Error: No mobility model found" << std::endl;
        return Position();
    }
    
    Vector pos = mobility->GetPosition();
    return Position(pos.x, pos.y, pos.z);
}

std::vector<Position> DataCollector::getNodeGroupPosition(NodeGroup* nodes) {
    std::vector<Position> positions;
    
    if (!nodes) {
        std::cerr << "Error: NodeGroup is null" << std::endl;
        return positions;
    }
    
    // 假设NodeGroup有获取所有节点的方法
    auto nodeList = nodes->getAllNodes();
    for (auto& node : nodeList) {
        Position pos = getNodePosition(node);
        positions.push_back(pos);
    }
    
    return positions;
}

bool DataCollector::isChannelActive(Channel* channel) {
    if (!channel) {
        return false;
    }
    
    // 检查信道是否真实传输数据
    // 这里需要根据具体的Channel实现来判断
    return channel->isTransmitting() && channel->hasActiveConnections();
}

bool DataCollector::isChannelAvailable(Channel* channel) {
    if (!channel) {
        return false;
    }
    
    // 检查信道是否可用于通信
    return channel->isEnabled() && !channel->isBusy();
}

bool DataCollector::isConnected(LogicNode* node1, LogicNode* node2) {
    if (!node1 || !node2) {
        return false;
    }
    
    // 检查两节点间是否有可用通信线路
    // 这里需要根据具体的网络拓扑实现
    auto channels = node1->getConnectedChannels();
    for (auto& channel : channels) {
        if (channel->isConnectedTo(node2) && isChannelAvailable(channel)) {
            return true;
        }
    }
    
    return false;
}

double DataCollector::getDistance(LogicNode* node1, LogicNode* node2) {
    Position pos1 = getNodePosition(node1);
    Position pos2 = getNodePosition(node2);
    
    double dx = pos1.x - pos2.x;
    double dy = pos1.y - pos2.y;
    double dz = pos1.z - pos2.z;
    
    return sqrt(dx*dx + dy*dy + dz*dz);
}

Position DataCollector::coordinateTrans(Position pos) {
    // 先转换为地心参考系
    Position earthFramePos = blueFrameToEarthFrame(pos);
    
    // 然后转换为经纬度
    double lat, lon;
    earthFramePos.toGeographic(lat, lon);
    
    return Position(lat, lon, earthFramePos.z);
}

Position DataCollector::blueFrameToEarthFrame(Position blueFramePos) {
    // 获取当前仿真时间
    Time currentTime = Simulator::Now();
    double timeInSeconds = currentTime.GetSeconds();
    
    // 蓝方节点的位移（30节速度定速巡航）
    double blueNodeDisplacement = BLUE_NODE_SPEED * 0.514444 * timeInSeconds; // 转换为m/s
    
    // 根据蓝方节点的运动轨迹计算偏移
    Position earthFramePos;
    earthFramePos.x = blueFramePos.x + blueNodeDisplacement;
    earthFramePos.y = blueFramePos.y;
    earthFramePos.z = blueFramePos.z;
    
    return earthFramePos;
}

void Position::toGeographic(double& lat, double& lon) {
    // 这里需要根据具体的坐标系转换公式实现
    // 简化实现，实际需要更复杂的地理坐标转换
    const double EARTH_RADIUS = 6378137.0; // 地球半径（米）
    
    lat = asin(z / EARTH_RADIUS) * 180.0 / M_PI;
    lon = atan2(y, x) * 180.0 / M_PI;
}

// ProcessDataCollector 实现
ProcessDataCollector::ProcessDataCollector() : DataCollector() {
}

void ProcessDataCollector::collectBatchData() {
    // 批量数据收集的具体实现
    std::cout << "Collecting batch data..." << std::endl;
}

void ProcessDataCollector::scheduleDataCollection() {
    // 定时数据收集
    Simulator::Schedule(Seconds(1.0), &ProcessDataCollector::collectBatchData, this);
}

// NodeDataCollector 实现
NodeDataCollector::NodeDataCollector() : DataCollector() {
}

void NodeDataCollector::collectRealTimeData(LogicNode* node) {
    if (node) {
        Position pos = getNodePosition(node);
        std::cout << "Real-time position: " << pos.x << ", " << pos.y << ", " << pos.z << std::endl;
    }
}

void NodeDataCollector::collectCallbackData() {
    // 回调函数中的数据收集
    std::cout << "Collecting callback data..." << std::endl;
}