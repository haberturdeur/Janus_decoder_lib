#pragma once
#include <vector>
#include <memory>
#include <map>
#include <string>
#include <vector>

typedef uint8_t id_t;
typedef std::vector<uint8_t> input_t;

enum{
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
        virtual uint8_t getType() = 0;
};

class Janus_command_base {
 public:
  virtual ~Janus_command_base() = default;
  virtual BaseResult* Execute(input_t* in) = 0;
};

template<class T>
class templated_result : public BaseResult{
    public:
        uint8_t getType(){ return templated_r;};
        T getValue(){return value;};
        T value;
};

class u32Result : public BaseResult {
	public:
		uint8_t getType() { return u32_r; };
		uint32_t getValue() { return value; };
		void setValue(uint32_t v) {
			value = v;
		};
    private:
		uint32_t value = 0;
};

class AddressResult : public BaseResult{
    public:
        uint8_t getType(){return addr_r;}
        uint8_t getValue(){return value;}
    protected:
        void setValue(uint8_t v){value = v;}
    private:
        uint8_t value;
};

class SessionResult : public BaseResult{
    public:
        uint8_t getType(){return session_r;}
        uint64_t getValue(){return value;}
    protected:
        void setValue(uint8_t v){value = v;}
    private:
        uint64_t value;
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


class Command_handler{
    public:
        void store( id_t cmd_id, std::string cmd_name, Janus_command_base* cmd);
        void store( std::map<std::string, id_t>* names, std::map<id_t, Janus_command_base*>* ids);
        void setInput(input_t* in);
        BaseResult* run(id_t id, input_t* in = nullptr);
        BaseResult* run(std::string name, input_t* in = nullptr);
        id_t getId(std::string name);
        std::string getName(id_t id);
    private:
        std::map<id_t, Janus_command_base*> _commands = {};
        std::map<std::string, id_t> _names = {
            {"read", 0x01},
            {"write", 0x02},
            {"welcome", 0x03},
            {"address_change", 0x04},
            {"session", 0x05}
        };
        input_t* input;
};







