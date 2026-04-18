/**
 * globals_stub.cpp – Defines all extern global variables declared in
 * external_globals.h so the test executable links successfully.
 * Only the variables actually exercised by the tests need non-trivial values;
 * the rest are zero/null initialised.
 */

// Include stubs first so Windows types are available everywhere.
#include <windows.h>

#include <string>
#include <memory>

// Stub dialog / NPP types (from test stubs)
#include "ui/dialogs/LoaderDlg.h"
#include "ui/dialogs/ChatSettingsDlg.h"

// Bring in PluginInterface for NppData / FuncItem
#include "PluginInterface.h"

// ---- Global variables required by external_globals.h ----

NppData          nppData              = {};
HANDLE           _hModule             = nullptr;
TCHAR            iniFilePath[MAX_PATH]          = {};
TCHAR            instructionsFilePath[MAX_PATH] = {};
LoaderDlg        _loaderDlg;
ChatSettingsDlg  _chatSettingsDlg;
FuncItem         funcItem[1]          = {};
bool             isKeepQuestion       = false;
bool             debugMode            = false;

// API configuration values – tests may change these directly.
std::wstring configAPIValue_secretKey;
std::wstring configAPIValue_baseURL;
std::wstring configAPIValue_chatRoute;
std::wstring configAPIValue_responseType;
std::wstring configAPIValue_proxyURL;
std::wstring configAPIValue_model;
std::wstring configAPIValue_instructions;
std::wstring configAPIValue_temperature;
std::wstring configAPIValue_maxTokens;
std::wstring configAPIValue_topP;
std::wstring configAPIValue_frequencyPenalty;
std::wstring configAPIValue_presencePenalty;
std::wstring configAPIValue_streaming;
std::wstring configAPIValue_showReasoning;   // key value: L"0" or L"1"
std::wstring configAPIValue_keepAlive;

HWND s_streamTargetScintilla = nullptr;

// Service interfaces – null for unit tests
namespace UIServices
{
    class IUIService;
    class IConfigurationService;
    class IMenuService;
    class INotepadService;
}
std::shared_ptr<UIServices::IUIService>           g_globalUIService;
std::shared_ptr<UIServices::IConfigurationService> g_globalConfigService;
std::shared_ptr<UIServices::IMenuService>          g_globalMenuService;
std::shared_ptr<UIServices::INotepadService>       g_globalNotepadService;
