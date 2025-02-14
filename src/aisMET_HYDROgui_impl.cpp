/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  aisMET_HYDRO Plugin
 * Author:   Mike Rossiter
 *
 ***************************************************************************
 *   Copyright (C) 2017 by Mike Rossiter                                   *
 *   $EMAIL$                                                               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************
 */

#include "aisMET_HYDROgui_impl.h"
#include "aisMET_HYDRO_pi.h"
#include <wx/progdlg.h>
#include <wx/wx.h>

#include "wx/textfile.h"
#include <stdio.h>
#include <wx/timer.h>
#include "signal.h"
#include "widget.h"
#include <memory>

using mylibais::AisMsg;
using std::unique_ptr;
using namespace mylibais;


/**************************************************************************/
/*          Some assorted utilities                                       */
/**************************************************************************/

wxString toSDMM(int NEflag, double a)
{
	short neg = 0;
	int d;
	long m;

	if (a < 0.0)
	{
		a = -a;
		neg = 1;
	}
	d = (int)a;
	m = (long)((a - (double)d) * 60000.0);

	if (neg)
		d = -d;

	wxString s;

	if (!NEflag)
		s.Printf(_T("%d %02ld.%03ld'"), d, m / 1000, m % 1000);
	else
	{
		if (NEflag == 1)
		{
			char c = 'N';

			if (neg)
			{
				d = -d;
				c = 'S';
			}

			s.Printf(_T("%03d %02ld.%03ld %c"), d, m / 1000, (m % 1000), c);
		}
		else if (NEflag == 2)
		{
			char c = 'E';

			if (neg)
			{
				d = -d;
				c = 'W';
			}
			s.Printf(_T("%03d %02ld.%03ld %c"), d, m / 1000, (m % 1000), c);
		}
	}
	return s;
}




void assign(char* dest, char* arrTest2) { strcpy(dest, arrTest2); }

#define BUFSIZE 0x10000

Dlg::Dlg(wxWindow* parent, wxWindowID id, const wxString& title,
	const wxPoint& pos, const wxSize& size, long style)
	: aisMET_HYDROBase(parent, id, title, pos, size, style)
{
	this->Fit();
	dbg = false; // for debug output set to true

	m_bUsingWind = false;
	m_bUsingFollow = false;
	m_bInvalidPolarsFile = false;
	m_bInvalidGribFile = false;
	m_baisMET_HYDROHasStarted = false;
	m_bDisplayStarted = false;
	m_bPaused = false;
	m_bUsingTest = false;

	bool m_bShowaisMET_HYDRO = true;
	m_bHaveMessageList = false;
	m_bHaveDisplay = false;
	m_bUpdateTarget = false; 

	wxFileConfig* pConf = GetOCPNConfigObject();

	if (pConf) {
		pConf->SetPath(_T("/Settings/aisMET_HYDRO_pi"));

		pConf->Read(_T("aisMET_HYDROUseAis"), &m_bUseAis, 0);
		pConf->Read(_T("aisMET_HYDROUseFile"), &m_bUseFile, 0);
		pConf->Read(_T("aisMET_HYDROHECT"), &m_tMMSI, "12345");
	}

	AISTargetList = new AIS_Target_Hash;
	AISTargetNamesC = new AIS_Target_Name_Hash;
	AISTargetNamesNC = new AIS_Target_Name_Hash;
	pTargetData = new AIS_Target_Data;

	m_pASMmessages1 = NULL;
	myAISdisplay = NULL;

	auto path = GetPluginDataDir("aisMET_HYDRO_pi");
	fn.SetPath(path);
	fn.AppendDir("data");
	fn.AppendDir("pins");
	fn.SetFullName("green-pin.png");
	path = fn.GetFullPath();
	wxImage panelIcon(path);
    wxBitmap* m_panelBitmap = new wxBitmap(panelIcon);

	AddCustomWaypointIcon(m_panelBitmap, "green-pin", "MET_HYDRO");
	
}

namespace mylibais {
	template <typename T, typename... Args>
	std::unique_ptr<T> MakeUnique(Args &&... args) {
		//wxMessageBox("here");
		return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
	}

	unique_ptr<AisMsg>CreateAisMsg8(const string &body, const int fill_bits) {
		mylibais::Ais8 msg(body.c_str(), fill_bits);
		int fi = msg.fi;
		wxString fis = wxString::Format("%i", fi);
		wxMessageBox(fis);
		return MakeUnique<mylibais::Ais8_1_11>(body.c_str(), fill_bits);
	}



