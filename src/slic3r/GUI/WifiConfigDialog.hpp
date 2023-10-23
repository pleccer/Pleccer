#ifndef slic3r_WifiConfigDialog_hpp_
#define slic3r_WifiConfigDialog_hpp_

#include "GUI_Utils.hpp"

#include "../Utils/WifiScanner.hpp"

#include <wx/event.h>
#include <wx/dialog.h>
#include <wx/combobox.h>
#include <wx/textctrl.h>

#include "Widgets/ComboBox.hpp"
#include "Widgets/TextInput.hpp"

namespace Slic3r {
namespace GUI {

class RemovableDriveManager;
class WifiConfigDialog : public DPIDialog
{
public:
    WifiConfigDialog(wxWindow* parent, std::string& file_path, RemovableDriveManager* removable_manager);
    ~WifiConfigDialog();
private:
    ::ComboBox* m_ssid_combo {nullptr};
    ::TextInput* m_pass_textctrl {nullptr};
    ::ComboBox* m_drive_combo {nullptr};

    void on_ok(wxCommandEvent& e);
    void on_combo(wxCommandEvent& e);
    void on_rescan_drives(wxCommandEvent& e);
    void on_rescan_networks(wxCommandEvent& e);
    void on_retrieve_password(wxCommandEvent& e);
    void rescan_drives();
    void rescan_networks(bool select);
    void fill_password();
    // reference to string that is filled after ShowModal is called from owner
    std::string& out_file_path; 
    WifiScanner* m_wifi_scanner;
    RemovableDriveManager* m_removable_manager;
protected:
    void on_dpi_changed(const wxRect& suggested_rect) override;
    void on_sys_color_changed() override {}
};

}} // Slicer::GUI
#endif