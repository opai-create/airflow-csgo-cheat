#include "globals.hpp"
#include "spoofers.hpp"

void c_spoofers::unlock_hidden_cvars()
{
    constexpr auto flag_devonly_or_hidden = (1 << 1) | (1 << 4);

    if (enabled != g_cfg.misc.unlock_hidden_cvars) 
    {
        enabled = g_cfg.misc.unlock_hidden_cvars;

        if (hidden_cvars.empty()) 
        {
            for (auto cmd = (**HACKS->cvar->get_con_command_base())->next; cmd != nullptr; cmd = cmd->next)
                if (cmd->flags & (XORN(flag_devonly_or_hidden)))
                    hidden_cvars.push_back(cmd);
        }

        for (auto cmd : hidden_cvars) 
        {
            if (enabled)
                cmd->flags &= ~(XORN(flag_devonly_or_hidden));
            else
                cmd->flags |= (XORN(flag_devonly_or_hidden));
        }
    }
}