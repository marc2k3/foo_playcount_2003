#include "stdafx.hpp"

namespace
{
	static const std::vector<Component::MenuItem> context_items =
	{
		{ &guids::context_rating_zero, "Clear" },
		{ &guids::context_rating_one, "Set Rating to 1" },
		{ &guids::context_rating_two, "Set Rating to 2" },
		{ &guids::context_rating_three, "Set Rating to 3" },
		{ &guids::context_rating_four, "Set Rating to 4" },
		{ &guids::context_rating_five, "Set Rating to 5" },
		{ &guids::context_rating_six, "Set Rating to 6" },
		{ &guids::context_rating_seven, "Set Rating to 7" },
		{ &guids::context_rating_eight, "Set Rating to 8" },
		{ &guids::context_rating_nine, "Set Rating to 9" },
		{ &guids::context_rating_ten, "Set Rating to 10" },
	};

	class ContextMenuRating : public contextmenu_item_simple
	{
	public:
		GUID get_item_guid(uint32_t index) final
		{
			if (index >= context_items.size()) FB2K_BugCheck();

			return *context_items[index].guid;
		}

		GUID get_parent() final
		{
			return guids::context_group_rating;
		}

		bool context_get_display(uint32_t index, metadb_handle_list_cref, pfc::string_base& out, uint32_t&, const GUID&) final
		{
			if (index >= context_items.size()) FB2K_BugCheck();

			get_item_name(index, out);
			return true;
		}

		bool get_item_description(uint32_t index, pfc::string_base& out) final
		{
			if (index >= context_items.size()) FB2K_BugCheck();

			get_item_name(index, out);
			return true;
		}

		uint32_t get_num_items() final
		{
			return static_cast<uint32_t>(context_items.size());
		}

		void context_command(uint32_t index, metadb_handle_list_cref handles, const GUID&) final
		{
			PlaybackStatistics::HashList to_refresh;
			const auto hashes = PlaybackStatistics::get_hashes(handles);
			auto ptr = PlaybackStatistics::api()->begin_transaction();

			for (auto&& hash : hashes)
			{
				auto f = PlaybackStatistics::get_fields(hash);
				f.rating = index;
				PlaybackStatistics::set_fields(ptr, hash, f);
				to_refresh.add_item(hash);
			}

			ptr->commit();
			PlaybackStatistics::refresh(to_refresh);
		}

		void get_item_name(uint32_t index, pfc::string_base& out) final
		{
			if (index >= context_items.size()) FB2K_BugCheck();

			out = context_items[index].name;
		}
	};

	static contextmenu_group_popup_factory g_context_group_rating(guids::context_group_rating, guids::context_group, "Rating", 0);
	FB2K_SERVICE_FACTORY(ContextMenuRating);
}
