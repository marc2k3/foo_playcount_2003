#include "stdafx.hpp"

class ImportExport
{
public:
	static std::string from_file(metadb_handle_list_cref handles, std::string_view path);
	static void from_file(metadb_handle_list_cref handles);
	static void to_file(metadb_handle_list_cref handles);

private:
	static std::filesystem::path get_fs_path(std::string_view path);
	static std::string read_file(const std::filesystem::path& path);
	static uint32_t get_uint32(JSON& json, uint32_t upper_limit = UINT_MAX);
	static void popup(std::string_view msg);

	static constexpr const char* UTF_8_BOM = "\xEF\xBB\xBF";
};
