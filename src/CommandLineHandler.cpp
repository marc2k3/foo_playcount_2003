#include "stdafx.hpp"
#include "ImportExport.hpp"

namespace
{
	class CommandLineHandler : public commandline_handler
	{
	public:
		result on_token(const char* token) final
		{
			static constexpr std::string_view prefix = "/playcount_2003_import:";
			const std::string s = token;

			if (s.starts_with(prefix))
			{
				const auto filename = s.substr(prefix.length());
				const auto path = full_path(filename);

				metadb_handle_list items;
				library_manager::get()->get_all_items(items);
				const auto msg = ImportExport::from_file(items, path);
				FB2K_console_print(Component::name.data(), ": Command line import: ", msg.c_str());
				return RESULT_PROCESSED;
			}

			return RESULT_NOT_OURS;
		}

	private:
		std::string full_path(std::string_view path)
		{
			static const auto profile_path = core_api::get_profile_path();

			pfc::string8 full_path;
			filesystem::g_get_display_path(pfc::io::path::combine(profile_path, path), full_path);
			return full_path.get_ptr();
		}
	};

	FB2K_SERVICE_FACTORY(CommandLineHandler);
}
