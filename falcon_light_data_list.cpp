#include "PTO2_for_BMS.hpp"
#include <vector>
#include <algorithm>

static FlightData flightData;

inline FalconLightData	create_FalconLightData_struct(const char *name, int light_bit, unsigned int *member)
{
	std::string lowercase(name);
	for (char &c : lowercase)
		c = std::tolower(c);

	return FalconLightData{
		.display_name = name,
		.search_name = std::move(lowercase),
		.ID = {
			.offset = reinterpret_cast<char *>(member) - reinterpret_cast<char *>(&flightData),
			.light_bit = light_bit,
		}
	};
}

std::vector<FalconLightData>	init_falcon_light_data_list()
{
	std::vector<FalconLightData>	retval{
		create_FalconLightData_struct("Master Caution",					FlightData::LightBits::MasterCaution,	&flightData.lightBits),

		create_FalconLightData_struct("TF FAIL eyebrow light",			FlightData::LightBits::TF,				&flightData.lightBits),
		create_FalconLightData_struct("OXY LOW eyebrow light",			FlightData::LightBits::OXY_BROW,		&flightData.lightBits),
		create_FalconLightData_struct("EQUIP HOT caution",				FlightData::LightBits::EQUIP_HOT,		&flightData.lightBits),
		//create_FalconLightData_struct("Weight on Wheels",				FlightData::LightBits::ONGROUND,		&flightData.lightBits),
		create_FalconLightData_struct("ENG FIRE eyebrow light",			FlightData::LightBits::ENG_FIRE,		&flightData.lightBits),
		create_FalconLightData_struct("STORES CONFIG caution",			FlightData::LightBits::CONFIG,			&flightData.lightBits),
		create_FalconLightData_struct("HYD/OIL PRESS",					FlightData::LightBits::HYD,				&flightData.lightBits),
		//create_FalconLightData_struct("FLCS ABCD test",				FlightData::LightBits::Flcs_ABCD,		&flightData.lightBits),
		create_FalconLightData_struct("FLCS eyebrow light",				FlightData::LightBits::FLCS,			&flightData.lightBits),
		create_FalconLightData_struct("CANOPY eyebrow light",			FlightData::LightBits::CAN,				&flightData.lightBits),
		create_FalconLightData_struct("TO/LDG CONFIG eyebrow light",	FlightData::LightBits::T_L_CFG,			&flightData.lightBits),

		create_FalconLightData_struct("AoA indexer slow",				FlightData::LightBits::AOAAbove,		&flightData.lightBits),
		create_FalconLightData_struct("AoA indexer on-speed",			FlightData::LightBits::AOAOn,			&flightData.lightBits),
		create_FalconLightData_struct("AoA indexer fast",				FlightData::LightBits::AOABelow,		&flightData.lightBits),

		create_FalconLightData_struct("Refuel RDY light",				FlightData::LightBits::RefuelRDY,		&flightData.lightBits),
		create_FalconLightData_struct("Refuel AR/NWS light",			FlightData::LightBits::RefuelAR,		&flightData.lightBits),
		create_FalconLightData_struct("Refuel DISC light",				FlightData::LightBits::RefuelDSC,		&flightData.lightBits),

		create_FalconLightData_struct("FLCS FAULT caution",				FlightData::LightBits::FltControlSys,	&flightData.lightBits),
		//create_FalconLightData_struct("LE Flaps caution",				FlightData::LightBits::LEFlaps,			&flightData.lightBits),
		create_FalconLightData_struct("ENGINE FAULT caution",			FlightData::LightBits::EngineFault,		&flightData.lightBits),
		create_FalconLightData_struct("OVERHEAT caution",				FlightData::LightBits::Overheat,		&flightData.lightBits),
		//create_FalconLightData_struct("Below BINGO fuel",				FlightData::LightBits::FuelLow,			&flightData.lightBits),
		create_FalconLightData_struct("AVIONICS FAULT caution",			FlightData::LightBits::Avionics,		&flightData.lightBits),
		create_FalconLightData_struct("RADAR ALT caution",				FlightData::LightBits::RadarAlt,		&flightData.lightBits),
		create_FalconLightData_struct("IFF caution",					FlightData::LightBits::IFF,				&flightData.lightBits),
		//create_FalconLightData_struct("????????????????????",			FlightData::LightBits::ECM,				&flightData.lightBits),
		create_FalconLightData_struct("HOOK caution",					FlightData::LightBits::Hook,			&flightData.lightBits),
		create_FalconLightData_struct("NWS FAIL caution",				FlightData::LightBits::NWSFail,			&flightData.lightBits),
		create_FalconLightData_struct("CABIN PRESS caution",			FlightData::LightBits::CabinPress,		&flightData.lightBits),

		//create_FalconLightData_struct("Autopilot ON",					FlightData::LightBits::AutoPilotOn,		&flightData.lightBits),
		create_FalconLightData_struct("TFR STBY light",					FlightData::LightBits::TFR_STBY,		&flightData.lightBits),

		create_FalconLightData_struct("RWR DIAMOND HANDOFF light",		FlightData::LightBits2::HandOff,		&flightData.lightBits2),
		create_FalconLightData_struct("RWR MISSILE LAUNCH light",		FlightData::LightBits2::Launch,			&flightData.lightBits2),
		create_FalconLightData_struct("RWR PRIORITY light",				FlightData::LightBits2::PriMode,		&flightData.lightBits2),
		create_FalconLightData_struct("RWR NAVAL light",				FlightData::LightBits2::Naval,			&flightData.lightBits2),
		create_FalconLightData_struct("RWR UNKNOWN light",				FlightData::LightBits2::Unk,			&flightData.lightBits2),
		create_FalconLightData_struct("RWR TGT SEP light",				FlightData::LightBits2::TgtSep,			&flightData.lightBits2),

		create_FalconLightData_struct("CMDS GO light",				FlightData::LightBits2::Go,				&flightData.lightBits2),
		create_FalconLightData_struct("CMDS NO GO light",			FlightData::LightBits2::NoGo,			&flightData.lightBits2),
		// create_FalconLightData_struct("CMDS ???? light",			FlightData::LightBits2::Degr,			&flightData.lightBits2),
		create_FalconLightData_struct("CMDS DISPENSE RDY light",	FlightData::LightBits2::Rdy,			&flightData.lightBits2),
		create_FalconLightData_struct("CMDS Chaff LO light",		FlightData::LightBits2::ChaffLo,		&flightData.lightBits2),
		create_FalconLightData_struct("CMDS Flare LO light",		FlightData::LightBits2::FlareLo,		&flightData.lightBits2),

		create_FalconLightData_struct("RWR SEARCH light",			FlightData::LightBits2::AuxSrch,		&flightData.lightBits2),
		create_FalconLightData_struct("RWR ACTIVITY light",			FlightData::LightBits2::AuxAct,			&flightData.lightBits2),
		create_FalconLightData_struct("RWR ALT LOW light",			FlightData::LightBits2::AuxLow,			&flightData.lightBits2),
		create_FalconLightData_struct("RWR SYSTEM POWER light",		FlightData::LightBits2::AuxPwr,			&flightData.lightBits2),

		create_FalconLightData_struct("ECM ENBL light",			FlightData::LightBits2::EcmPwr,			&flightData.lightBits2),
		// create_FalconLightData_struct("ECM ???? light",			FlightData::LightBits2::EcmFail,		&flightData.lightBits2),

		create_FalconLightData_struct("FWD FUEL LOW caution",			FlightData::LightBits2::FwdFuelLow,			&flightData.lightBits2),
		create_FalconLightData_struct("AFT FUEL LOW caution",			FlightData::LightBits2::AftFuelLow,			&flightData.lightBits2),

		create_FalconLightData_struct("EPU ON light",			FlightData::LightBits2::EPUOn,			&flightData.lightBits2),
		create_FalconLightData_struct("JFS RUN light",			FlightData::LightBits2::JFSOn,			&flightData.lightBits2),

		create_FalconLightData_struct("SEC caution",				FlightData::LightBits2::SEC,			&flightData.lightBits2),
		create_FalconLightData_struct("OXY LOW caution",			FlightData::LightBits2::OXY_LOW,		&flightData.lightBits2),
		create_FalconLightData_struct("PROBE HEAT caution",			FlightData::LightBits2::PROBEHEAT,		&flightData.lightBits2),
		create_FalconLightData_struct("SEAT NOT ARMED caution",		FlightData::LightBits2::SEAT_ARM,		&flightData.lightBits2),
		create_FalconLightData_struct("BUC caution light",			FlightData::LightBits2::BUC,			&flightData.lightBits2),
		create_FalconLightData_struct("FUEL OIL HOT caution",		FlightData::LightBits2::FUEL_OIL_HOT,	&flightData.lightBits2),
		create_FalconLightData_struct("ANTI SKID caution",			FlightData::LightBits2::ANTI_SKID,		&flightData.lightBits2),

		create_FalconLightData_struct("TFR ACTIVE light",			FlightData::LightBits2::TFR_ENGAGED,	&flightData.lightBits2),
		create_FalconLightData_struct("Gear handle light",			FlightData::LightBits2::GEARHANDLE,		&flightData.lightBits2),
		create_FalconLightData_struct("ENGINE eyebrow light",		FlightData::LightBits2::ENGINE,			&flightData.lightBits2),

		create_FalconLightData_struct("ELEC panel: FLCS PMG light",		FlightData::LightBits3::FlcsPmg,		&flightData.lightBits3),
		create_FalconLightData_struct("ELEC panel: MAIN GEN light",		FlightData::LightBits3::MainGen,		&flightData.lightBits3),
		create_FalconLightData_struct("ELEC panel: STBY GEN light",		FlightData::LightBits3::StbyGen,		&flightData.lightBits3),
		create_FalconLightData_struct("ELEC panel: EPU GEN light",		FlightData::LightBits3::EpuGen,			&flightData.lightBits3),
		create_FalconLightData_struct("ELEC panel: EPU PMG light",		FlightData::LightBits3::EpuPmg,			&flightData.lightBits3),
		create_FalconLightData_struct("ELEC panel: TO FLCS light",		FlightData::LightBits3::ToFlcs,			&flightData.lightBits3),
		create_FalconLightData_struct("ELEC panel: FLCS RLY light",		FlightData::LightBits3::FlcsRly,		&flightData.lightBits3),
		create_FalconLightData_struct("ELEC panel: FAIL light",			FlightData::LightBits3::BatFail,		&flightData.lightBits3),

		create_FalconLightData_struct("EPU HYDRAZN light",		FlightData::LightBits3::Hydrazine,		&flightData.lightBits3),
		create_FalconLightData_struct("EPU AIR light",			FlightData::LightBits3::Air,			&flightData.lightBits3),

		create_FalconLightData_struct("ELEC SYS caution",			FlightData::LightBits3::Elec_Fault,		&flightData.lightBits3),
		// create_FalconLightData_struct("?????????????",			FlightData::LightBits3::Lef_Fault,		&flightData.lightBits3),

		// create_FalconLightData_struct("Weight on Wheels",			FlightData::LightBits3::OnGround,		&flightData.lightBits3),
		create_FalconLightData_struct("FLCS BIT RUN light",			FlightData::LightBits3::FlcsBitRun,			&flightData.lightBits3),
		create_FalconLightData_struct("FLCS BIT FAIL light",		FlightData::LightBits3::FlcsBitFail,		&flightData.lightBits3),
		create_FalconLightData_struct("DBU ON eyebrow light",		FlightData::LightBits3::DbuWarn,			&flightData.lightBits3),
		create_FalconLightData_struct("Nose gear locked light",		FlightData::LightBits3::NoseGearDown,		&flightData.lightBits3),
		create_FalconLightData_struct("Left gear locked light",		FlightData::LightBits3::LeftGearDown,		&flightData.lightBits3),
		create_FalconLightData_struct("Right gear locked light",	FlightData::LightBits3::RightGearDown,		&flightData.lightBits3),
		// create_FalconLightData_struct("Parking brake engaged",			FlightData::LightBits3::ParkBrakeOn,		&flightData.lightBits3),
		// create_FalconLightData_struct("Powered off",			FlightData::LightBits3::Power_Off,		&flightData.lightBits3),

		create_FalconLightData_struct("CADC caution light",	FlightData::LightBits3::cadc,			&flightData.lightBits3),

		create_FalconLightData_struct("Speedbrake out",	FlightData::LightBits3::SpeedBrake,			&flightData.lightBits3),

		create_FalconLightData_struct("RWR SYS TEST ON light",	FlightData::LightBits3::SysTest,	&flightData.lightBits3),

		// create_FalconLightData_struct("N/A",	FlightData::LightBits3::MCAnnounced,	&flightData.lightBits3),

		// create_FalconLightData_struct("Main gear WoW",	FlightData::LightBits3::MLGWOW,	&flightData.lightBits3),
		// create_FalconLightData_struct("Nose gear WoW",	FlightData::LightBits3::NLGWOW,	&flightData.lightBits3),

		create_FalconLightData_struct("ATF NOT ENGAGED caution",	FlightData::LightBits3::ATF_Not_Engaged,	&flightData.lightBits3),

		create_FalconLightData_struct("INLET ICING caution",		FlightData::LightBits3::Inlet_Icing,		&flightData.lightBits3),
	};

	std::stable_sort(retval.begin(), retval.end(), [](auto a, auto b) {return a.display_name < b.display_name;});
	return retval;
}
