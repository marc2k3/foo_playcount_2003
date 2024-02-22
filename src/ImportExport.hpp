#include "stdafx.hpp"

class ImportExport
{
public:
	static string8 from_file(metadb_handle_list_cref handles, const string8& path);
	static void from_file(metadb_handle_list_cref handles);
	static void to_file(metadb_handle_list_cref handles);

private:
	static JSON create_json_array(const std::vector<uint32_t>& v);
	static std::filesystem::path get_fs_path(const string8& path);
	static std::string read_file(const std::filesystem::path& path);
	static uint32_t get_uint32(JSON& json, uint32_t upper_limit = UINT_MAX);
	static void popup(const string8& msg);

	static constexpr const char* UTF_8_BOM = "\xEF\xBB\xBF";
};
