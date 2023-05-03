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
		FB2K_console_formatter() << pfc::format(Component::name, ": initialised in ", Component::simple_mode ? "simple" : "advanced", " mode.");
	}
	catch (const std::exception& e)
	{
		api->remove(guids::metadb_index);
		FB2K_console_formatter() << pfc::format(Component::name, ": critical initialisation failure: ", e.what());
	}
}
#pragma endregion

metadb_index_hash MetadbIndex::path_subsong_hash(const pfc::string8& path)
{
	const auto str = pfc::format(path, "|0");
	return m_hasher->process_single_string(str).xorHalve();
}

metadb_index_hash MetadbIndex::transform(const file_info& info, const playable_location& location)
{
	pfc::string8 str;
	m_obj->run_simple(location, &info, str);
	return m_hasher->process_single_string(str).xorHalve();
}
