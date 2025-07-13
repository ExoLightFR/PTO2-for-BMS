#include <fstream>
#include <filesystem>

#include "nlohmann/json_fwd.hpp"
#include "nlohmann/json.hpp"
#include "PTO2_for_BMS.hpp"

Json	PTO2_mapping_to_json(PTO2LightBinds const& mapping)
{
	Json conf;

	auto const &gear_mapping = mapping[GEAR_HANDLE_BRIGHTNESS];
	if (gear_mapping.has_value())
	{
		conf["1"] = {
			{ "offset", gear_mapping->ID.offset },
			{ "light_bit", gear_mapping->ID.light_bit },
		};
	}
	else
		conf["1"] = nullptr;

	for (int i = MASTER_CAUTION; i <= HOOK; ++i)
	{
		std::string index = std::to_string(i);
		if (mapping[i].has_value())
		{
			conf[index] = {
				{ "offset", mapping[i]->ID.offset },
				{ "light_bit", mapping[i]->ID.light_bit },
			};
		}
		else
			conf[index] = nullptr;
	}
	return conf;
}

PTO2LightBinds	json_to_PTO2_mapping(Json const &conf)
{
	PTO2LightBinds	mapping = {};	// Populated by empty optionals at construction

	if (!conf.contains("light_mapping") || !conf["light_mapping"].is_object())
		return mapping;

	for (auto const &[key, value] : conf["light_mapping"].items())
	{
		try {
			int index = std::stoi(key);
			if (!value.at("offset").is_number_unsigned() || !value.at("light_bit").is_number_unsigned())
			{
				// Just continue in case of error, item will be an empty optional
				continue;
			}
			FalconLightData::LightID lightID = {
				.offset = value["offset"], .light_bit = value["light_bit"]
			};
			// Find light data that matches the light ID given in json
			for (auto const &light : g_context.falcon_lights)
			{
				if (light.ID == lightID)
					mapping[index] = light;
			}
		}
		catch (std::exception const &) {
			// Just continue in case of error, item will be an empty optional
			continue;
		}
	}
	return mapping;
}

/*
* Get all of the paths of configuration files, sorted by version number. Conf files are
* located in every detected BMS install's /User/Config directory.
* The resulting vector ALWAYS ends with the PWD of the executable. This is meant to be the
* "last resort" in case no BMS install is detected.
* For example, if you have BMS 4.37 & 4.38, the resulting vector would be:
* { "path_to_4.37", "path_to_4.38", "PWD" }
*/
std::vector<ConfFileLocation>	get_conf_file_paths()
{
	namespace fs = std::filesystem;

	std::vector<ConfFileLocation> installs;
	const wchar_t *possible_BMS_installs[] = {
		L"Falcon BMS 4.36",
		L"Falcon BMS 4.37",
		L"Falcon BMS 4.38", // Soon™
		L"Falcon BMS 4.39",	// Inshallah this will be future proof
	};

	std::wstring bms_reg_path_prefix = REG_BENCHMARKSIMS_PATH;
	for (const wchar_t *suffix : possible_BMS_installs)
	{
		std::wstring bms_root;
		if (RegGetString(HKEY_LOCAL_MACHINE, (bms_reg_path_prefix + suffix).c_str(), L"baseDir",
			bms_root) == 0)
		{
			// This API looks cool, until you actually use it. God dammit, C++ committee.
			fs::path conf_path = bms_root;
			conf_path = conf_path / "User" / "Config";
			if (fs::is_directory(conf_path))
			{
				char name[] = "BMS X.YY";
// Yes, it is "unsafe". It's fine here though.
#pragma warning(suppress : 4996)
				std::wcstombs(name, suffix + 7, sizeof(name));
				name[8] = '\0'; // NULL-terminate, just in case.
				installs.emplace_back(ConfFileLocation{
					.install_name = name,
					.full_path = conf_path / CONF_FILE_NAME
				});
			}
		}
	}
	// Append PWD to the end of the vector as a fallback option
	installs.emplace_back(ConfFileLocation{
		.install_name = "PWD",
		.full_path = fs::current_path() / CONF_FILE_NAME
	});

	return installs;
}

