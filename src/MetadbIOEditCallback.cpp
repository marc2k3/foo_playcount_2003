#include "stdafx.hpp"

namespace
{
	class MetadbIOEditCallback : public metadb_io_edit_callback
	{
	public:
		void on_edited(metadb_handle_list_cref handles, t_infosref before, t_infosref after)
		{
			// ignore tag updates if bound to path/subsong
			if (Component::pin_to.get() == Component::path_subsong) return;

			PlaybackStatistics::HashList hash_list;
			PlaybackStatistics::HashSet hash_set;
			const size_t count = handles.get_count();
			auto client = MetadbIndex::client();
			auto ptr = PlaybackStatistics::api()->begin_transaction();

			for (const size_t i : std::views::iota(size_t{}, count))
			{
				const auto& location = handles[i]->get_location();
				const auto old_hash = client->transform(*before[i], location);
				const auto new_hash = client->transform(*after[i], location);

				if (old_hash == new_hash) continue;
				if (!hash_set.emplace(new_hash).second) continue;

				const auto f = PlaybackStatistics::get_fields(old_hash);
				PlaybackStatistics::set_fields(ptr, new_hash, f);
				hash_list.add_item(new_hash);
			}

			ptr->commit();
			PlaybackStatistics::refresh(hash_list);
		}
	};

	FB2K_SERVICE_FACTORY(MetadbIOEditCallback);
}
