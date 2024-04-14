#include "stdafx.hpp"

MetadbIndex::MetadbIndex() : m_hasher(hasher_md5::get())
{
	titleformat_compiler::get()->compile_safe(m_obj, Component::pin_to.get());
}

#pragma region static
MetadbIndex* MetadbIndex::client()
{
	static MetadbIndex* cached = new service_impl_single_t<MetadbIndex>();
	return cached;
}

void MetadbIndex::init()
{
	auto api = PlaybackStatistics::api();

	try
	{
		api->add(client(), guids::metadb_index, system_time_periods::week * 4);
		api->dispatch_global_refresh();
		FB2K_console_formatter() << fmt::format("{}: initialised in {} mode.", Component::name, Component::simple_mode ? "simple" : "advanced").c_str();
	}
	catch (const std::exception& e)
	{
		api->remove(guids::metadb_index);
		FB2K_console_formatter() << fmt::format("{}: critical initialisation failure: {}", Component::name, e.what()).c_str();
	}
}
#pragma endregion

metadb_index_hash MetadbIndex::hash_path(std::string_view path)
{
	const auto str = fmt::format("{}|0", path);
	return hash_string(str);
}

metadb_index_hash MetadbIndex::hash_string(std::string_view str)
{
	return m_hasher->process_single_string(str.data()).xorHalve();
}

metadb_index_hash MetadbIndex::transform(const file_info& info, const playable_location& location)
{
	pfc::string8 str;
	m_obj->run_simple(location, &info, str);
	return hash_string(str);
}
