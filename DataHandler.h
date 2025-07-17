#ifndef DATA_HANDLER_H
#define DATA_HANDLER_H

#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <iostream>
#include <map>
#include "ns3/core-module.h"
#include "DataCollector.h"



class DataHandler {
    public:
        DataHandler();
        ~DataHandler();

        void identifyCMD(Command cmd);

        void toJSON(PositionMessage msg);

        void cmdCallBack(/* Arg */);
    };

#endif // DATA_HANDLER_H