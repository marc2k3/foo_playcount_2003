#include "stdafx.hpp"
#include "ImportExport.hpp"

pfc::string8 ImportExport::from_file(metadb_handle_list_cref handles, const pfc::string8& path)
{
	const auto fs_path = get_fs_path(path);

	std::error_code ec;
	if (!std::filesystem::is_regular_file(fs_path))
	{
		return "File not found.";
	}

	const std::string content = read_file(fs_path);

	if (content.empty())
	{
		return "The selected file was empty or there was an unexpected read error.";
	}

	auto records = JSON::parse(content, nullptr, false);
	if (!records.is_array())
	{
		return "Unexpected JSON parse error.";
	}

	PlaybackStatistics::HashList to_refresh;
	const auto now = PlaybackStatistics::now();
	auto hashes = PlaybackStatistics::get_hashes(handles);

	auto client = MetadbIndex::client();
	auto ptr = PlaybackStatistics::api()->begin_transaction();
	size_t matches{};

	for (auto&& record : records)
	{
		auto& jid = record["id"];
		if (!jid.is_string()) continue; // not a good sign but we'll persist

		const pfc::string8 id = jid.get<std::string>().c_str();
		const auto hash = client->hash_string(id);

		if (!hashes.contains(hash)) continue; // hash not found in set for a given handle list
		hashes.erase(hash); // this hash exists but remove it so any future duplicate is skipped

		matches++;

		auto f = PlaybackStatistics::get_fields(hash);
		bool changed{};

		auto& timestamps = record["2003_timestamps"];
		if (timestamps.is_array())
		{
			std::set<uint32_t> s;
			std::vector<uint32_t> v;

			for (auto&& timestamp : timestamps)
			{
				if (timestamp.is_number_unsigned())
				{
					const auto ts64 = timestamp.get<uint64_t>();
					if (ts64 == 0 || ts64 > UINT_MAX) continue;

					const auto ts = static_cast<uint32_t>(ts64);
					if (s.emplace(ts).second)
					{
						v.emplace_back(ts);
					}
				}
			}

			if (v.size())
			{
				std::ranges::sort(v);

				if (!Component::simple_mode)
				{
					f.timestamps = JSON(v).dump().c_str();
				}

				f.first_played = v[0];
				f.last_played = v[v.size() - 1];
				f.playcount = static_cast<uint32_t>(v.size());

				changed = true;
			}
		}
		else if (Component::simple_mode)
		{
			const auto first_played = get_uint32(record["2003_first_played"]);
			const auto last_played = get_uint32(record["2003_last_played"]);
			const auto playcount = get_uint32(record["2003_playcount"]);

			if (PlaybackStatistics::update_value(first_played, f.first_played)) changed = true;
			if (PlaybackStatistics::update_value(last_played, f.last_played)) changed = true;
			if (PlaybackStatistics::update_value(playcount, f.playcount)) changed = true;
		}

		const auto loved = get_uint32(record["2003_loved"], 1);
		const auto rating = get_uint32(record["2003_rating"], 10);
		const auto added = get_uint32(record["2003_added"]);

		if (PlaybackStatistics::update_value(loved, f.loved)) changed = true;
		if (PlaybackStatistics::update_value(rating, f.rating)) changed = true;
		if (PlaybackStatistics::update_value(added, f.added))
		{
			changed = true;
			if (f.added == 0) f.added = now;
		}

		if (changed)
		{
			PlaybackStatistics::set_fields(ptr, hash, f);
			to_refresh += hash;
		}
	}
	
	if (matches == 0)
	{
		return "The selected JSON file did not contain any matches.";
	}

	if (to_refresh.get_count() == 0)
	{
		return pfc::format("The selected JSON file contained ", matches, " matches but no values were modified.");
	}
	
	ptr->commit();
	PlaybackStatistics::refresh(to_refresh);
	return pfc::format(to_refresh.get_count(), " records were imported.");
}

std::filesystem::path ImportExport::get_fs_path(const pfc::string8& path)
{
	auto wpath = pfc::wideFromUTF8(path);
	return std::filesystem::path(wpath.c_str());
}

std::string ImportExport::read_file(const std::filesystem::path& path)
{
	std::string content;
	auto stream = std::ifstream(path);
	if (stream.is_open())
	{
		std::string line;
		while (std::getline(stream, line))
		{
			content += line;
		}
	}

	if (content.starts_with(UTF_8_BOM))
	{
		return content.substr(3);
	}

	return content;
}

uint32_t ImportExport::get_uint32(JSON& json, uint32_t upper_limit)
{
	if (json.is_number_unsigned())
	{
		const auto value64 = json.get<uint64_t>();
		if (value64 <= upper_limit)
		{
			return static_cast<uint32_t>(value64);
		}
	}
	return UINT_MAX;
}

void ImportExport::from_file(metadb_handle_list_cref handles)
{
	pfc::string8 path;
	if (!uGetOpenFileName(core_api::get_main_window(), "JSON file|*.json|All files|*.*", 0, "txt", "Import from", nullptr, path, FALSE)) return;

	const auto msg = from_file(handles, path);
	popup(msg);
}

void ImportExport::popup(const pfc::string8& msg)
{
	popup_message::g_show(msg, Component::name);
}

void ImportExport::to_file(metadb_handle_list_cref handles)
{
	pfc::string8 path, tf;
	if (!uGetOpenFileName(core_api::get_main_window(), "JSON file|*.json|All files|*.*", 0, "txt", "Save as", nullptr, path, TRUE)) return;

	titleformat_object_ptr obj;
	titleformat_compiler::get()->compile_safe(obj, Component::pin_to.get());

	PlaybackStatistics::HashSet hash_set;
	auto client = MetadbIndex::client();
	auto data = JSON::array();

	for (auto&& handle : handles)
	{
		metadb_index_hash hash{};
		if (client->hashHandle(handle, hash) && hash_set.emplace(hash).second)
		{
			const auto f = PlaybackStatistics::get_fields(hash);
			if (!f) continue;

			JSON json;

			handle->format_title(nullptr, tf, obj, nullptr);
			json["id"] = tf.get_ptr();

			if (f.added > 0) json["2003_added"] = f.added;
			if (f.loved > 0) json["2003_loved"] = f.loved;
			if (f.rating > 0) json["2003_rating"] = f.rating;

			if (Component::simple_mode)
			{
				if (f.first_played > 0) json["2003_first_played"] = f.first_played;
				if (f.last_played > 0) json["2003_last_played"] = f.last_played;
				if (f.playcount > 0) json["2003_playcount"] = f.playcount;
			}
			else
			{
				json["2003_timestamps"] = PlaybackStatistics::get_timestamps_array(f);
			}

			data.emplace_back(json);
		}
	}

	if (data.size() > 0)
	{
		auto fs_path = get_fs_path(path);
		auto stream = std::ofstream(fs_path, std::ios::binary);
		if (stream.is_open())
		{
			stream << data.dump();
			popup(pfc::format(data.size(), " record(s) exported succesfully."));
		}
	}
	else
	{
		popup("Nothing found to export.");
	}
}