	unique_ptr<AisMsg> CreateAisMsg(const string &body, const int fill_bits) {


		string mybody = body;
		//wxMessageBox(mybody);

		int fill = fill_bits;
		wxString fills = wxString::Format("%i", fill);
		//wxMessageBox(fills);

		Ais8 msg(body.c_str(), fill_bits);



		int dac = msg.dac;
		wxString dacs = wxString::Format("%i", dac);
		wxMessageBox(dacs);

		int fi = msg.fi;
		wxString fis = wxString::Format("%i", fi);
		wxMessageBox(fis);

		return CreateAisMsg8(body, fill_bits);
	}




} // mylibais

/*
AIS_Target_Data* Dlg::Get_Target_Data_From_HECT(int mmsi)
{
	int sz = AISTargetList->count(pTargetData->HECT);

	wxString testCount = wxString::Format("%i", sz);
	wxMessageBox(testCount);


	if (AISTargetList->find(mmsi) == AISTargetList->end())     // if entry does not exist....
		return NULL;
	else
		return (*AISTargetList)[mmsi];          // find current entry
}
*/


inline const char * const BoolToString(bool b)
{
	return b ? "true" : "false";
}


void Dlg::OnMessageList(wxCommandEvent& event) {

	

	if (NULL == m_pASMmessages1) {		
	
		m_pASMmessages1 = new asmMessages(this, wxID_ANY, _T("ASM Messages"), wxPoint(100, 100), wxSize(300, 400), wxDEFAULT_DIALOG_STYLE |wxCLOSE_BOX| wxRESIZE_BORDER);
		CreateControlsMessageList();
		m_pASMmessages1->Show();
		m_bHaveMessageList = true;

		myTestDataCollection.clear();
		m_timer1.Start();
	}

	if (m_bHaveMessageList) {
		m_pASMmessages1->Show();
	}

	//wxMessageBox(BoolToString(listYN));
	//wxMessageBox(BoolToString(listYN));

	
}

void Dlg::OnCloseList(wxCloseEvent& event) {

	wxMessageBox("closing");
}

void Dlg::OnLogging(wxCommandEvent& event) {
	
	if ( NULL == myAISdisplay) {		
		myAISdisplay = new AISdisplay(this, wxID_ANY, _T("AIS Logging"), wxPoint(20, 20), wxSize(350, 400), wxCAPTION|wxCLOSE_BOX|wxRESIZE_BORDER);
		myAISdisplay->Show();
		m_bHaveDisplay = true;
	}

	if (m_bHaveDisplay) {
		myAISdisplay->Show();
	}
}

void Dlg::OnToggleButton(wxCommandEvent& event) {
	m_bPaused = !m_bPaused;
}

void Dlg::SetAISMessage(wxString &msg , wxString &sentence)
{
		
	if (m_bPaused) return;
	
	m_message = sentence;

	bool m_bGotDAC = DecodeForDAC(msg);

	if (m_bGotDAC) {
		
		Decode(msg);
	}
	
}

wxString Dlg::SetaisMET_HYDROMessage(string &msg) {
	unique_ptr<AisMsg> myMsg;

	//AIS_Bitstring* myBits;

	string payload;
	//   !AIVDM,1,1,,A,E>j4e>@LtqHIpHHLh@@@@@@@@@@0Vei<=iWL000000N2P0,4*05
	//payload = "E>j4e>@LtqHIpHHLh@@@@@@@@@@0Vei<=iWL000000N2P0"; //aisMET_HYDRO
	payload = msg;
	//string output;

	// myMsg = CreateAisMsg(payload,0);
	int mm = myMsg->mmsi;
	int ms = myMsg->message_id;

	wxString outstring(wxString::Format(("%i"), mm));
	return outstring;
	
}

void Dlg::JumpTo(wxString lat, wxString lon, int scale)
{
	double mla, mlo;

	lat.ToDouble(&mla);
	lon.ToDouble(&mlo);
	JumpToPosition(mla, mlo, scale);

}

void Dlg::OnClose(wxCloseEvent& event)
{	
	plugin->OnaisMET_HYDRODialogClose();
}


wxString Dlg::makeCheckSum(wxString mySentence)
{
	size_t i;
	unsigned char XOR;

	wxString s(mySentence);
	wxCharBuffer buffer = s.ToUTF8();
	char* Buff = buffer.data(); // data() returns const char *
	size_t iLen = strlen(Buff);
	for (XOR = 0, i = 0; i < iLen; i++)
		XOR ^= (unsigned char)Buff[i];
	stringstream tmpss;
	tmpss << hex << (int)XOR << endl;
	wxString mystr = tmpss.str();
	return mystr;
}

wxString Dlg::DateTimeToTimeString(wxDateTime myDT)
{
	wxString sHours, sMinutes, sSecs;
	sHours = myDT.Format(_T("%H"));
	sMinutes = myDT.Format(_T("%M"));
	sSecs = myDT.Format(_T("%S"));
	wxString dtss = sHours + sMinutes + sSecs;
	return dtss;
}

