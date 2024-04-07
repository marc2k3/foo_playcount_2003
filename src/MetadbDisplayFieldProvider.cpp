#include "stdafx.hpp"

namespace
{
	static constexpr std::array field_names =
	{
		"2003_now",
		"2003_now_ts",
		"2003_first_played",
		"2003_first_played_ts",
		"2003_last_played",
		"2003_last_played_ts",
		"2003_added",
		"2003_added_ts",
		"2003_loved",
		"2003_playcount",
		"2003_rating",
		"2003_first_played_ago",
		"2003_last_played_ago",
		"2003_added_ago",
		"2003_first_played_ago2",
		"2003_last_played_ago2",
		"2003_added_ago2",
		"2003_timestamps",
		"2003_playcount_this_year",
		"2003_playcount_last_year",
	};

	class MetadbDisplayFieldProvider : public metadb_display_field_provider_v2
	{
	public:
		bool process_field(uint32_t index, metadb_handle* handle, titleformat_text_out* out) final
		{
			return process_field_v2(index, handle, handle->query_v2_(), out);
		}

		bool process_field_v2(uint32_t index, metadb_handle* handle, const metadb_v2::rec_t& rec, titleformat_text_out* out) final
		{
			if (write_now(out, index)) return true;
			if (rec.info.is_empty()) return false;

			const auto hash = MetadbIndex::client()->transform(rec.info->info(), handle->get_location());
			const auto f = PlaybackStatistics::get_fields(hash);

			switch (index)
			{
			case 2:
			case 3:
				return write_ts(out, f.first_played, index);
			case 4:
			case 5:
				return write_ts(out, f.last_played, index);
			case 6:
			case 7:
				return write_ts(out, f.added, index);
			case 8:
				return write_num(out, f.loved);
			case 9:
				return write_num(out, f.playcount);
			case 10:
				return write_num(out, f.rating);
			case 11:
				return ago(out, f.first_played);
			case 12:
				return ago(out, f.last_played);
			case 13:
				return ago(out, f.added);
			case 14:
				return ago2(out, f.first_played);
			case 15:
				return ago2(out, f.last_played);
			case 16:
				return ago2(out, f.added);
			case 17:
				if (Component::simple_mode) return false;
				out->write(titleformat_inputtypes::meta, f.timestamps);
				return true;
			case 18:
			case 19:
				{
					if (Component::simple_mode) return false;

					const auto playcount = PlaybackStatistics::playcount_year(f, index == 19);
					if (playcount == 0) return false;

					out->write_int(titleformat_inputtypes::meta, playcount);
					return true;
				}
			}

			return false;
		}

		uint32_t get_field_count() final
		{
			return static_cast<uint32_t>(field_names.size());
		}

		void get_field_name(uint32_t index, pfc::string_base& out) final
		{
			out = field_names[index];
		}

	private:
		bool ago(titleformat_text_out* out, uint32_t ts)
		{
			const auto now = PlaybackStatistics::now();
			if (ts == 0 || ts > now) return false;

			const double diff = static_cast<double>(now - ts);
			out->write(titleformat_inputtypes::unknown, pfc::format_time_ex(diff, 0));
			return true;
		}

		bool ago2(titleformat_text_out* out, uint32_t ts)
		{
			const auto now = PlaybackStatistics::now();
			if (ts == 0 || ts > now) return false;

			bool include_weeks_days = true;
			pfc::string8 str;
			uint32_t diff = now - ts;

			if (diff < day_in_seconds * 2)
			{
				const auto today_string = PlaybackStatistics::timestamp_to_string(now).subString(0, 10);
				const auto yesterday_string = PlaybackStatistics::timestamp_to_string(now - day_in_seconds).subString(0, 10);
				const auto ts_string = PlaybackStatistics::timestamp_to_string(ts).subString(0, 10);

				if (ts_string == today_string)
				{
					str = "Today";
				}
				else if (ts_string == yesterday_string)
				{
					str = "Yesterday";
				}
				else
				{
					str = "1d";
				}

				out->write(titleformat_inputtypes::unknown, str);
				return true;
			}

			const auto years = diff / year_in_seconds;
			if (years > 0)
			{
				include_weeks_days = false;
				diff -= years * year_in_seconds;
				str << years << "y ";
			}

			const auto months = diff / month_in_seconds;
			if (months > 0)
			{
				diff -= months * month_in_seconds;
				str << months << "m ";
			}

			if (include_weeks_days)
			{
				const auto weeks = diff / week_in_seconds;
				if (weeks > 0)
				{
					diff -= weeks * week_in_seconds;
					str << weeks << "w ";
				}

				const auto days = diff / day_in_seconds;
				if (days > 0)
				{
					str << days << "d";
				}
			}

			out->write(titleformat_inputtypes::unknown, str.trim(' '));
			return true;
		}

		bool write_now(titleformat_text_out* out, uint32_t index)
		{
			if (index > 1) return false;

			const auto now = PlaybackStatistics::now();
			const bool as_string = index % 2 == 0;

			if (as_string)
			{
				const auto str = PlaybackStatistics::timestamp_to_string(now);
				out->write(titleformat_inputtypes::unknown, str);
			}
			else
			{
				out->write_int(titleformat_inputtypes::unknown, now);
			}

			return true;
		}

		bool write_num(titleformat_text_out* out, uint32_t num)
		{
			if (num == 0) return false;

			out->write_int(titleformat_inputtypes::meta, num);
			return true;
		}

		bool write_ts(titleformat_text_out* out, uint32_t ts, uint32_t index)
		{
			if (ts == 0) return false;

			const bool as_string = index % 2 == 0;

			if (as_string)
			{
				const auto str = PlaybackStatistics::timestamp_to_string(ts);
				out->write(titleformat_inputtypes::meta, str);
			}
			else
			{
				out->write_int(titleformat_inputtypes::meta, ts);
			}

			return true;
		}

		static constexpr uint32_t day_in_seconds = 24 * 60 * 60;
		static constexpr uint32_t week_in_seconds = 7 * day_in_seconds;
		static constexpr uint32_t month_in_seconds = 30 * day_in_seconds;
		static constexpr uint32_t year_in_seconds = 365 * day_in_seconds;
	};

	FB2K_SERVICE_FACTORY(MetadbDisplayFieldProvider);
}
