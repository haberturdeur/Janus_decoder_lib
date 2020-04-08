#pragma once

#include <map>
#include <memory>
#include <string>
#include <vector>

typedef uint8_t Janus_id_t;
typedef std::vector<uint8_t> input_t;

enum {
    u8_r = 1,
    u16_r,
    u32_r,
    u64_r,
    i8_r,
    i16_r,
    i32_r,
    i64_r,
    float_r,
    char_r,
    bool_r,
    templated_r,
    session_r,
    addr_r,
    max_r
};

class BaseResult {
public:
    virtual uint8_t type() = 0;
};

class JanusCommandBase {
public:
    virtual ~JanusCommandBase() = default;
    virtual BaseResult* execute(input_t* in) = 0;
};

class U32Result : public BaseResult {
public:
    uint8_t type() override { return u32_r; };
    uint32_t value() { return m_value; };
    void setValue(uint32_t v)
    {
        m_value = v;
    };

private:
    uint32_t m_value = 0;
};

class AddressResult : public BaseResult {
public:
    uint8_t type() final { return addr_r; }
    uint8_t value() { return m_value; }
    void setValue(uint8_t v) { m_value = v; }

private:
    uint8_t m_value = 0;
};

class SessionResult : public BaseResult {
public:
    uint8_t type() final { return session_r; }
    uint64_t value() { return m_value; }
    void setValue(uint8_t v) { m_value = v; }

private:
    uint64_t m_value = 0;
};

// class Janus_command_test: public Janus_command_base{
// public:
// 	~Janus_command_test() = default;
// 	BaseResult* Execute(input_t* in) {
// 		u32_result result;
// 		u32_result* result_p = &result;
// 		result_p->value = (uint32_t)2;
// 		return (static_cast<BaseResult*>(result_p)); };
// };

class Command_handler {
public:
    void store(Janus_id_t i_cmdId, std::string i_cmdName, JanusCommandBase* i_cmd);
    void store(std::map<std::string, Janus_id_t>* i_names, std::map<Janus_id_t, JanusCommandBase*>* i_ids);

    void setInput(input_t* i_inputPtr) { m_inputPtr = i_inputPtr; }

    BaseResult* run(Janus_id_t i_id, input_t* i_inputPtr = nullptr) { return m_commands[i_id]->execute((i_inputPtr == nullptr) ? m_inputPtr : i_inputPtr); }
    BaseResult* run(std::string i_name, input_t* i_inputPtr = nullptr) { return run(m_names[i_name], i_inputPtr); }

    Janus_id_t id(std::string i_name) { return m_names[i_name]; }
    std::string name(Janus_id_t i_id);

private:
    std::map<Janus_id_t, JanusCommandBase*> m_commands = {};
    std::map<std::string, Janus_id_t> m_names = {
        { "read", 0x01 },
        { "write", 0x02 },
        { "welcome", 0x03 },
        { "address_change", 0x04 },
        { "session", 0x05 }
    };
    input_t* m_inputPtr;
};