wxString Dlg::DateTimeToDateString(wxDateTime myDT)
{

	wxString sDay, sMonth, sYear;
	sDay = myDT.Format(_T("%d"));
	sMonth = myDT.Format(_T("%m"));
	sYear = myDT.Format(_T("%y"));

	return sDay + sMonth + sYear;
}


bool Dlg::DecodeForDAC(wxString insentence)
{

	string myMsg = std::string(insentence.mb_str());
    
	const char* payload1 = myMsg.c_str();
	mylibais::Ais8 myDacFi(payload1, 0);

	int n = strlen(payload1);
	wxString len = wxString::Format("%i", n);
	//wxMessageBox(len);

	int dac0 = myDacFi.dac;
	wxString outdac0 = wxString::Format("%i", dac0);
	//wxMessageBox(outdac0);
	
	int fi0 = myDacFi.fi;
	wxString outfi0 = wxString::Format("%i", fi0);
    //wxMessageBox(outfi0);
	if (fi0 == 31 && dac0 == 1) {
		//wxMessageBox("here");

		return true;
	}

	if (fi0 == 25 || fi0 == 26 || fi0 == 41) {
		return false;
	}
	else
		return false;


}

void Dlg::Decode(wxString sentence)
{

	string myMsg = std::string(sentence.mb_str());

	const char* payload1 = myMsg.c_str();
	mylibais::Ais8 myDacFi(payload1, 0);

	int dac0 = myDacFi.dac;
	int fi0;
	if (dac0 == 1) {
		//wxString outdac0 = wxString::Format("%i", dac0);
		//wxMessageBox(outdac0);


		fi0 = myDacFi.fi;
		if (fi0 == 31 && dac0 == 1) {
			wxString outfi0 = wxString::Format("%i", fi0);
			//wxMessageBox(outfi0);
			if (m_bHaveDisplay) {
				if (myAISdisplay->m_tbAISPause->GetValue()) {

					int mm = 0;
					int ms = 0;

					myAISdisplay->m_tcAIS->AppendText(mySentence);
				}
			}
			getAis8_1_31(myMsg);

		}
	}
	else {

		if (fi0 == 11 && dac0 == 1) {
			wxString outfi0 = wxString::Format("%i", fi0);
			wxMessageBox("Deprecated");
			if (m_bHaveDisplay) {
				if (myAISdisplay->m_tbAISPause->GetValue()) {

					int mm = 0;
					int ms = 0;

					myAISdisplay->m_tcAIS->AppendText(mySentence);
				}
			}
			getAis8_1_11(myMsg);

		}

	}
	
}

void Dlg::OnTest(wxCommandEvent& event)
{
	m_bUsingTest = true;

	wxString mySentence = plugin->m_pDialog->m_textCtrlTest->GetValue();

	if (mySentence.IsEmpty() || mySentence.IsNull()) {
		wxMessageBox("No sentence has been entered");
		return;
	}

	if (mySentence.Mid(0, 6) != "!AIVDM") {
		wxMessageBox("Invalid sentence");
		return;
	}

	string myMsg = parseNMEASentence(mySentence).ToStdString();

	const char* payload1 = myMsg.c_str();
	mylibais::Ais8 myDacFi(payload1, 0);

	int dac0 = myDacFi.dac;
	wxString outdac0 = wxString::Format("%i", dac0);
	wxMessageBox(outdac0, "DAC");

	int fi0 = myDacFi.fi;
	wxString outfi0 = wxString::Format("%i", fi0);
	wxMessageBox(outfi0, "FI");

	

}


wxString Dlg::parseNMEASentence(wxString& sentence)
{

	wxString token[40];
	wxString s0, s1, s2, s3, s4, s5, s6, s7, s8, s9, s10, s11;
	token[0] = _T("");

	wxStringTokenizer tokenizer(sentence, wxT(","));
	
	int i = 0;

	while (tokenizer.HasMoreTokens()) {
		token[i] = tokenizer.GetNextToken();
		i++;
	}
	if (token[0].Right(3) == _T("VDM")) {
		
	}
	
	return "999";
}

