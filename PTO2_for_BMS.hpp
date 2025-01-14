#pragma once

#include <thread>
#include <atomic>
#include <vector>
#include <array>
#include <optional>

#include <Windows.h>

#include "nlohmann/json_fwd.hpp"
#include "nlohmann/json.hpp"
#include "imgui.h"
#include "hidapi.h"
#include "FlightData.h"
#include "resource.h"

using Json = nlohmann::json;

constexpr int	WINDOW_ICON_ID_GREEN	= IDI_ICON1;
constexpr int	WINDOW_ICON_ID_RED		= IDI_ICON2;

// Unique ID for the system tray icon.
constexpr UINT	NOTIFY_ICON_DATA_ID		= 42;
// Message ID for custom event in WndProc, in this case, for our system tray icon messages
constexpr UINT	WM_TRAYICON				= WM_USER + 1;
// System tray icon -> popup menu element IDs
constexpr WORD	ID_TRAY_MENU_QUIT		= 1;
constexpr WORD	ID_TRAY_MENU_SEPARATOR	= 2;
constexpr WORD	ID_TRAY_MENU_CONNECT	= 3;
constexpr WORD	ID_TRAY_MENU_MINIMIZE	= 4;

constexpr const wchar_t		REG_BENCHMARKSIMS_PATH[] = L"SOFTWARE\\WOW6432Node\\Benchmark Sims\\";
constexpr const char		CONF_FILE_NAME[] = "PTO2_lights.conf";

constexpr const char		WIN_TITLE[] = "Winwing PTO2 for Falcon BMS";
constexpr int				WIN_WIDTH = 610;
constexpr int				WIN_HEIGHT = 570;
constexpr unsigned short	PTO2_VENDOR_ID = 0x4098;
constexpr unsigned short	PTO2_PRODUCT_ID = 0xbf05;
constexpr auto				THREAD_SLEEP_INTERVAL = std::chrono::milliseconds(100);

// Roboto font embeded into the binary for ImGui
extern const char Roboto_Medium_compressed_data_base85[148145 + 1];

enum PTO2LightID : unsigned char {
	/* Brightness has 256 values, from 0x00 to 0xff */
	BACKLIGHT_BRIGHTNESS = 0x00,	// Brightness of panel backlight
	GEAR_HANDLE_BRIGHTNESS,			// Gear handle has variable brightness, not on/off
	SL_BRIGHTNESS,					// Buttons brightness (master caution, station select...)
	FLAG_BRIGHTNESS,				// Flags brightness (NOSE/LEFT/RIGHT, HOOK...)
	/* Light have two states, off and on, either 0x00 or 0x01.
	Their brightness is controlled by SL or FLAG_BRIGHTNESS */
	MASTER_CAUTION,
	JETTISON,
	STATION_CTR,
	STATION_LI,
	STATION_LO,
	STATION_RO,
	STATION_RI,
	FLAPS,
	NOSE,
	FULL,
	RIGHT,
	LEFT,
	HALF,
	HOOK
};

struct FalconLightData
{
	std::string	display_name;
	std::string	search_name;	// Unused. Meant to be a lowercase version of the display name.
	struct LightID {
		ptrdiff_t	offset;			// Offset in shared memory in which to read
		int			light_bit;		// Light bit to check against
		bool operator==(LightID const &) const = default;
	} ID;
};
std::vector<FalconLightData>	init_falcon_light_data_list();

using PTO2LightBinds = std::array< std::optional<FalconLightData>, HOOK + 1 >;
PTO2LightBinds	init_PTO2_light_map_and_conf_file();

struct Context
{
	hid_device			*hid_device = nullptr;

	struct GLFWwindow	*glfw_window = nullptr;

	std::jthread		thread;
	std::atomic_bool	thread_running = false;
	// Not pretty but I don't really care. Asks for the main thread to close and reopen the HID device.
	// Used in case of hid_write error.
	std::atomic_bool	require_device_reopen = false;

	const std::vector<FalconLightData>	falcon_lights = init_falcon_light_data_list();
	// Array that maps PTO2 lights to a Falcon LightID (shared memory offset + bit to check)
	PTO2LightBinds	PTO2_light_assignment_map = init_PTO2_light_map_and_conf_file();

	struct {
		unsigned int	min_y = 0;		// GLFW minimum window height
		unsigned int	imgui_y = 0;	// ImGui main window height (even if clipped by GLFW window)
	} window_sizes;
};

