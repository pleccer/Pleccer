#include "BitmapToggleButton.hpp"

#include <wx/settings.h>

BitmapToggleButton::BitmapToggleButton(wxWindow* parent, const wxString& label, wxWindowID id)
{
    if (label.IsEmpty())
        wxBitmapToggleButton::Create(parent, id, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE | wxBU_EXACTFIT);
    else {
#ifdef __linux__
        wxSize label_size = parent->GetTextExtent(label);
        wxSize def_size = wxSize(label_size.GetX() + 20, label_size.GetY());
#else
        wxSize def_size = wxDefaultSize;
#endif
        // Call Create() from wxToggleButton instead of wxBitmapToggleButton to allow add Label text under Linux
        wxToggleButton::Create(parent, id, label, wxDefaultPosition, def_size, wxBORDER_NONE | wxBU_EXACTFIT);
    }

#ifdef __WXMSW__
	if (parent) {
		SetBackgroundColour(parent->GetBackgroundColour());
		SetForegroundColour(parent->GetForegroundColour());
	}
#elif __linux__
    SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW));
#endif

    Bind(wxEVT_TOGGLEBUTTON, [this](auto& e) {
	    update();

	    wxCommandEvent evt(wxEVT_CHECKBOX);
	    evt.SetInt(int(GetValue()));
	    wxPostEvent(this, evt);

	    e.Skip();
	});
}

void BitmapToggleButton::update_size()
{
#ifndef __WXGTK__
    wxSize best_sz = GetBestSize();
    SetSize(best_sz);
#endif
}
