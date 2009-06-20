/* Copyright (C) 2009 Wildfire Games.
 * This file is part of 0 A.D.
 *
 * 0 A.D. is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * 0 A.D. is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with 0 A.D.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "precompiled.h"

#include "Environment.h"
#include "LightControl.h"

#include "GameInterface/Messages.h"
#include "ScenarioEditor/ScenarioEditor.h"
#include "General/Observable.h"
#include "CustomControls/ColourDialog/ColourDialog.h"

using AtlasMessage::Shareable;

static Observable<AtlasMessage::sEnvironmentSettings> g_EnvironmentSettings;

const float M_PIf = 3.14159265f;

//////////////////////////////////////////////////////////////////////////

class VariableSliderBox : public wxPanel
{
	static const int range = 1024;
public:
	VariableSliderBox(wxWindow* parent, const wxString& label, Shareable<float>& var, float min, float max)
		: wxPanel(parent),
		m_Var(var), m_Min(min), m_Max(max)
	{
		m_Conn = g_EnvironmentSettings.RegisterObserver(0, &VariableSliderBox::OnSettingsChange, this);

		m_Sizer = new wxStaticBoxSizer(wxVERTICAL, this, label);
		SetSizer(m_Sizer);

		m_Slider = new wxSlider(this, -1, 0, 0, range);
		m_Sizer->Add(m_Slider, wxSizerFlags().Expand());
	}
	
	void OnSettingsChange(const AtlasMessage::sEnvironmentSettings& WXUNUSED(env))
	{
		m_Slider->SetValue((m_Var - m_Min) * (range / (m_Max - m_Min)));
	}

	void OnScroll(wxScrollEvent& evt)
	{
		m_Var = m_Min + (m_Max - m_Min)*(evt.GetInt() / (float)range);

		g_EnvironmentSettings.NotifyObserversExcept(m_Conn);
	}

private:
	ObservableScopedConnection m_Conn;
	wxStaticBoxSizer* m_Sizer;
	wxSlider* m_Slider;
	Shareable<float>& m_Var;
	float m_Min, m_Max;

	DECLARE_EVENT_TABLE();
};

BEGIN_EVENT_TABLE(VariableSliderBox, wxPanel)
	EVT_SCROLL(VariableSliderBox::OnScroll)
END_EVENT_TABLE()

//////////////////////////////////////////////////////////////////////////

class VariableListBox : public wxPanel
{
public:
	VariableListBox(wxWindow* parent, const wxString& label, Shareable<std::wstring>& var)
		: wxPanel(parent),
		m_Var(var)
	{
		m_Conn = g_EnvironmentSettings.RegisterObserver(0, &VariableListBox::OnSettingsChange, this);

		m_Sizer = new wxStaticBoxSizer(wxVERTICAL, this, label);
		SetSizer(m_Sizer);

		m_Combo = new wxComboBox(this, -1, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxArrayString(), wxCB_READONLY),
		m_Sizer->Add(m_Combo, wxSizerFlags().Expand());
	}

	void SetChoices(const std::vector<std::wstring>& choices)
	{
		wxArrayString choices_arraystr;
		for (size_t i = 0; i < choices.size(); ++i)
			choices_arraystr.Add(choices[i].c_str());

		m_Combo->Clear();
		m_Combo->Append(choices_arraystr);

		m_Combo->SetValue(m_Var.c_str());
	}

	void OnSettingsChange(const AtlasMessage::sEnvironmentSettings& WXUNUSED(env))
	{
		m_Combo->SetValue(m_Var.c_str());
	}

	void OnSelect(wxCommandEvent& WXUNUSED(evt))
	{
		m_Var = std::wstring(m_Combo->GetValue().c_str());

		g_EnvironmentSettings.NotifyObserversExcept(m_Conn);
	}

private:
	ObservableScopedConnection m_Conn;
	wxStaticBoxSizer* m_Sizer;
	wxComboBox* m_Combo;
	Shareable<std::wstring>& m_Var;

	DECLARE_EVENT_TABLE();
};

BEGIN_EVENT_TABLE(VariableListBox, wxPanel)
	EVT_COMBOBOX(wxID_ANY, VariableListBox::OnSelect)
END_EVENT_TABLE()

//////////////////////////////////////////////////////////////////////////

class VariableColourBox : public wxPanel
{
public:
	VariableColourBox(wxWindow* parent, const wxString& label, Shareable<AtlasMessage::Colour>& colour)
		: wxPanel(parent),
		m_Colour(colour)
	{
		m_Conn = g_EnvironmentSettings.RegisterObserver(0, &VariableColourBox::OnSettingsChange, this);

		m_Sizer = new wxStaticBoxSizer(wxVERTICAL, this, label);
		SetSizer(m_Sizer);

		m_Button = new wxButton(this, -1);
		m_Sizer->Add(m_Button, wxSizerFlags().Expand());
	}

	void OnSettingsChange(const AtlasMessage::sEnvironmentSettings& WXUNUSED(env))
	{
		UpdateButton();
	}

	void OnClick(wxCommandEvent& WXUNUSED(evt))
	{
		ColourDialog dlg (this, _T("Scenario Editor/LightingColour"),
			wxColour(m_Colour->r, m_Colour->g, m_Colour->b));

		if (dlg.ShowModal() == wxID_OK)
		{
			wxColour& c = dlg.GetColourData().GetColour();
			m_Colour = AtlasMessage::Colour(c.Red(), c.Green(), c.Blue());
			UpdateButton();

			g_EnvironmentSettings.NotifyObserversExcept(m_Conn);
		}
	}

	void UpdateButton()
	{
		m_Button->SetBackgroundColour(wxColour(m_Colour->r, m_Colour->g, m_Colour->b));
		m_Button->SetLabel(wxString::Format(_T("%02X %02X %02X"), m_Colour->r, m_Colour->g, m_Colour->b));

		int y = 3*m_Colour->r + 6*m_Colour->g + 1*m_Colour->b;
		if (y > 1280)
			m_Button->SetForegroundColour(wxColour(0, 0, 0));
		else
			m_Button->SetForegroundColour(wxColour(255, 255, 255));
	}


private:
	ObservableScopedConnection m_Conn;
	wxStaticBoxSizer* m_Sizer;
	wxButton* m_Button;
	Shareable<AtlasMessage::Colour>& m_Colour;

	DECLARE_EVENT_TABLE();
};

BEGIN_EVENT_TABLE(VariableColourBox, wxPanel)
	EVT_BUTTON(wxID_ANY, VariableColourBox::OnClick)
END_EVENT_TABLE()

//////////////////////////////////////////////////////////////////////////

static void SendToGame(const AtlasMessage::sEnvironmentSettings& settings)
{
	POST_COMMAND(SetEnvironmentSettings, (settings));
}

EnvironmentSidebar::EnvironmentSidebar(ScenarioEditor& scenarioEditor, wxWindow* sidebarContainer, wxWindow* bottomBarContainer)
: Sidebar(scenarioEditor, sidebarContainer, bottomBarContainer)
{
	wxSizer* waterSizer = new wxGridSizer(2);
	m_MainSizer->Add(waterSizer, wxSizerFlags().Expand());

	waterSizer->Add(new VariableSliderBox(this, _("Water height"), g_EnvironmentSettings.waterheight, 0.f, 1.2f), wxSizerFlags().Expand());
	waterSizer->Add(new VariableSliderBox(this, _("Water shininess"), g_EnvironmentSettings.watershininess, 0.f, 250.f), wxSizerFlags().Expand());

	waterSizer->Add(new VariableSliderBox(this, _("Water waviness"), g_EnvironmentSettings.waterwaviness, 0.f, 10.f), wxSizerFlags().Expand());
	waterSizer->Add(new VariableSliderBox(this, _("Water murkiness"), g_EnvironmentSettings.watermurkiness, 0.f, 1.f), wxSizerFlags().Expand());

	waterSizer->Add(new VariableColourBox(this, _("Water colour"), g_EnvironmentSettings.watercolour), wxSizerFlags().Expand());
	waterSizer->Add(new VariableColourBox(this, _("Water tint"), g_EnvironmentSettings.watertint), wxSizerFlags().Expand());

	waterSizer->Add(new VariableColourBox(this, _("Reflection tint"), g_EnvironmentSettings.waterreflectiontint), wxSizerFlags().Expand());
	waterSizer->Add(new VariableSliderBox(this, _("Refl. tint strength"), g_EnvironmentSettings.waterreflectiontintstrength, 0.f, 1.f), wxSizerFlags().Expand());

	wxSizer* sunSizer = new wxGridSizer(2);
	m_MainSizer->Add(sunSizer, wxSizerFlags().Expand().Border(wxTOP, 8));

	sunSizer->Add(new VariableSliderBox(this, _("Sun rotation"), g_EnvironmentSettings.sunrotation, -M_PIf, M_PIf), wxSizerFlags().Expand());
	sunSizer->Add(new VariableSliderBox(this, _("Sun elevation"), g_EnvironmentSettings.sunelevation, -M_PIf/2, M_PIf/2), wxSizerFlags().Expand());
	sunSizer->Add(new VariableSliderBox(this, _("Sun overbrightness"), g_EnvironmentSettings.sunoverbrightness, 1.0f, 3.0f), wxSizerFlags().Expand());

	m_MainSizer->Add(new LightControl(this, wxSize(150, 150), g_EnvironmentSettings));
	m_MainSizer->Add(m_SkyList = new VariableListBox(this, _("Sky set"), g_EnvironmentSettings.skyset));
	m_MainSizer->Add(new VariableColourBox(this, _("Sun colour"), g_EnvironmentSettings.suncolour));
	m_MainSizer->Add(new VariableColourBox(this, _("Terrain ambient colour"), g_EnvironmentSettings.terraincolour));
	m_MainSizer->Add(new VariableColourBox(this, _("Object ambient colour"), g_EnvironmentSettings.unitcolour));

	m_Conn = g_EnvironmentSettings.RegisterObserver(0, &SendToGame);
}

void EnvironmentSidebar::OnFirstDisplay()
{
	// Load the list of skies. (Can only be done now rather than in the constructor,
	// after the game has been initialised.)
	AtlasMessage::qGetSkySets qry_skysets;
	qry_skysets.Post();
	m_SkyList->SetChoices(*qry_skysets.skysets);

	AtlasMessage::qGetEnvironmentSettings qry_env;
	qry_env.Post();
	g_EnvironmentSettings = qry_env.settings;

	g_EnvironmentSettings.NotifyObservers();
	// TODO: reupdate everything when loading a new map...
}
