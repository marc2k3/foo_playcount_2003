#pragma once

class PlaybackStatistics
{
public:
	struct Fields
	{
		uint32_t first_played{}, last_played{}, loved{}, playcount{}, rating{}, added{};
		pfc::string8 timestamps = "[]";

		operator bool() const
		{
			if (loved || rating || playcount) return true;
			if (Component::simple_mode) return first_played || last_played;
			return false;
		}
	};

	using HashList = pfc::list_t<metadb_index_hash>;
	using HashSet = std::set<metadb_index_hash>;

	static Fields get_fields(metadb_index_hash hash);
	static HashSet get_hashes(metadb_handle_list_cref handles);
	static bool update_value(uint32_t new_value, uint32_t& old_value);
	static metadb_index_manager_v2::ptr api();
	static pfc::string8 timestamp_to_string(uint64_t ts);
	static uint32_t get_total_playcount(metadb_handle_list_cref handles, track_property_provider_v5_info_source& source);
	static uint32_t get_year(uint64_t ts);
	static uint32_t now();
	static uint32_t playcount_year(const Fields& f, bool last_year);
	static uint32_t string_to_timestamp(std::string_view str);
	static uint32_t windows_to_unix(uint64_t ts);
	static void clear(metadb_handle_list_cref handles);
	static void on_item_played(const metadb_handle_ptr& handle);
	static void refresh(const HashList& to_refresh);
	static void set_fields(const metadb_index_transaction::ptr& ptr, metadb_index_hash hash, const Fields& f);
};