void	serialize_settings_to_conf_file(PTO2LightBinds const &mapping)
{
	std::ofstream conf_file(g_context.get_selected_conf().full_path);
	Json conf;
	conf.emplace("light_mapping", PTO2_mapping_to_json(mapping));
	conf["retro_mode"] = g_context.retro_mode;
	conf_file << conf.dump();
}

void	deserialize_conf_to_settings()
{
	try {
		std::ifstream conf_file(g_context.get_selected_conf().full_path);
		Json conf;
		conf_file >> conf;
		if (conf.contains("retro_mode") && conf["retro_mode"].is_boolean())
			g_context.retro_mode = conf["retro_mode"];
		g_context.PTO2_light_assignment_map = json_to_PTO2_mapping(conf);
	}
	catch (std::exception const &) {
		g_context.PTO2_light_assignment_map = {};
	}
}

/*
* This default config assigns:
* - Gear handle light
* - Master caution
* - "Three greens"
* - Hook warning light (because why the hell not?)
*/
static const char *DEFAULT_PTO2_JSON_CONF = R"(
{
	"light_mapping": {
		"1": {
			"light_bit": 1073741824,
			"offset": 124
		},
		"10": null,
		"11": null,
		"12": {
			"light_bit": 65536,
			"offset": 128
		},
		"13": null,
		"14": {
			"light_bit": 262144,
			"offset": 128
		},
		"15": {
			"light_bit": 131072,
			"offset": 128
		},
		"16": null,
		"17": {
			"light_bit": 134217728,
			"offset": 108
		},
		"4": {
			"light_bit": 1,
			"offset": 108
		},
		"5": null,
		"6": null,
		"7": null,
		"8": null,
		"9": null
	},
	"retro_mode": false
}
)";

/*
* For every detected BMS install, create a config file if it does not exist.
* If there is at least one BMS install, select it, and load its config file.
* If no BMS install is found, create a conf file in PWD and load it.
*/
void	init_settings_and_conf_file()
{
	PTO2LightBinds	mapping;

	// Initialize conf files into all BMS installs if they do not exist yet
	for (int i = 0; i < g_context.conf_file_paths.size() - 1; ++i)
	{
		auto &conf_file_path = g_context.conf_file_paths[i];
		if (!std::filesystem::exists(conf_file_path.full_path))
		{
			Json conf = Json::parse(DEFAULT_PTO2_JSON_CONF);
			g_context.PTO2_light_assignment_map = json_to_PTO2_mapping(conf);
			g_context.retro_mode = false;
			std::ofstream conf_file(conf_file_path.full_path);
			conf_file << conf.dump();
		}
	}

	// If there is at least one BMS install detected, select the last possible install
	// (which would be the most recent). Otherwise, leave this variable initialized to 0.
	if (g_context.conf_file_paths.size() >= 2)
		g_context.selected_conf_file_idx = g_context.conf_file_paths.size() - 2;

	// If no BMS install is found, fallback to PWD (which would be the last item in the vector).
	// Check if file exists in PWD, if not, create it.
	if (g_context.conf_file_paths.size() == 1
		&& !std::filesystem::exists(g_context.conf_file_paths.back().full_path))
	{
		Json conf = Json::parse(DEFAULT_PTO2_JSON_CONF);
		g_context.PTO2_light_assignment_map = json_to_PTO2_mapping(conf);
		g_context.retro_mode = false;
		std::ofstream conf_file(g_context.conf_file_paths.back().full_path);
		conf_file << conf.dump();
	}
	// At least one file is found (in PWD or BMS), deserialize it
	else
	{
		deserialize_conf_to_settings();
	}
}