void Dlg::UpdateAISTargetList(void)
{
	if (m_bHaveMessageList) {
		
			int index = 0; 
			AIS_Target_Hash::iterator it;
			AIS_Target_Data *pAISTarget;
			AIS_Target_Hash *current_targets = new AIS_Target_Hash;

			current_targets = AISTargetList;
			size_t z = current_targets->size();
			//wxString sz = wxString::Format("%i", z);
			//wxMessageBox(sz);

			if (z == 0) {
				return;
			}

			m_pASMmessages1->m_pListCtrlAISTargets->DeleteAllItems();

			int h = 0;
			int id = 0;

			for (it = (current_targets)->begin(); it != (current_targets)->end(); ++it, ++index) {

				pAISTarget = it->second;

				wxListItem item;
				item.SetId(id);
				m_pASMmessages1->m_pListCtrlAISTargets->InsertItem(item);
				for (int j = 0; j < 2; j++) {
					item.SetColumn(j);
					if (j == 0) {
						h = pAISTarget->hect;
						wxString shect = wxString::Format("%i", h);
						item.SetText(shect);
						m_pASMmessages1->m_pListCtrlAISTargets->SetItem(item);
					}

					if (j == 1) {
						h = pAISTarget->hect;;
						string r = pAISTarget->RISindex;
						item.SetText(r);
						m_pASMmessages1->m_pListCtrlAISTargets->SetItem(item);
					}
				}

				id++;
			}

#ifdef __WXMSW__

			m_pASMmessages1->m_pListCtrlAISTargets->Refresh(false);
			
#endif 

	}		
}

wxString Dlg::MakeDescription(mylibais::Ais8_1_31 myData) {

	wxString myDescription;
	wxString cr = "\n";
	int idefault  = 0;
	
	wxString sMMSI = wxString::Format("%i", pTargetData->MMSI);
	myDescription += "MMSI: " + sMMSI + cr;
	
	int dd = myData.utc_day;
	wxString outdd = wxString::Format("%i", dd);
	int hr = myData.utc_hour;
	wxString outhr = wxString::Format("%i", hr);
	int min = myData.utc_min;
	wxString outmin = wxString::Format("%i", min);

	wxString ddhhmm = outdd + outhr + outmin;
	string DayHourMinute = std::string(ddhhmm.mb_str());

	myDescription += DayHourMinute + cr;

	int wind_ave = myData.wind_ave;
	wxString oWind_ave = wxString::Format("%i", wind_ave);
	idefault = (int)wind_ave;
	if (idefault == 127) {}
		//myDescription += "Average wind speed: n/a" + cr;
	else
		myDescription += "Average wind speed: " + oWind_ave + " knots" + cr;

	int wind_dir = myData.wind_dir;
	wxString oWind_dir = wxString::Format("%i", wind_dir);
	idefault = (int)wind_dir;
	if (idefault == 360) {}
		//myDescription += "Average wind speed: n/a" + cr;
	else
		myDescription += "Average wind direction: " + oWind_dir + " deg" + cr;

	float air_press = myData.air_pres;
	idefault = (int)air_press;
	wxString outpress = wxString::Format("%i", idefault);
	if (idefault == 1311 || idefault == 800) {}
		//myDescription += "Air pressure: n/a" + cr;
	else
		myDescription += "Air pressure:" + outpress + cr;

	float surf_cur_speed = myData.surf_cur_speed;
	wxString osurf_cur_speed = wxString::Format("%f", surf_cur_speed);
	idefault = (int)surf_cur_speed;
	if (idefault != 255) 
		//myDescription +=  "Surface current spd: n/a" + cr;
		myDescription += "Surface current spd: " + osurf_cur_speed + cr;

	int water_level = myData.water_level;	
	if (water_level == 4001) {}
		//myDescription += "Water level: n/a" + cr;
	else {
		double wl = (1.0 * water_level / 100) -10.;
		wxString oWater_level = wxString::Format("%f", wl);
		myDescription += "Water level: " + oWater_level + cr;
	}
	
	int ice = myData.ice;
	wxString oIce = wxString::Format("%i", ice);
	switch (ice) {
	case 0:
		//myDescription += "Ice: NO ice present" + cr;
		break;
	case 1:
		myDescription += "Ice: YES, ice is present" + cr;
		break;
	case 3:
		//myDescription += "Ice: n/a" + cr;
		break;
	}

	return myDescription;

}

