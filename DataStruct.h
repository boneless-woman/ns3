// #include




struct Command {
    std::string json;
    // std::string cmd;改为CommandType对象
    std::string time;

    Command(std::string json);

    void setCMD(std::string cmd);

};

struct GlideCommand : public Command {
    std::string nodeId;
    Position position;

    GlideCommand(std::string nodeId, Position position);

};

struct Message {
    std::string json;
    std::string time;
};

struct PositionMessage : public Message {
    std::map<std::string, Position> poses;

    PositionMessage(std::map<std::string, Position> poses);


};