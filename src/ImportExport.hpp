#include "stdafx.hpp"

class ImportExport
{
public:
	static pfc::string8 from_file(metadb_handle_list_cref handles, const pfc::string8& path);
	static void from_file(metadb_handle_list_cref handles);
	static void to_file(metadb_handle_list_cref handles);

private:
	static std::filesystem::path get_fs_path(const pfc::string8& path);
	static std::string read_file(const std::filesystem::path& path);
	static void popup(const pfc::string8& msg);
	static void set_uint32(JSON& json, uint32_t& value, uint32_t upper_limit = UINT_MAX);

	static constexpr const char* UTF_8_BOM = "\xEF\xBB\xBF";
};