// ************ Weather (International) **************
void Dlg::getAis8_1_11(string rawPayload) {

	//mySignalCollection.clear();  // This avoids polluting myTestDataCollection


	AIS_Target_Data *pStaleTarget = NULL;
    AIS_Target_Data *pWeatherTarget = NULL;

	bool bnewtarget = false;
	bool bdecode_result = false;

	const char* payload = rawPayload.c_str();

	mylibais::Ais8_1_11 myRIS(payload, 0);

	AisPoint weatherPoint = myRIS.position;
	wxString outLat = wxString::Format("%f", weatherPoint.lat_deg);
	//wxMessageBox(outLat);

	wxString outLon = wxString::Format("%f", weatherPoint.lng_deg);
	//wxMessageBox(outLon);

	int mm = myRIS.mmsi;
	wxString outmmsi = wxString::Format("%i", mm);
	string mmsi = std::string(outmmsi.mb_str());

	//wxMessageBox(outmm);

	double air_press = myRIS.air_pres;
	wxString outpress = wxString::Format("%f", air_press);
	//wxMessageBox(outpress);

	
	// Search the current AISTargetList for an mmsi match
	AIS_Target_Hash::iterator it = AISTargetList->find(mm);
	

	if (it == AISTargetList->end())                  // not found
	{			
		pTargetData = new AIS_Target_Data;
		pTargetData->IMO = mm;

		pTargetData->Lat = weatherPoint.lat_deg;
		pTargetData->Lon = weatherPoint.lng_deg;
		pTargetData->air_press = air_press;
		

		bdecode_result = true;
		bnewtarget = true;
		m_n_targets++;
	}
	else {
		
		// Not needed at present
		// pTargetData = it->second;          // find current entry
		// save a pointer to stale data
	}

	

	//  If the message was decoded correctly
	//  Update the AIS Target information
	if (bdecode_result) {
		m_bUpdateTarget = true;
		//signalData.clear();
		
		// **********
		// signalData is not used but FindSignalData populates myTestDataCollection
		// This is used by the factory to draw the signal locations
		//
		// ***********
		//AISshipNameCache(pTargetData, AISTargetNamesC, AISTargetNamesNC, hect);
		(*AISTargetList)[pTargetData->MMSI] = pTargetData;  // update the hash table entry
		//mySignalsFoundCollection.push_back(*pTargetData);
		
		//wxString sz = wxString::Format("%i",pTargetData->hect);
	   // wxMessageBox(signalData.at(1).RISindex);
		 wxFileName fn;

		double myLat = pTargetData->Lat;
		double myLon = pTargetData->Lon;

		PlugIn_Waypoint*  wayPoint = new PlugIn_Waypoint(myLat, myLon, "", "", "") ;
		wayPoint->m_MarkDescription = outpress;
		wayPoint->m_IsVisible = true;
		wayPoint->m_IconName = "green-pin";

		AddSingleWaypoint(wayPoint, false);
		GetParent()->Refresh();

	}

	/*
	int sect = myRIS.section;
	wxString outsect = wxString::Format("%i", sect);
	//wxMessageBox(outsect);

	int objtype = myRIS.objectType;
	wxString outobjtype = wxString::Format("%i", objtype);
	//wxMessageBox(outtype);

	int objnum = myRIS.objectNumber;
	wxString outobjnum = wxString::Format("%i", objnum);
	//wxMessageBox(outobj);

	int hect = 0;

	hect = myRIS.hectometre;
	wxString outhect = wxString::Format("%i", hect);
	//wxMessageBox(outhect);

	int sig = myRIS.signalForm;
	wxString outsig = wxString::Format("%i", sig);
	//wxMessageBox(outsig);

	int orientation = myRIS.orientation;
	wxString outorientation = wxString::Format("%i", orientation);
	//wxMessageBox(outorientation);

	int imp = myRIS.impact;
	wxString outimp = wxString::Format("%i", imp);
	//wxMessageBox(outimp);

	int stat = myRIS.lightStatus;
	wxString outstat = wxString::Format("%i", stat);
	//wxMessageBox(outstat);
    vector<AIS_Target_Data> signalData;
	signalData = FindSignalRISindex(hect, "SISTAT_6");	

    // Search the current AISTargetList for an RISindex match
	AIS_Target_Hash::iterator it = AISTargetList->find(signalData.at(0).RISindex);
	

	if (it == AISTargetList->end())                  // not found
	{			
		//signalData.clear();
		pTargetData = new AIS_Target_Data;
		pTargetData->RISindex = signalData.at(0).RISindex;
		pTargetData->Lat = signalData.at(0).Lat;
		pTargetData->Lon = signalData.at(0).Lon;
		pTargetData->hect = hect;
		pTargetData->signalForm = sig;
		pTargetData->signalStatus = stat;
		pTargetData->country = outcountry;

		bdecode_result = true;
		bnewtarget = true;
		m_n_targets++;
	}
	else {
		
		// Not needed at present
		// pTargetData = it->second;          // find current entry
		// save a pointer to stale data
	}

	

	//  If the message was decoded correctly
	//  Update the AIS Target information
	if (bdecode_result) {
		m_bUpdateTarget = true;
		//signalData.clear();
		
		// **********
		// signalData is not used but FindSignalData populates myTestDataCollection
		// This is used by the factory to draw the signal locations
		//
		// ***********
		//AISshipNameCache(pTargetData, AISTargetNamesC, AISTargetNamesNC, hect);
		(*AISTargetList)[pTargetData->RISindex] = pTargetData;  // update the hash table entry
		mySignalsFoundCollection.push_back(*pTargetData);
		
		//wxString sz = wxString::Format("%i",pTargetData->hect);
	   // wxMessageBox(signalData.at(1).RISindex);
		 wxFileName fn;

		double myLat = pTargetData->Lat;
		double myLon = pTargetData->Lon;

		PlugIn_Waypoint*  wayPoint = new PlugIn_Waypoint(myLat, myLon, "", "", "") ;
		wayPoint->m_MarkDescription = pTargetData->RISindex;
		wayPoint->m_IsVisible = true;
		wayPoint->m_IconName = "green-pin";

		AddSingleWaypoint(wayPoint, false);
		GetParent()->Refresh();

	}
	

	*/
}

