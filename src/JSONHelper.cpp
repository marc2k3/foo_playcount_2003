#include "stdafx.hpp"
#include "JSONHelper.hpp"

JSON JSONHelper::create_export_entry(std::string_view id, const PlaybackStatistics::Fields& f)
{
	JSON json;
	json["id"] = id;

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
		json["2003_timestamps"] = get_timestamps_array(f);
	}

	return json;
}

JSON JSONHelper::create_timestamps_array(const std::vector<uint32_t>& v)
{
	// auto j = JSON(v) would usually be fine but this considers timestamps
	// too close together as duplicates

	static constexpr uint32_t good_diff = 10U;

	auto j = JSON::array();
	j.emplace_back(v.at(0));
	uint32_t last_good = 1U;

	for (const uint32_t i : v | std::views::drop(1))
	{
		if (i >= last_good + good_diff)
		{
			last_good = i;
			j.emplace_back(i);
		}
	}

	return j;
}

JSON JSONHelper::get_timestamps_array(const PlaybackStatistics::Fields& f)
{
	auto timestamps = parse(f.timestamps.get_ptr());
	if (!timestamps.is_array()) timestamps = JSON::array(); // WTFBBQLOL
	return timestamps;
}

JSON JSONHelper::parse(const std::string& str)
{
	return JSON::parse(str, nullptr, false);
}