ImGuiStyle		get_custom_imgui_style(float scale_factor);
void			set_ImGui_scaling_from_DPI(UINT new_dpi);
void			thread_routine();
void			render_main_window(ImGuiIO &io);

// ImGui custom widgets
namespace widgets {
	bool		ColoredButton(const char *label, ImColor color, const ImVec2 &size = ImVec2(0, 0));
	void		TextCentered(const char *text);
	bool		PTO2_light_assign(const char *label, PTO2LightID PTO_light_ID,
		int popup_max_height_in_items = -1);
	void		PTO2_firmware_warning_modal();
}

// Serialisation for JSON config file
Json			PTO2_mapping_to_json(PTO2LightBinds const &mapping);
PTO2LightBinds	json_to_PTO2_mapping(Json const &conf);
void			serialize_PTO2_mapping_to_conf_file(PTO2LightBinds const &mapping);
PTO2LightBinds	deserialize_conf_to_PTO2_mapping();

// Win32 utils, stuff like window/system tray icon, registry...
void			set_window_icon(int IDI_thing);
void			add_tray_icon(int IDI_thingy);
void			change_tray_icon(int IDI_thingy);
void			remove_tray_icon();
std::wstring	RegGetString(HKEY hKey, const std::wstring &subKey, const std::wstring &value);
int				RegGetString(HKEY hKey, const std::wstring &subKey, const std::wstring &value, std::wstring &outstr);

/*
* Enumeration of all cockpit data that can be checked by the app to synchronise a PTO2 light.
* Unused (for now at least).
*/
enum FalconPitData
{
	LIGHT_MASTER_CAUTION,
	LIGHT_TF,   // Left eyebrow
	LIGHT_OXY_BROW,   // repurposed for eyebrow OXY LOW (was OBS, unused)
	LIGHT_EQUIP_HOT,   // Caution light; repurposed for cooling fault (was: not used)
	LIGHT_ONGROUND,  // True if on ground: this is not a lamp bit!
	LIGHT_ENG_FIRE,  // Right eyebrow; upper half of split face lamp
	LIGHT_CONFIG,  // Stores config, caution panel
	LIGHT_HYD,  // Right eyebrow; see also OIL (this lamp is not split face)
	LIGHT_Flcs_ABCD, // TEST panel FLCS channel lamps; repurposed, was OIL (see HYD; that lamp is not split face)
	LIGHT_FLCS, // Right eyebrow; was called DUAL which matches block 25, 30/32 and older 40/42
	LIGHT_CAN, // Right eyebrow
	LIGHT_T_L_CFG, // Right eyebrow

	// AOA Indexers
	LIGHT_AOAAbove  ,
	LIGHT_AOAOn     ,
	LIGHT_AOABelow  ,

	// Refuel/NWS
	LIGHT_RefuelRDY ,
	LIGHT_RefuelAR  ,
	LIGHT_RefuelDSC ,

	// Caution Lights
	LIGHT_FltControlSys,
	LIGHT_LEFlaps,
	LIGHT_EngineFault,
	LIGHT_Overheat,
	LIGHT_FuelLow,
	LIGHT_Avionics,
	LIGHT_RadarAlt,
	LIGHT_IFF,
	LIGHT_ECM,
	LIGHT_Hook,
	LIGHT_NWSFail,
	LIGHT_CabinPress,

	// XXX: If this is not a lamp bit then what in the god damned fuck is it then sir?
	// AutoPilotOn,  // TRUE if is AP on.  NB: This is not a lamp bit!
	LIGHT_TFR_STBY,  // MISC panel; lower half of split face TFR lamp

	// Threat Warning Prime
	LIGHT_HandOff = 0x1,
	LIGHT_Launch  = 0x2,
	LIGHT_PriMode = 0x4,
	LIGHT_Naval   = 0x8,
	LIGHT_Unk     = 0x10,
	LIGHT_TgtSep  = 0x20,

	// EWS
	LIGHT_Go      = 0x40,     // On and operating normally
	LIGHT_NoGo    = 0x80,     // On but malfunction present
	LIGHT_Degr    = 0x100,    // Status message: AUTO DEGR
	LIGHT_Rdy     = 0x200,    // Status message: DISPENSE RDY
	LIGHT_ChaffLo = 0x400,    // Bingo chaff quantity reached
	LIGHT_FlareLo = 0x800,    // Bingo flare quantity reached

