#include "AtlasWindow.h"

class ActorEditorListCtrl;

class ActorEditor : public AtlasWindow
{
public:
	ActorEditor(wxWindow* parent);

protected:
	void Import(AtObj& in);
	AtObj Export();

private:
	ActorEditorListCtrl* m_ActorEditorListCtrl;

	wxCheckBox* m_CastShadows;
	wxTextCtrl* m_Material;
};
