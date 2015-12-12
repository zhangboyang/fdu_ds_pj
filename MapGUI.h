#ifndef ZBY_MAPGUI_H
#define ZBY_MAPGUI_H
#include "common.h"

class MapGUI {
    private:
    int fake_argc;
    char *fake_argv[1];
    char fake_arg1[MAXLINE];
    
    std::wstring msgbox_title, msgbox_description, msgbox_text;
    std::wstring inputbox_title, inputbox_description, inputbox_text;
    
    void prepare_argc_argv();
    
    public:
    // msgbox
    void prepare_msgbox();
    void set_msgbox_title(const std::wstring &wstr);
    void set_msgbox_description(const std::wstring &wstr);
    void set_msgbox_append(const std::wstring &wstr);
    void show_msgbox();
    
    // inputbox
    void prepare_inputbox();
    void set_inputbox_title(const std::wstring &wstr);
    void set_inputbox_description(const std::wstring &wstr);
    void set_inputbox_text(const std::wstring &wstr); // set default text in input-box
    std::wstring show_inputbox();
};

#endif
