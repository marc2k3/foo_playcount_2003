#include "stdafx.hpp"
#include "DialogEdit.hpp"
#include "ImportExport.hpp"

namespace
{
	static const std::vector<Component::MenuItem> context_items =
	{
		{ &guids::context_edit, "Edit..." },
		{ &guids::context_love, "Love" },
		{ &guids::context_unlove, "Unlove" },
		{ &guids::context_clear_all, "Clear all" },
		{ &guids::context_file_import, "Import from file" },
		{ &guids::context_file_export, "Export to file" },
	};

	class ContextMenu : public contextmenu_item_simple
	{
	public:
		GUID get_item_guid(uint32_t index) final
		{
			if (index >= context_items.size()) FB2K_BugCheck();

			return *context_items[index].guid;
		}

		GUID get_parent() final
		{
			return guids::context_group;
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
			switch (index)
			{
			case 0:
				init_dialog(handles);
				break;
			case 1:
			case 2:
				PlaybackStatistics::set_loved(handles, index == 1);
				break;
			case 3:
				PlaybackStatistics::clear(handles);
				break;
			case 4:
				ImportExport::from_file(handles);
				break;
			case 5:
				ImportExport::to_file(handles);
				break;
			}
		}

		void get_item_name(uint32_t index, pfc::string_base& out) final
		{
			if (index >= context_items.size()) FB2K_BugCheck();

			out = context_items[index].name;
		}

	private:
		bool parse_tf(std::string_view str, PlaybackStatistics::Fields& f)
		{
			pfc::string_list_impl list;
			pfc::splitStringByChar(list, str.data(), '|');

			if (list.get_count() != 5)
			{
				return false;
			}

			bool changed{};

			const auto first_played = PlaybackStatistics::string_to_timestamp(list[0]);
			if (PlaybackStatistics::update_value(first_played, f.first_played)) changed = true;

			const auto last_played = PlaybackStatistics::string_to_timestamp(list[1]);
			if (PlaybackStatistics::update_value(last_played, f.last_played)) changed = true;

			const auto added = PlaybackStatistics::string_to_timestamp(list[2]);
			if (PlaybackStatistics::update_value(added, f.added)) changed = true;

			const std::string pc = list[3];
			if (pfc::string_is_numeric(pc.c_str()))
			{
				const auto playcount = std::stoul(pc);
				if (playcount <= UINT_MAX && playcount != f.playcount)
				{
					f.playcount = playcount;
					changed = true;

					if (f.playcount == 0)
					{
						f.first_played = 0;
						f.last_played = 0;
					}
				}
			}

			const std::string rt = list[4];
			if (pfc::string_is_numeric(rt.c_str()))
			{
				const auto rating = std::stoul(rt);
				if (rating <= 10 && rating != f.rating)
				{
					f.rating = rating;
					changed = true;
				}
			}

			return changed;
		}

		void import_from_dialog_tf(metadb_handle_list_cref handles, std::string_view tf)
		{
			titleformat_object_ptr obj;
			titleformat_compiler::get()->compile_safe(obj, tf.data());

			PlaybackStatistics::HashList to_refresh;
			PlaybackStatistics::HashSet hash_set;
			auto client = MetadbIndex::client();
			auto ptr = PlaybackStatistics::api()->begin_transaction();
			size_t unique_ids{};

			for (auto&& handle : handles)
			{
				metadb_index_hash hash{};
				if (client->hashHandle(handle, hash) && hash_set.emplace(hash).second)
				{
					unique_ids++;

					pfc::string8 str;
					handle->format_title(nullptr, str, obj, nullptr);
					auto f = PlaybackStatistics::get_fields(hash);

					if (parse_tf(str, f))
					{
						PlaybackStatistics::set_fields(ptr, hash, f);
						to_refresh.add_item(hash);
					}
				}
			}

			if (to_refresh.get_count() > 0)
			{
				ptr->commit();
				PlaybackStatistics::refresh(to_refresh);
			}

			FB2K_console_formatter() << fmt::format("{}: {} items were selected and {} unique ids were found. Of those, {} were updated.", Component::name, handles.get_count(), unique_ids, to_refresh.get_count()).c_str();
		}

		void init_dialog(metadb_handle_list_cref handles)
		{
			modal_dialog_scope scope;
			if (!scope.can_create())
				return;

			auto wnd = core_api::get_main_window();
			scope.initialize(wnd);

			CDialogEdit dlg;
			metadb_index_hash hash{};

			if (handles.get_count() == 1 && MetadbIndex::client()->hashHandle(handles[0], hash))
			{
				auto f = PlaybackStatistics::get_fields(hash);
				if (f.added > 0) dlg.m_added = PlaybackStatistics::timestamp_to_string(f.added);
				if (f.first_played > 0) dlg.m_first_played = PlaybackStatistics::timestamp_to_string(f.first_played);
				if (f.last_played > 0) dlg.m_last_played = PlaybackStatistics::timestamp_to_string(f.last_played);
				if (f.playcount > 0) dlg.m_playcount = pfc::format_uint(f.playcount);
				if (f.rating > 0) dlg.m_rating = pfc::format_uint(f.rating);
			}

			if (dlg.DoModal(wnd) == IDOK)
			{
				const auto tf = pfc::format(dlg.m_first_played, "|", dlg.m_last_played, "|", dlg.m_added, "|", dlg.m_playcount, "|", dlg.m_rating);
				if (tf == "||||")
					return;

				import_from_dialog_tf(handles, tf);
			}
		}
	};

	static contextmenu_group_popup_factory g_context_group(guids::context_group, contextmenu_groups::root, Component::name.data(), 0);
	FB2K_SERVICE_FACTORY(ContextMenu);
}