// ************ Weather (International) **************
void Dlg::getAis8_1_31(string rawPayload) {

	AIS_Target_Data *pStaleTarget = NULL;
    AIS_Target_Data *pWeatherTarget = NULL;

	bool bnewtarget = false;
	bool bdecode_result = false;

	const char* payload = rawPayload.c_str();

	mylibais::Ais8_1_31 myRIS(payload, 0);
	
	int mm = myRIS.mmsi;

	AisPoint weatherPoint = myRIS.position;
	wxString outLat = wxString::Format("%f", weatherPoint.lat_deg);
	wxString outLon = wxString::Format("%f", weatherPoint.lng_deg);

	// Search the current AISTargetList for an mmsi match
	AIS_Target_Hash::iterator it = AISTargetList->find(mm);
	

	if (it == AISTargetList->end())                  // not found
	{			
		pTargetData = new AIS_Target_Data;		

		pTargetData->Lat = weatherPoint.lat_deg;
		pTargetData->Lon = weatherPoint.lng_deg;
		pTargetData->Description = MakeDescription(myRIS);		

		bdecode_result = true;
		bnewtarget = true;
		m_n_targets++;
	}
	else {
		
		// Not needed at present
		// pTargetData = it->second;          // find current entry
		// save a pointer to stale data
	}

	

	//  If the message was decoded correctly
	//  Update the AIS Target information
	if (bdecode_result) {
		m_bUpdateTarget = true;
		//signalData.clear();

		// **********
		// signalData is not used but FindSignalData populates myTestDataCollection
		// This is used by the factory to draw the signal locations
		//
		// ***********
		//AISshipNameCache(pTargetData, AISTargetNamesC, AISTargetNamesNC, hect);
		(*AISTargetList)[pTargetData->MMSI] = pTargetData;  // update the hash table entry
		//mySignalsFoundCollection.push_back(*pTargetData);
		
		//wxString sz = wxString::Format("%i",pTargetData->hect);
	   // wxMessageBox(signalData.at(1).RISindex);
		 wxFileName fn;

		double myLat = pTargetData->Lat;
		double myLon = pTargetData->Lon;
		
		plugin->m_pDialog->m_textCtrlTest->SetValue(m_message);
		PlugIn_Waypoint*  wayPoint = new PlugIn_Waypoint(myLat, myLon, "", "", "");
		wayPoint->m_MarkDescription = pTargetData->Description;
		wayPoint->m_IsVisible = true;
		//wayPoint->m_MarkName = sMMSI;
		wayPoint->m_IconName = "green-pin";

		AddSingleWaypoint(wayPoint, false);
		GetParent()->Refresh();
		
	}

	/*
	int sect = myRIS.section;
	wxString outsect = wxString::Format("%i", sect);
	//wxMessageBox(outsect);

	int objtype = myRIS.objectType;
	wxString outobjtype = wxString::Format("%i", objtype);
	//wxMessageBox(outtype);

	int objnum = myRIS.objectNumber;
	wxString outobjnum = wxString::Format("%i", objnum);
	//wxMessageBox(outobj);

	int hect = 0;

	hect = myRIS.hectometre;
	wxString outhect = wxString::Format("%i", hect);
	//wxMessageBox(outhect);

	int sig = myRIS.signalForm;
	wxString outsig = wxString::Format("%i", sig);
	//wxMessageBox(outsig);

	int orientation = myRIS.orientation;
	wxString outorientation = wxString::Format("%i", orientation);
	//wxMessageBox(outorientation);

	int imp = myRIS.impact;
	wxString outimp = wxString::Format("%i", imp);
	//wxMessageBox(outimp);

	int stat = myRIS.lightStatus;
	wxString outstat = wxString::Format("%i", stat);
	//wxMessageBox(outstat);
    vector<AIS_Target_Data> signalData;
	signalData = FindSignalRISindex(hect, "SISTAT_6");	

    // Search the current AISTargetList for an RISindex match
	AIS_Target_Hash::iterator it = AISTargetList->find(signalData.at(0).RISindex);
	

	if (it == AISTargetList->end())                  // not found
	{			
		//signalData.clear();
		pTargetData = new AIS_Target_Data;
		pTargetData->RISindex = signalData.at(0).RISindex;
		pTargetData->Lat = signalData.at(0).Lat;
		pTargetData->Lon = signalData.at(0).Lon;
		pTargetData->hect = hect;
		pTargetData->signalForm = sig;
		pTargetData->signalStatus = stat;
		pTargetData->country = outcountry;

		bdecode_result = true;
		bnewtarget = true;
		m_n_targets++;
	}
	else {
		
		// Not needed at present
		// pTargetData = it->second;          // find current entry
		// save a pointer to stale data
	}

	

	//  If the message was decoded correctly
	//  Update the AIS Target information
	if (bdecode_result) {
		m_bUpdateTarget = true;
		//signalData.clear();
		
		// **********
		// signalData is not used but FindSignalData populates myTestDataCollection
		// This is used by the factory to draw the signal locations
		//
		// ***********
		//AISshipNameCache(pTargetData, AISTargetNamesC, AISTargetNamesNC, hect);
		(*AISTargetList)[pTargetData->RISindex] = pTargetData;  // update the hash table entry
		mySignalsFoundCollection.push_back(*pTargetData);
		
		//wxString sz = wxString::Format("%i",pTargetData->hect);
	   // wxMessageBox(signalData.at(1).RISindex);
		 wxFileName fn;

		double myLat = pTargetData->Lat;
		double myLon = pTargetData->Lon;

		PlugIn_Waypoint*  wayPoint = new PlugIn_Waypoint(myLat, myLon, "", "", "") ;
		wayPoint->m_MarkDescription = pTargetData->RISindex;
		wayPoint->m_IsVisible = true;
		wayPoint->m_IconName = "green-pin";

		AddSingleWaypoint(wayPoint, false);
		GetParent()->Refresh();

	}
	

	*/
}

