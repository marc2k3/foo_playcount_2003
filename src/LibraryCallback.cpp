#include "stdafx.hpp"

namespace
{
	class LibraryCallback : public library_callback_v2
	{
	public:
		void on_items_added(metadb_handle_list_cref handles) final
		{
			if (library_manager_v5::get()->is_initialized())
			{
				auto index = search_index_manager::get()->create_index(handles, nullptr);
				update_added(index);
			}
		}

		void on_library_initialized() final
		{
			auto t = std::thread([this]
				{
					std::this_thread::sleep_for(std::chrono::milliseconds(5000));

					fb2k::inMainThread([this]
						{
							auto index = search_index_manager::get()->get_library_index();
							update_added(index);
						});
				});

			t.detach();
		}

		void on_items_modified(metadb_handle_list_cref) final {}
		void on_items_modified_v2(metadb_handle_list_cref, metadb_io_callback_v2_data&) final {}
		void on_items_removed(metadb_handle_list_cref) final {}

	private:
		void update_added(const search_index::ptr& index)
		{
			static const auto filter = search_filter_manager_v2::get()->create_ex("%2003_added% MISSING", fb2k::service_new<completion_notify_dummy>(), search_filter_manager_v2::KFlagSuppressNotify);
			auto arr = index->search(filter, nullptr, 0, fb2k::noAbort);
			if (arr->get_count() == 0)
				return;

			PlaybackStatistics::HashList to_refresh;
			const auto now = PlaybackStatistics::now();
			const auto& handles = arr->as_list_of<metadb_handle>();
			const auto hashes = PlaybackStatistics::get_hashes(handles);
			auto ptr = PlaybackStatistics::api()->begin_transaction();

			for (auto&& hash : hashes)
			{
				auto f = PlaybackStatistics::get_fields(hash);
				f.added = now;
				PlaybackStatistics::set_fields(ptr, hash, f);
				to_refresh.add_item(hash);
			}

			ptr->commit();
			PlaybackStatistics::refresh(to_refresh);
		}
	};

	FB2K_SERVICE_FACTORY(LibraryCallback);
}
