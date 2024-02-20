#include "stdafx.hpp"

namespace
{
	class FileOperationCallback : public file_operation_callback
	{
	public:
		void on_files_copied_sorted(t_pathlist from, t_pathlist to) final
		{
			on_files(from, to);
		}

		void on_files_deleted_sorted(t_pathlist) final {}

		void on_files_moved_sorted(t_pathlist from, t_pathlist to) final
		{
			on_files(from, to);
		}

	private:
		pfc::string8 display_path(const pfc::string8& path)
		{
			pfc::string8 ret;
			filesystem::g_get_display_path(path, ret);
			return ret;
		}

		void on_files(t_pathlist from, t_pathlist to)
		{
			if (Component::pin_to.get() != Component::path_subsong) return;

			static const PlaybackStatistics::Fields dummy;
			const size_t count = from.get_count();

			PlaybackStatistics::HashList to_refresh;
			auto client = MetadbIndex::client();
			auto ptr = PlaybackStatistics::api()->begin_transaction();

			for (const size_t i : std::views::iota(0U, count))
			{
				const auto old_path = display_path(from[i]);
				const auto new_path = display_path(to[i]);

				const auto old_hash = client->hash_path(old_path);
				const auto new_hash = client->hash_path(new_path);

				const auto f = PlaybackStatistics::get_fields(old_hash);
				PlaybackStatistics::set_fields(ptr, new_hash, f);
				to_refresh.add_item(new_hash);
			}

			ptr->commit();
			PlaybackStatistics::refresh(to_refresh);
		}
	};

	FB2K_SERVICE_FACTORY(FileOperationCallback);
}