void Dlg::SetViewPort(PlugIn_ViewPort *vp)
{
	if (m_vp == vp)  return;

	m_vp = new PlugIn_ViewPort(*vp);
}

void Dlg::OnFactory(wxCommandEvent& event)
{
	myTestData.Lat = 50.0;
	myTestData.Lon = -4.0;

	myTestDataCollection.push_back(myTestData);
	myTestData.Lat = 50.5;
	myTestData.Lon = -4.5;

	myTestDataCollection.push_back(myTestData);

}

void Dlg::RenderHTMLQuery(AIS_Target_Data *td) {

	int font_size = 12;



	wxString html;
	wxColor bg = GetBackgroundColour();
	wxColor fg = GetForegroundColour();

	html.Printf(_T("<html><body bgcolor=#%02x%02x%02x><font color=#%02x%02x%02x><center>"), bg.Red(), bg.Green(), bg.Blue(), fg.Red(), fg.Green(), fg.Blue());

	wxString myQueryResult = BuildQueryResult(td);

	html << myQueryResult;

	html << _T("</center></font></body></html>");

	//m_pASMmessages->m_htmlWin->SetFonts( fp_font->GetFaceName(), fp_font->GetFaceName(), sizes );

	wxCharBuffer buf = html.ToUTF8();
	//if (buf.data())                            // string OK?
		//m_pASMmessages->m_htmlWin->SetPage(html);
}

wxString Dlg::BuildQueryResult(AIS_Target_Data *td)
{
	wxString html;
	wxDateTime now = wxDateTime::Now();

	wxString tableStart = _T("\n<table border=0 cellpadding=1 cellspacing=0>\n");
	wxString tableEnd = _T("</table>\n\n");
	wxString rowStart = _T("<tr><td><font size=-2>");
	wxString rowStartH = _T("<tr><td nowrap>");
	wxString rowSeparator = _T("</font></td><td></td><td><b>");
	wxString rowSeparatorH = _T("</td><td></td><td>");
	wxString colSeparator = _T("<td></td>");
	wxString rowEnd = _T("</b></td></tr>\n");
	wxString vertSpacer = _T("<tr><td></td></tr><tr><td></td></tr><tr><td></td></tr>\n\n");

	wxString IMOstr, HECTstr, ClassStr;

	html << tableStart << _T("<tr><td nowrap colspan=2>");




	html << vertSpacer;



	if (IMOstr.Length())
		html << _T("<tr><td colspan=2><table width=100% border=0 cellpadding=0 cellspacing=0>")
		<< rowStart << _("HECT") << _T("</font></td><td>&nbsp;</td><td><font size=-2>")
		<< _("Class") << _T("</font></td><td>&nbsp;</td><td align=right><font size=-2>")
		<< _("IMO") << _T("</font></td></tr>")
		<< rowStartH << _T("<b>") << HECTstr << _T("</b></td><td>&nbsp;</td><td><b>")
		<< ClassStr << _T("</b></td><td>&nbsp;</td><td align=right><b>")
		<< IMOstr << rowEnd << _T("</table></td></tr>");

	else
		html << _T("<tr><td colspan=2><table width=100% border=0 cellpadding=0 cellspacing=0>")
		<< rowStart << _("HECT") << _T("</font></td><td>&nbsp;</td><td align=right><font size=-2>")
		<< _("Class") << _T("</font></td></tr>")
		<< rowStartH << _T("<b>") << HECTstr << _T("</b></td><td>&nbsp;</td><td align=right><b>")
		<< ClassStr << rowEnd << _T("</table></td></tr>");



	html << vertSpacer;

	wxString navStatStr;




	now.MakeGMT();
	int target_age = now.GetTicks();
	//   wxLogMessage(wxString::Format(_T("** PositionReportTicks %ld %ld %d"),
	//                                 now.GetTicks(), PositionReportTicks, target_age));

	html << vertSpacer
		<< rowStart << _("Position") << "ASM" << _T("</font></td><td align=right><font size=-2>")
		<< _("Report Age") << _T("</font></td></tr>")

		<< rowStartH << _T("<b>") << toSDMM(1, td->Lat) << _T("</b></td><td align=right><b>")

		<< rowStartH << _T("<b>") << toSDMM(2, td->Lon) << rowEnd;


	wxString courseStr, sogStr, hdgStr, rotStr, rngStr, brgStr, destStr, etaStr;



	html << _T("</table>");
	return html;
}

