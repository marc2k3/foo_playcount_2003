#include "stdafx.hpp"

namespace Component
{
	DECLARE_COMPONENT_VERSION(
		name,
		"0.1.1",
		"Copyright (C) 2024 marc2003\n\n"
		"Build: " __TIME__ ", " __DATE__
	);

	class InstallationValidator : public component_installation_validator
	{
	public:
		bool is_installed_correctly() final
		{
			return test_my_name("foo_playcount_2003.dll") && core_version_info_v2::get()->test_version(2, 1, 0, 0);
		}
	};

	FB2K_SERVICE_FACTORY(InstallationValidator);

	cfg_bool simple_mode(guids::cfg_bool_simple, true);
	cfg_string pin_to(guids::cfg_string_pin_to, "");

	namespace
	{
		advconfig_branch_factory advconfig_branch(name + " (read documentation, changing settings causes data loss)", guids::advconfig_branch, advconfig_branch::guid_branch_tools, 0.0);
		advconfig_string_factory advconfig_pin_to("Title format pattern", guids::advconfig_pin_to, guids::advconfig_branch, 0.0, Component::path_subsong, preferences_state::needs_restart);
		advconfig_radio_factory advconfig_simple("Simple mode", guids::advconfig_simple, guids::advconfig_branch, 1.0, true, preferences_state::needs_restart);
		advconfig_radio_factory advconfig_advanced("Advanced mode", guids::advconfig_advanced, guids::advconfig_branch, 2.0, false, preferences_state::needs_restart);

		void init_stage()
		{
			pfc::string8 pin_to_temp;
			advconfig_pin_to.get(pin_to_temp);

			if (pin_to_temp != pin_to.get() || advconfig_simple.get() != simple_mode)
			{
				pin_to.set(pin_to_temp);
				simple_mode = advconfig_simple.get();
				PlaybackStatistics::api()->erase_orphaned_data(guids::metadb_index);
			}

			MetadbIndex::init();
		}

		FB2K_ON_INIT_STAGE(init_stage, init_stages::after_config_read);
	}
}
