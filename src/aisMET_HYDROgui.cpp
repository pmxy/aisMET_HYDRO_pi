///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Oct 26 2018)
// http://www.wxformbuilder.org/
//
// PLEASE DO *NOT* EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#include "aisMET_HYDROgui.h"

///////////////////////////////////////////////////////////////////////////

aisMET_HYDROBase::aisMET_HYDROBase( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxSize( -1,-1 ), wxSize( -1,-1 ) );
	this->SetFont( wxFont( 10, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, false, wxT("Arial") ) );
	this->SetBackgroundColour( wxColour( 201, 201, 201 ) );

	wxBoxSizer* bSizer10;
	bSizer10 = new wxBoxSizer( wxVERTICAL );

	wxBoxSizer* bSizer121;
	bSizer121 = new wxBoxSizer( wxVERTICAL );

	m_staticText1 = new wxStaticText( this, wxID_ANY, _("Received sentence (For use with Test) :"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText1->Wrap( -1 );
	m_staticText1->SetFont( wxFont( 12, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, false, wxT("Arial") ) );

	bSizer121->Add( m_staticText1, 0, wxALL, 5 );

	m_textCtrlTest = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	m_textCtrlTest->SetMinSize( wxSize( 50,-1 ) );

	bSizer121->Add( m_textCtrlTest, 0, wxALL|wxEXPAND, 5 );


	bSizer10->Add( bSizer121, 0, wxEXPAND, 5 );

	wxFlexGridSizer* fgSizer82;
	fgSizer82 = new wxFlexGridSizer( 2, 1, 0, 0 );
	fgSizer82->AddGrowableCol( 0 );
	fgSizer82->AddGrowableCol( 1 );
	fgSizer82->AddGrowableCol( 2 );
	fgSizer82->AddGrowableCol( 3 );
	fgSizer82->AddGrowableRow( 0 );
	fgSizer82->AddGrowableRow( 1 );
	fgSizer82->SetFlexibleDirection( wxVERTICAL );
	fgSizer82->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

	m_buttonMessages = new wxButton( this, wxID_ANY, _("Log"), wxDefaultPosition, wxDefaultSize, 0 );
	m_buttonMessages->SetFont( wxFont( 12, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, false, wxT("Arial") ) );

	fgSizer82->Add( m_buttonMessages, 0, wxALL|wxEXPAND, 5 );

	m_button7 = new wxButton( this, wxID_ANY, _("Test"), wxDefaultPosition, wxDefaultSize, 0 );
	m_button7->SetFont( wxFont( 12, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, false, wxT("Arial") ) );

	fgSizer82->Add( m_button7, 1, wxALL|wxEXPAND, 2 );


	bSizer10->Add( fgSizer82, 0, wxEXPAND, 5 );


	this->SetSizer( bSizer10 );
	this->Layout();
	bSizer10->Fit( this );
	m_timer1.SetOwner( this, wxID_ANY );

	this->Centre( wxBOTH );

	// Connect Events
	this->Connect( wxEVT_CLOSE_WINDOW, wxCloseEventHandler( aisMET_HYDROBase::OnClose ) );
	m_buttonMessages->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( aisMET_HYDROBase::OnLogging ), NULL, this );
	m_button7->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( aisMET_HYDROBase::OnTest ), NULL, this );
	this->Connect( wxID_ANY, wxEVT_TIMER, wxTimerEventHandler( aisMET_HYDROBase::OnTimer ) );
}

aisMET_HYDROBase::~aisMET_HYDROBase()
{
	// Disconnect Events
	this->Disconnect( wxEVT_CLOSE_WINDOW, wxCloseEventHandler( aisMET_HYDROBase::OnClose ) );
	m_buttonMessages->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( aisMET_HYDROBase::OnLogging ), NULL, this );
	m_button7->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( aisMET_HYDROBase::OnTest ), NULL, this );
	this->Disconnect( wxID_ANY, wxEVT_TIMER, wxTimerEventHandler( aisMET_HYDROBase::OnTimer ) );

}

