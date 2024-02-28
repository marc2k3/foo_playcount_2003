#include "stdafx.hpp"

PlaybackStatistics::Fields PlaybackStatistics::get_fields(metadb_index_hash hash)
{
	mem_block_container_impl temp;
	api()->get_user_data(guids::metadb_index, hash, temp);
	if (temp.get_size() > 0)
	{
		try
		{
			Fields f;
			stream_reader_formatter_simple_ref reader(temp.get_ptr(), temp.get_size());

			reader >> f.first_played;
			reader >> f.last_played;
			reader >> f.loved;
			reader >> f.playcount;
			reader >> f.rating;
			reader >> f.added;

			if (reader.get_remaining() > 0)
			{
				reader >> f.timestamps;
			}

			return f;
		}
		catch (exception_io_data) {}
	}
	return Fields();
}

PlaybackStatistics::HashSet PlaybackStatistics::get_hashes(metadb_handle_list_cref handles)
{
	HashSet hashes;
	auto client = MetadbIndex::client();

	for (auto&& handle : handles)
	{
		metadb_index_hash hash{};
		if (client->hashHandle(handle, hash))
		{
			hashes.emplace(hash);
		}
	}

	return hashes;
}

bool PlaybackStatistics::update_value(uint32_t new_value, uint32_t& old_value)
{
	if (new_value == UINT_MAX || new_value == old_value) return false;

	old_value = new_value;
	return true;
}

metadb_index_manager_v2::ptr PlaybackStatistics::api()
{
	static metadb_index_manager_v2* cached = metadb_index_manager_v2::get().detach();
	return cached;
}

string8 PlaybackStatistics::timestamp_to_string(uint64_t ts)
{
	const uint64_t windows_time = pfc::fileTimeUtoW(ts);
	return pfc::format_filetimestamp(windows_time);
}

uint32_t PlaybackStatistics::get_total_playcount(metadb_handle_list_cref handles, track_property_provider_v5_info_source& source)
{
	HashSet hash_set;
	uint32_t total{};

	const size_t count = handles.get_count();
	auto client = MetadbIndex::client();

	for (const size_t i : std::views::iota(0U, count))
	{
		auto rec = source.get_info(i);
		if (rec.info.is_empty()) continue;

		const auto hash = client->transform(rec.info->info(), handles[i]->get_location());
		if (hash_set.emplace(hash).second)
		{
			total += get_fields(hash).playcount;
		}
	}

	return total;
}

uint32_t PlaybackStatistics::get_year(uint32_t ts)
{
	const auto windows_time = pfc::fileTimeUtoW(ts);

	try
	{
		FILETIME ft;
		if (FileTimeToLocalFileTime((FILETIME*)&windows_time, &ft))
		{
			SYSTEMTIME st;
			if (FileTimeToSystemTime(&ft, &st))
			{
				return st.wYear;
			}
		}
	}
	catch (...) {}

	return 0;
}

uint32_t PlaybackStatistics::now()
{
	return static_cast<uint32_t>(pfc::fileTimeWtoU(pfc::fileTimeNow()));
}

uint32_t PlaybackStatistics::playcount_year(const Fields& f, bool last_year)
{
	if (f.playcount == 0) return 0;

	const auto now_ts = now();
	auto target_year = get_year(now_ts);
	if (last_year) target_year--;

	const auto timestamps = JSONHelper::get_timestamps_array(f) | std::views::reverse;
	uint32_t count{};

	for (auto&& timestamp : timestamps)
	{
		const auto play_year = get_year(timestamp.get<uint32_t>());
		if (play_year < target_year) break;
		if (play_year == target_year) count++;
	}

	return count;
}

uint32_t PlaybackStatistics::string_to_timestamp(const string8& str)
{
	static const auto lower_limit = pfc::fileTimeUtoW(1);
	static const auto upper_limit = pfc::fileTimeUtoW(UINT_MAX - 1);

	if (str.empty()) return UINT_MAX;
	if (str == "0") return 0; // special handling for edit dialog

	const auto windows_time = pfc::filetimestamp_from_string(str);
	if (windows_time == filetimestamp_invalid || windows_time < lower_limit || windows_time > upper_limit) return UINT_MAX;

	return static_cast<uint32_t>(pfc::fileTimeWtoU(windows_time));
}

void PlaybackStatistics::clear(metadb_handle_list_cref handles)
{
	HashList to_refresh;
	const auto hashes = get_hashes(handles);
	auto ptr = api()->begin_transaction();

	static const Fields f;

	for (auto&& hash : hashes)
	{
		set_fields(ptr, hash, f);
		to_refresh.add_item(hash);
	}

	ptr->commit();
	refresh(to_refresh);
}

void PlaybackStatistics::on_item_played(const metadb_handle_ptr& handle)
{
	metadb_index_hash hash{};
	if (MetadbIndex::client()->hashHandle(handle, hash))
	{
		const auto ts = now();
		auto f = get_fields(hash);
		f.last_played = ts;

		if (f.first_played == 0)
		{
			f.first_played = ts;
		}

		if (Component::simple_mode)
		{
			f.playcount += 1;
		}
		else
		{
			auto timestamps = JSONHelper::get_timestamps_array(f);
			timestamps.emplace_back(ts);

			f.timestamps = timestamps.dump().c_str();
			f.playcount = static_cast<uint32_t>(timestamps.size());
		}

		auto ptr = api()->begin_transaction();
		set_fields(ptr, hash, f);
		ptr->commit();
		api()->dispatch_refresh(guids::metadb_index, hash);
	}
}

void PlaybackStatistics::refresh(const HashList& to_refresh)
{
	if (to_refresh.get_count() > 0)
	{
		api()->dispatch_refresh(guids::metadb_index, to_refresh);
	}
}

void PlaybackStatistics::set_fields(const metadb_index_transaction::ptr& ptr, metadb_index_hash hash, const Fields& f)
{
	stream_writer_formatter_simple writer;
	writer << f.first_played;
	writer << f.last_played;
	writer << f.loved;
	writer << f.playcount;
	writer << f.rating;
	writer << f.added;
	writer << f.timestamps;

	ptr->set_user_data(guids::metadb_index, hash, writer.m_buffer.get_ptr(), writer.m_buffer.get_size());
}
