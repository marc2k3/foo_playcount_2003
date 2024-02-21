#include "stdafx.hpp"
#include "ImportExport.hpp"

namespace
{
	class CommandLineHandler : public commandline_handler
	{
	public:
		result on_token(const char* token) final
		{
			static const string8 prefix = "/playcount_2003_import:";
			const string8 s = token;

			if (s.startsWith(prefix))
			{
				const auto filename = s.subString(prefix.get_length());
				const auto path = full_path(filename);

				metadb_handle_list items;
				library_manager::get()->get_all_items(items);
				const auto msg = ImportExport::from_file(items, path);
				FB2K_console_print(Component::name, ": Command line import: ", msg);
				return RESULT_PROCESSED;
			}

			return RESULT_NOT_OURS;
		}

	private:
		string8 full_path(const string8& path)
		{
			static const auto profile_path = core_api::get_profile_path();
			string8 full_path;
			filesystem::g_get_display_path(pfc::io::path::combine(profile_path, path), full_path);
			return full_path;
		}
	};

	FB2K_SERVICE_FACTORY(CommandLineHandler)
}
