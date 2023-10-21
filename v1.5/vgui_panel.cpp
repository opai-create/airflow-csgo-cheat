#include "globals.hpp"

void c_vgui_panel::update_panels(unsigned int id)
{
    int panels_count = 0;

    // catch panel handles
    // don't get it every frame because it eats perfomance
    for (auto& _panel : panels)
    {
        // found all panels, end loop
        if (panels_count >= panels.size())
            break;

        // add new panel if it was found
        if (_panel.panel_id > 0)
        {
            panels_count++;
            continue;
        }

        // get panel id once
        if (!_panel.panel_id)
            _panel.panel_id = get_new_panel(id, _panel.name);
    }
}