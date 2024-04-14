#include "stdafx.hpp"

namespace
{
	static constexpr std::array context_guids =
	{
		&guids::context_rating_zero,
		&guids::context_rating_one,
		&guids::context_rating_two,
		&guids::context_rating_three,
		&guids::context_rating_four,
		&guids::context_rating_five,
		&guids::context_rating_six,
		&guids::context_rating_seven,
		&guids::context_rating_eight,
		&guids::context_rating_nine,
		&guids::context_rating_ten,
	};

	class ContextMenuRating : public contextmenu_item_simple
	{
	public:
		GUID get_item_guid(uint32_t index) final
		{
			if (index >= context_guids.size()) FB2K_BugCheck();

			return *context_guids[index];
		}

		GUID get_parent() final
		{
			return guids::context_group_rating;
		}

		bool context_get_display(uint32_t index, metadb_handle_list_cref, pfc::string_base& out, uint32_t&, const GUID&) final
		{
			if (index >= context_guids.size()) FB2K_BugCheck();

			get_item_name(index, out);
			return true;
		}

		bool get_item_description(uint32_t index, pfc::string_base& out) final
		{
			if (index >= context_guids.size()) FB2K_BugCheck();

			get_item_name(index, out);
			return true;
		}

		uint32_t get_num_items() final
		{
			return static_cast<uint32_t>(context_guids.size());
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
			if (index >= context_guids.size()) FB2K_BugCheck();

			if (index == 0)
			{
				out = "Clear";
			}
			else
			{
				out = fmt::format("Set Rating to {}", index);
			}
		}
	};

	static contextmenu_group_popup_factory g_context_group_rating(guids::context_group_rating, guids::context_group, "Rating", 0);
	FB2K_SERVICE_FACTORY(ContextMenuRating);
}
