#include "Janus_command_handler.h"

void Command_handler::store( id_t cmd_id, std::string cmd_name, Janus_command_base* cmd){
    if(cmd){
        _commands.insert({cmd_id, cmd});
        _names.insert({cmd_name, cmd_id}); 
    }
}

void Command_handler::store( std::map<std::string, id_t>* names, std::map<id_t, Janus_command_base*>* ids){
    _commands.insert(ids->begin(), ids->end());
    _names.insert(names->begin(), names->end());
}

void Command_handler::setInput(input_t* in){
    input = in;
}

BaseResult* Command_handler::run(id_t id, input_t* in){
    return _commands[id]->Execute((in==nullptr) ? input : in);
}

BaseResult* Command_handler::run(std::string name, input_t* in){
    return run(_names[name], in);
}

id_t Command_handler::getId(std::string name){
    return _names[name];
}

std::string Command_handler::getName(id_t id){
    for (std::map<std::string, id_t>::iterator it = _names.begin(); it != _names.end(); ++it)
            if(it->second == id)
                return it->first;
    return "error";
}