	// Aux Threat Warning
	LIGHT_AuxSrch = 0x1000,
	LIGHT_AuxAct  = 0x2000,
	LIGHT_AuxLow  = 0x4000,
	LIGHT_AuxPwr  = 0x8000,

	// ECM
	LIGHT_EcmPwr  = 0x10000,
	LIGHT_EcmFail = 0x20000,

	// Caution Lights
	LIGHT_FwdFuelLow = 0x40000,
	LIGHT_AftFuelLow = 0x80000,

	LIGHT_EPUOn      = 0x100000,  // EPU panel; run light
	LIGHT_JFSOn      = 0x200000,  // Eng Jet Start panel; run light

	// Caution panel
	LIGHT_SEC          = 0x400000,
	LIGHT_OXY_LOW      = 0x800000,
	LIGHT_PROBEHEAT    = 0x1000000,
	LIGHT_SEAT_ARM     = 0x2000000,
	LIGHT_BUC          = 0x4000000,
	LIGHT_FUEL_OIL_HOT = 0x8000000,
	LIGHT_ANTI_SKID    = 0x10000000,

	LIGHT_TFR_ENGAGED  = 0x20000000,  // MISC panel; upper half of split face TFR lamp
	LIGHT_GEARHANDLE   = 0x40000000,  // Lamp in gear handle lights on fault or gear in motion
	LIGHT_ENGINE       = 0x80000000,  // Lower half of right eyebrow ENG FIRE/ENGINE lamp

	// Used with the MAL/IND light code to light up "everything"
	// please update this if you add/change bits!
	// AllLampBits2On = 0xFFFFF03F,
	// AllLampBits2OnExceptCarapace = AllLampBits2On ^ HandOff ^ Launch ^ PriMode ^ Naval ^ Unk ^ TgtSep ^ AuxSrch ^ AuxAct ^ AuxLow ^ AuxPwr

	// Elec panel
	LIGHT_FlcsPmg,
	LIGHT_MainGen,
	LIGHT_StbyGen,
	LIGHT_EpuGen,
	LIGHT_EpuPmg,
	LIGHT_ToFlcs,
	LIGHT_FlcsRly,
	LIGHT_BatFail,

	// EPU panel
	LIGHT_Hydrazine ,
	LIGHT_Air       ,

	// Caution panel
	LIGHT_Elec_Fault,
	LIGHT_Lef_Fault ,

	OnGround            ,   // weight-on-wheels
	LIGHT_FlcsBitRun    ,   // FLT CONTROL panel RUN light (used to be Multi-engine fire light)
	LIGHT_FlcsBitFail   ,   // FLT CONTROL panel FAIL light (used to be Lock light Cue; non-F-16)
	LIGHT_DbuWarn       ,   // Right eyebrow DBU ON cell; was Shoot light cue; non-F16
	LIGHT_NoseGearDown  ,  // Landing gear panel; on means down and locked
	LIGHT_LeftGearDown  ,  // Landing gear panel; on means down and locked
	LIGHT_RightGearDown ,  // Landing gear panel; on means down and locked
	LIGHT_ParkBrakeOn   , // Parking brake engaged; NOTE: not a lamp bit
	Power_Off     , // Set if there is no electrical power.  NB: not a lamp bit

	// Caution panel
	LIGHT_cadc,

	// Left Aux console
	SpeedBrake, // True if speed brake is in anything other than stowed position

	// Threat Warning Prime - additional bits
	LIGHT_SysTest,

	// Master Caution WILL come up (actual lightBit has 3sec delay like in RL),
	// usable for cockpit builders with RL equipment which has a delay on its own.
	// Will be set to false again as soon as the MasterCaution bit is set.
	MCAnnounced,

	//MLGWOW is only for AFM , it means WOW switches on MLG are triggered => FLCS switches to WOWPitchRockGain
	MLGWOW,
	NLGWOW,

	LIGHT_ATF_Not_Engaged,

	// Caution panel
	Inlet_Icing,

	// Free bits in LightBits3
	//0x40000000,
	//0x80000000,

	// Used with the MAL/IND light code to light up "everything"
	// please update this if you add/change bits!
	// AllLampBits3On = 0x3147EFFF,
	// AllLampBits3OnExceptCarapace = AllLampBits3On ^ SysTest
};

extern Context	g_context;
