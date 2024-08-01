#include "stdafx.hpp"

namespace
{
	bool g_tracked{};

	class PlayCallbackStatic : public play_callback_static
	{
	public:
		uint32_t get_flags() final
		{
			return flag_on_playback_time | flag_on_playback_new_track;
		}

		void on_playback_new_track(metadb_handle_ptr handle) final
		{
			g_tracked = false;
			elapsed_time = 0;
			target_time = SIZE_MAX;

			const auto length = handle->get_length();
			if (length <= 0.0) return;

			if (Component::advconfig_tracking_default.get())
			{
				const auto tmp = std::min(std::floor(length), 60.0);
				target_time = static_cast<size_t>(tmp);
			}
			else
			{
				titleformat_object_ptr ptr;
				titleformat_compiler::get()->compile_safe(ptr, Component::advconfig_tracking_pattern.get());

				pfc::string8 str;
				handle->format_title(nullptr, str, ptr, nullptr);

				if (pfc::string_is_numeric(str))
				{
					target_time = std::stoul(str.get_ptr());
				}
			}
		}

		void on_playback_time(double) final
		{
			if (g_tracked || target_time == SIZE_MAX) return;

			elapsed_time++;

			if (target_time == elapsed_time)
			{
				metadb_handle_ptr handle;
				playback_control::get()->get_now_playing(handle);
				PlaybackStatistics::on_item_played(handle);
				g_tracked = true;
			}
		}

		void on_playback_dynamic_info(const file_info&) final {}
		void on_playback_dynamic_info_track(const file_info&) final {}
		void on_playback_edited(metadb_handle_ptr) final {}
		void on_playback_pause(bool) final {}
		void on_playback_seek(double) final {}
		void on_playback_starting(playback_control::t_track_command, bool) {}
		void on_playback_stop(playback_control::t_stop_reason) {}
		void on_volume_change(float) final {}

	private:
		size_t elapsed_time{}, target_time{};
	};

	FB2K_SERVICE_FACTORY(PlayCallbackStatic);
}