void Dlg::CreateControlsMessageList()
{
	int width;
	long lwidth;

	int dx = 20;

	width = dx * 4;
	if (m_pASMmessages1) {
		m_pASMmessages1->m_pListCtrlAISTargets->InsertColumn(tlTRK, _("Hectomtr"), wxLIST_FORMAT_LEFT, width);
		m_pASMmessages1->m_pListCtrlAISTargets->InsertColumn(tlNAME, _("RISindex"), wxLIST_FORMAT_LEFT, width);

	}
}

void Dlg::OnTimer(wxTimerEvent& event)
{
	if (m_bHaveMessageList && m_pASMmessages1->IsShown()) {

			// Refresh AIS target list every 5 seconds to avoid blinking
		if (m_pASMmessages1->m_pListCtrlAISTargets && (0 == (g_tick % (5)))) {
			if (m_bUpdateTarget) {
				UpdateAISTargetList();
				m_bUpdateTarget = false;
				GetParent()->Refresh();
			}
		}
			g_tick++;
	}

	
}


void Dlg::OnSelectMessage(wxCommandEvent& event) {

	mySignalCollection.clear();
	long itemIndex = -1;
	wxString itx;
/*
  while ((itemIndex = m_pASMmessages1->m_pListCtrlAISTargets->GetNextItem(itemIndex,
          wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED)) != wxNOT_FOUND) {
    // Got the selected item index
    itx = m_pASMmessages1->m_pListCtrlAISTargets->GetItemText(itemIndex);
	//wxMessageBox(itx);
  }

  long Pesel;
  itx.ToLong(&Pesel);
  mySignalCollection = FindSignalRISindex(Pesel);
  wxString ris = mySignalCollection.at(0).RISindex;
  double lat = mySignalCollection.at(0).Lat;
  double lon = mySignalCollection.at(0).Lon;

  m_vpppm = m_vp->view_scale_ppm;
  JumpToPosition(lat, lon, m_vpppm);
*/
 // m_vpscale = m_vp->chart_scale;
  
  

  //wxMessageBox(ris);
}

double Dlg::CalculatePPM(float scale) {
    double sc = m_vpscale / scale * m_vpppm;
    return sc;
}

wxString Dlg::StandardPath()
{
    wxString s = wxFileName::GetPathSeparator();
    wxString stdPath  = *GetpPrivateApplicationDataLocation();

    stdPath += s + _T("plugins");
    if (!wxDirExists(stdPath))
      wxMkdir(stdPath);

    stdPath += s + _T("aisMET_HYDRO");

#ifdef __WXOSX__
    // Compatibility with pre-OCPN-4.2; move config dir to
    // ~/Library/Preferences/opencpn if it exists
    {
        wxStandardPathsBase& std_path = wxStandardPathsBase::Get();
        wxString s = wxFileName::GetPathSeparator();
        // should be ~/Library/Preferences/opencpn
        wxString oldPath = (std_path.GetUserConfigDir() +s + _T("plugins") +s + _T("UKTides"));
        if (wxDirExists(oldPath) && !wxDirExists(stdPath)) {
            wxLogMessage("UKTides_pi: moving config dir %s to %s", oldPath, stdPath);
            wxRenameFile(oldPath, stdPath);
        }
    }
#endif

    if (!wxDirExists(stdPath))
      wxMkdir(stdPath);

    stdPath += s;
    return stdPath;
}

