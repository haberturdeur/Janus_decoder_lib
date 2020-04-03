#include "Janus_command_handler.h"

void Command_handler::store(id_t i_cmdId, std::string i_cmdName, JanusCommandBase* i_cmd)
{
    if (i_cmd) {
        m_commands.insert({ i_cmdId, i_cmd });
        m_names.insert({ i_cmdName, i_cmdId });
    }
}

void Command_handler::store(std::map<std::string, id_t>* i_names, std::map<id_t, JanusCommandBase*>* i_ids)
{
    m_commands.insert(i_ids->begin(), i_ids->end());
    m_names.insert(i_names->begin(), i_names->end());
}

std::string Command_handler::name(id_t i_id)
{
    for (std::map<std::string, id_t>::iterator it = m_names.begin(); it != m_names.end(); ++it)
        if (it->second == i_id)
            return it->first;
    return "error";
}