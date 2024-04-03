#pragma once

class JSONHelper
{
public:
	static JSON create_export_entry(std::string_view id, const PlaybackStatistics::Fields& f);
	static JSON create_timestamps_array(const std::vector<uint32_t>& v);
	static JSON get_timestamps_array(const PlaybackStatistics::Fields& f);
	static JSON parse(const std::string& str);
};
