#include "stdafx.hpp"
#include "ImportExport.hpp"

namespace
{
	static const std::vector<Component::MenuItem> main_items =
	{
		{ &guids::mainmenu_file_import, "Import from file" },
		{ &guids::mainmenu_file_export, "Export to file" },
	};

	class MainMenu : public mainmenu_commands
	{
	public:
		GUID get_command(uint32_t index) final
		{
			if (index >= main_items.size()) FB2K_BugCheck();

			return *main_items[index].guid;
		}

		GUID get_parent() final
		{
			return guids::mainmenu_group;
		}

		bool get_description(uint32_t index, pfc::string_base& out) final
		{
			if (index >= main_items.size()) FB2K_BugCheck();

			get_name(index, out);
			return true;
		}

		bool get_display(uint32_t index, pfc::string_base& out, uint32_t& flags) final
		{
			if (index >= main_items.size()) FB2K_BugCheck();

			if (get_library_items().get_count() == 0) flags = flag_disabled;
			get_name(index, out);
			return true;
		}

		uint32_t get_command_count() final
		{
			return static_cast<uint32_t>(main_items.size());
		}

		void execute(uint32_t index, service_ptr_t<service_base>) final
		{
			if (index >= main_items.size()) FB2K_BugCheck();

			const auto items = get_library_items();
			if (items.get_count() == 0) return;

			switch (index)
			{
			case 0:
				if (prompt())
				{
					PlaybackStatistics::clear(items);
					ImportExport::from_file(items);
				}
				break;
			case 1:
				ImportExport::to_file(items);
				break;
			}
		}

		void get_name(uint32_t index, pfc::string_base& out) final
		{
			if (index >= main_items.size()) FB2K_BugCheck();
			out = main_items[index].name;
		}

	private:
		bool prompt()
		{
			return popup_message_v3::get()->messageBox(core_api::get_main_window(), "Existing Media Library data will be cleared. Continue?", Component::name, MB_YESNO) == IDYES;
		}

		metadb_handle_list get_library_items()
		{
			metadb_handle_list items;
			library_manager::get()->get_all_items(items);
			return items;
		}
	};

	static mainmenu_group_popup_factory g_mainmenu_group(guids::mainmenu_group, mainmenu_groups::library, mainmenu_commands::sort_priority_base, Component::name);
	FB2K_SERVICE_FACTORY(MainMenu);
}
