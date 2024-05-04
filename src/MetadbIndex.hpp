#pragma once

class MetadbIndex : public metadb_index_client
{
public:
	MetadbIndex();

	static MetadbIndex* client();
	static void init();

	metadb_index_hash hash_path(std::string_view path);
	metadb_index_hash hash_string(std::string_view str);
	metadb_index_hash transform(const file_info& info, const playable_location& location) final;
	pfc::string8 get_id(const metadb_handle_ptr& handle);

private:
	hasher_md5::ptr m_hasher;
	titleformat_object_ptr m_obj;
};
