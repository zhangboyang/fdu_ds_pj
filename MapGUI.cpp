#include <cassert>
#include <string>
#include "common.h"
#include "MapGUI.h"
#include "wstr.h"

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

// MsgBox
static wxString MsgBox_Title, MsgBox_Description, MsgBox_Text;

enum { ID_BtnOK };

class MapMsgBoxFrame: public wxFrame {
    public:
    MapMsgBoxFrame(const wxString& title, const wxPoint& pos, const wxSize& size)
        : wxFrame(NULL, wxID_ANY, title, pos, size)
    {
        wxPanel *panel = new wxPanel(this, wxID_ANY);

        wxBoxSizer *vbox = new wxBoxSizer(wxVERTICAL);
        wxBoxSizer *hbox1 = new wxBoxSizer(wxHORIZONTAL);
        wxBoxSizer *hbox2 = new wxBoxSizer(wxHORIZONTAL);
        wxBoxSizer *hbox3 = new wxBoxSizer(wxHORIZONTAL);

        wxStaticText *lblinfo = new wxStaticText(panel, wxID_ANY, MsgBox_Description);
        wxTextCtrl *txtinfo = new wxTextCtrl(panel, wxID_ANY, MsgBox_Text, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE);
        wxButton *btnok = new wxButton(panel, ID_BtnOK, wxT("OK"));

        hbox1->Add(lblinfo);
        vbox->Add(hbox1, 0, wxALIGN_LEFT | wxLEFT | wxTOP, 10);
        hbox2->Add(txtinfo, 1, wxEXPAND);
        vbox->Add(hbox2, 1, wxLEFT | wxRIGHT | wxBOTTOM | wxTOP | wxEXPAND, 10);
        hbox3->Add(btnok);
        vbox->Add(hbox3, 0, wxALIGN_RIGHT | wxRIGHT | wxBOTTOM, 10);
        
        panel->SetSizer(vbox);
        btnok->SetFocus();
        Center();
    }
    
    private:
    void OnOK(wxCommandEvent& event) { Close(true); }
    wxDECLARE_EVENT_TABLE();
};
wxBEGIN_EVENT_TABLE(MapMsgBoxFrame, wxFrame)
EVT_BUTTON(ID_BtnOK, MapMsgBoxFrame::OnOK)
wxEND_EVENT_TABLE()

class MapMsgBoxApp: public wxApp {
    public:
    virtual bool OnInit()
    {
        //wxGetTextFromUser("please input", "haha", "hello world", NULL);
        MapMsgBoxFrame *frame = new MapMsgBoxFrame(MsgBox_Title, wxDefaultPosition, wxSize(500, 500));
        frame->Show( true );
        return true;
    }
};








// InputBox
static wxString InputBox_Title, InputBox_Description, InputBox_Text;
class MapInputBoxApp: public wxApp {
    public:
    virtual bool OnInit()
    {
        InputBox_Text = wxGetTextFromUser(InputBox_Description, InputBox_Title, InputBox_Text, NULL);
        printf("haha!\n");
        return true;
    }
};









static const char FAKE_ARG1[] = "fake app name";

// MapGUI
void MapGUI::prepare_argc_argv()
{
    assert(strlen(FAKE_ARG1) < sizeof(fake_arg1));
    fake_argc = 1;
    fake_argv[0] = strcpy(fake_arg1, FAKE_ARG1);
}

void MapGUI::prepare_msgbox() { msgbox_text.clear(); }
void MapGUI::set_msgbox_title(const std::wstring &wstr) { msgbox_title = wstr; }
void MapGUI::set_msgbox_description(const std::wstring &wstr) { msgbox_description = wstr; }
void MapGUI::set_msgbox_append(const std::wstring &wstr) { msgbox_text += wstr; msgbox_text += '\n'; }
void MapGUI::show_msgbox()
{
    MsgBox_Title = wxString(msgbox_title);
    MsgBox_Description = wxString(msgbox_description);
    MsgBox_Text = wxString(msgbox_text);
    
    wxApp *p = new MapMsgBoxApp();
    wxApp::SetInstance(p);
    prepare_argc_argv();
    wxEntry(fake_argc, fake_argv);
}

void MapGUI::prepare_inputbox() { inputbox_text.clear(); }
void MapGUI::set_inputbox_title(const std::wstring &wstr) { inputbox_title = wstr; }
void MapGUI::set_inputbox_description(const std::wstring &wstr) { inputbox_description = wstr; }
void MapGUI::set_inputbox_text(const std::wstring &wstr) { inputbox_text = wstr; }
std::wstring MapGUI::show_inputbox()
{
    InputBox_Title = wxString(inputbox_title);
    InputBox_Description = wxString(inputbox_description);
    InputBox_Text = wxString(inputbox_text);
    
    wxApp *p = new MapInputBoxApp();
    wxApp::SetInstance(p);
    prepare_argc_argv();
    wxEntryStart(fake_argc, fake_argv);
    p->OnInit(); // don't call p->Run(), call p->OnInit() only
    wxEntryCleanup();
    
    return InputBox_Text.ToStdWstring();
}

