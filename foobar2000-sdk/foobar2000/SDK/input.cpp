#include "foobar2000-sdk-pch.h"
#include <exception>
#include "input.h"
#include "input_impl.h"
#include "album_art.h"
#include "file_info_impl.h"

service_ptr input_entry::open(const GUID & whatFor, file::ptr hint, const char * path, event_logger::ptr logger, abort_callback & aborter) {

#ifdef FOOBAR2000_DESKTOP
	if ( whatFor == input_stream_info_reader::class_guid ) {
		input_entry_v2::ptr v2;
		if ( v2 &= this ) {
			GUID g = v2->get_guid();
			for (auto p : input_stream_info_reader_entry::enumerate()) {
				if (p->get_guid() == g) {
					return p->open(path, hint, aborter);
				}
			}
		}
		throw exception_io_unsupported_format();
	}
#endif
	if ( whatFor == album_art_extractor_instance::class_guid ) {
		input_entry_v2::ptr v2;
		if (v2 &= this) {
			GUID g = v2->get_guid();
			for (auto p : album_art_extractor::enumerate()) {
				if (p->get_guid() == g) {
					return p->open(hint, path, aborter);
				}
			}
		}
		throw exception_io_unsupported_format();
	}
	if ( whatFor == album_art_editor_instance::class_guid ) {
		input_entry_v2::ptr v2;
		if (v2 &= this) {
			GUID g = v2->get_guid();
			for (auto p : album_art_editor::enumerate()) {
				if (p->get_guid() == g) {
					return p->open(hint, path, aborter);
				}
			}
		}
		throw exception_io_unsupported_format();
	}

	input_entry_v3::ptr v3;

	if (v3 &= this) {
		return v3->open_v3( whatFor, hint, path, logger, aborter );
	} else {
		if (whatFor == input_decoder::class_guid) {
			input_decoder::ptr obj;
			open(obj, hint, path, aborter);
			if ( logger.is_valid() ) {
				input_decoder_v2::ptr v2;
				if (v2 &= obj) v2->set_logger(logger);
			}
			return obj;
		}
		if (whatFor == input_info_reader::class_guid) {
			input_info_reader::ptr obj;
			open(obj, hint, path, aborter);
			return obj;
		}
		if (whatFor == input_info_writer::class_guid) {
			input_info_writer::ptr obj;
			open(obj, hint, path, aborter);
			return obj;
		}
	}

	throw pfc::exception_not_implemented();
}

bool input_entry::g_find_service_by_path(service_ptr_t<input_entry> & p_out,const char * p_path)
{
    auto ext = pfc::string_extension(p_path);
    return g_find_service_by_path(p_out, p_path, ext );
}

bool input_entry::g_find_service_by_path(service_ptr_t<input_entry> & p_out,const char * p_path, const char * p_ext)
{
	for (auto ptr : enumerate()) {
		if (ptr->is_our_path(p_path,p_ext)) {
			p_out = ptr;
			return true;
		}
	}
	return false;
}

bool input_entry::g_find_service_by_content_type(service_ptr_t<input_entry> & p_out,const char * p_content_type)
{
	for (auto ptr : enumerate()) {
		if (ptr->is_our_content_type(p_content_type)) {
			p_out = ptr;
			return true;
		}
	}
	return false;
}


#if 0
static void prepare_for_open(service_ptr_t<input_entry> & p_service,service_ptr_t<file> & p_file,const char * p_path,filesystem::t_open_mode p_open_mode,abort_callback & p_abort,bool p_from_redirect)
{
	if (p_file.is_empty())
	{
		service_ptr_t<filesystem> fs;
		if (filesystem::g_get_interface(fs,p_path)) {
			if (fs->supports_content_types()) {
				fs->open(p_file,p_path,p_open_mode,p_abort);
			}
		}
	}

	if (p_file.is_valid())
	{
		pfc::string8 content_type;
		if (p_file->get_content_type(content_type))
		{
			if (input_entry::g_find_service_by_content_type(p_service,content_type))
				return;
		}
	}

	if (input_entry::g_find_service_by_path(p_service,p_path))
	{
		if (p_from_redirect && p_service->is_redirect()) throw exception_io_unsupported_format();
		return;
	}

	throw exception_io_unsupported_format();
}
#endif

bool input_entry::g_find_inputs_by_content_type(pfc::list_base_t<service_ptr_t<input_entry> > & p_out, const char * p_content_type, bool p_from_redirect) {
	auto filter = [=] (input_entry::ptr p) {
		return !(p_from_redirect && p->is_redirect());
	};
	return g_find_inputs_by_content_type_ex(p_out, p_content_type, filter );
}

bool input_entry::g_find_inputs_by_path(pfc::list_base_t<service_ptr_t<input_entry> > & p_out, const char * p_path, bool p_from_redirect) {
	auto filter = [=] (input_entry::ptr p) {
		return !(p_from_redirect && p->is_redirect());
	};
	return g_find_inputs_by_path_ex(p_out, p_path, filter);
}

bool input_entry::g_find_inputs_by_content_type_ex(pfc::list_base_t<service_ptr_t<input_entry> > & p_out, const char * p_content_type, input_filter_t filter ) {
	bool ret = false;
	for (auto ptr : enumerate()) {
		if (filter(ptr)) {
			if (ptr->is_our_content_type(p_content_type)) { p_out.add_item(ptr); ret = true; }
		}
	}
	return ret;
}

bool input_entry::g_find_inputs_by_path_ex(pfc::list_base_t<service_ptr_t<input_entry> > & p_out, const char * p_path, input_filter_t filter ) {
	auto extension = pfc::string_extension(p_path);
	bool ret = false;
	for( auto ptr : enumerate()) {
		GUID guid = pfc::guid_null;
		input_entry_v3::ptr ex;
		if ( ex &= ptr ) guid = ex->get_guid();
		if ( filter(ptr) ) {
			if (ptr->is_our_path(p_path, extension)) { p_out.add_item(ptr); ret = true; }
		}
	}
	return ret;
}

static GUID input_get_guid( input_entry::ptr e ) {
#ifdef FOOBAR2000_DESKTOP
	input_entry_v2::ptr p;
	if ( p &= e ) return p->get_guid();
#endif
	return pfc::guid_null;
}

service_ptr input_entry::g_open_from_list(input_entry_list_t const & p_list, const GUID & whatFor, service_ptr_t<file> p_filehint, const char * p_path, event_logger::ptr logger, abort_callback & p_abort, GUID * outGUID) {
	const t_size count = p_list.get_count();
	if ( count == 0 ) {
		// sanity
		throw exception_io_unsupported_format();
	} else if (count == 1) {
		auto ret = p_list[0]->open(whatFor, p_filehint, p_path, logger, p_abort);
		if ( outGUID != nullptr ) * outGUID = input_get_guid( p_list[0] );
		return ret;
	} else {
		std::exception_ptr errData, errUnsupported;
		for (t_size n = 0; n < count; n++) {
			try {
				auto ret = p_list[n]->open(whatFor, p_filehint, p_path, logger, p_abort);
				if (outGUID != nullptr) * outGUID = input_get_guid(p_list[n]);
				return ret;
			} catch (exception_io_no_handler_for_path) {
				//do nothing, skip over
			} catch(exception_io_unsupported_format) {
				if (!errUnsupported) errUnsupported = std::current_exception();
			} catch (exception_io_data) {
				if (!errData) errData = std::current_exception();
			}
		}
		if (errData) std::rethrow_exception(errData);
		if (errUnsupported) std::rethrow_exception(errUnsupported);
		throw exception_io_unsupported_format();
	}
}

#ifdef FOOBAR2000_DESKTOP
service_ptr input_manager::open_v2(const GUID & whatFor, file::ptr hint, const char * path, bool fromRedirect, event_logger::ptr logger, abort_callback & aborter, GUID * outUsedEntry) {
	// We're wrapping open_v2() on top of old open().
	// Assert on GUIDs that old open() is known to recognize.
	PFC_ASSERT(whatFor == input_decoder::class_guid || whatFor == input_info_reader::class_guid || whatFor == input_info_writer::class_guid || whatFor == input_stream_selector::class_guid);

    {
        input_manager_v2::ptr v2;
        if ( v2 &= this ) {
            return v2->open_v2( whatFor, hint, path, fromRedirect, logger, aborter, outUsedEntry );
        }
    }

	auto ret = open( whatFor, hint, path, fromRedirect, aborter, outUsedEntry );

#ifdef FB2K_HAVE_EVENT_LOGGER
	if ( logger.is_valid() ) {
		input_decoder_v2::ptr dec;
		if (dec &= ret) {
			dec->set_logger(logger);
		}
	}
#endif
	return ret;
}
#endif

service_ptr input_entry::g_open(const GUID & whatFor, file::ptr p_filehint, const char * p_path, event_logger::ptr logger, abort_callback & p_abort, bool p_from_redirect) {

#ifdef FOOBAR2000_DESKTOP
	return input_manager_v2::get()->open_v2(whatFor, p_filehint, p_path, p_from_redirect, logger, p_abort);
#else // FOOBAR2000_DESKTOP or not
	const bool needWriteAcecss = !!(whatFor == input_info_writer::class_guid);

	service_ptr_t<file> l_file = p_filehint;
	if (l_file.is_empty()) {
		service_ptr_t<filesystem> fs;
		if (filesystem::g_get_interface(fs, p_path)) {
			if (fs->supports_content_types()) {
				fs->open(l_file, p_path, needWriteAcecss ? filesystem::open_mode_write_existing : filesystem::open_mode_read, p_abort);
			}
		}
	}

	if (l_file.is_valid()) {
		pfc::string8 content_type;
		if (l_file->get_content_type(content_type)) {
			pfc::list_t< input_entry::ptr > list;
#if PFC_DEBUG
			FB2K_DebugLog() << "attempting input open by content type: " << content_type;
#endif
			if (g_find_inputs_by_content_type(list, content_type, p_from_redirect)) {
				try {
					return g_open_from_list(list, whatFor, l_file, p_path, logger, p_abort);
				} catch (exception_io_unsupported_format) {
#if PFC_DEBUG
					FB2K_DebugLog() << "Failed to open by content type, using fallback";
#endif
				}
			}
		}
	}

#if PFC_DEBUG
	FB2K_DebugLog() << "attempting input open by path: " << p_path;
#endif
	{
		pfc::list_t< input_entry::ptr > list;
		if (g_find_inputs_by_path(list, p_path, p_from_redirect)) {
			return g_open_from_list(list, whatFor, l_file, p_path, logger, p_abort);
		}
	}

	throw exception_io_unsupported_format();
#endif // not FOOBAR2000_DESKTOP
}

void input_entry::g_open_for_decoding(service_ptr_t<input_decoder> & p_instance,service_ptr_t<file> p_filehint,const char * p_path,abort_callback & p_abort,bool p_from_redirect) {
	TRACK_CALL_TEXT("input_entry::g_open_for_decoding");
	p_instance ^= g_open(input_decoder::class_guid, p_filehint, p_path, nullptr, p_abort, p_from_redirect);
}

void input_entry::g_open_for_info_read(service_ptr_t<input_info_reader> & p_instance,service_ptr_t<file> p_filehint,const char * p_path,abort_callback & p_abort,bool p_from_redirect) {
	TRACK_CALL_TEXT("input_entry::g_open_for_info_read");
	p_instance ^= g_open(input_info_reader::class_guid, p_filehint, p_path, nullptr, p_abort, p_from_redirect);
}

void input_entry::g_open_for_info_write(service_ptr_t<input_info_writer> & p_instance,service_ptr_t<file> p_filehint,const char * p_path,abort_callback & p_abort,bool p_from_redirect) {
	TRACK_CALL_TEXT("input_entry::g_open_for_info_write");
	p_instance ^= g_open(input_info_writer::class_guid, p_filehint, p_path, nullptr, p_abort, p_from_redirect);
}

void input_entry::g_open_for_info_write_timeout(service_ptr_t<input_info_writer> & p_instance,service_ptr_t<file> p_filehint,const char * p_path,abort_callback & p_abort,double p_timeout,bool p_from_redirect) {
	pfc::lores_timer timer;
	timer.start();
	for(;;) {
		try {
			g_open_for_info_write(p_instance,p_filehint,p_path,p_abort,p_from_redirect);
			break;
		} catch(exception_io_sharing_violation) {
			if (timer.query() > p_timeout) throw;
			p_abort.sleep(0.01);
		}
	}
}

bool input_entry::g_is_supported_path(const char * p_path)
{
	auto ext = pfc::string_extension (p_path);
	for( auto ptr : enumerate() ) {
		if (ptr->is_our_path(p_path,ext)) return true;
	}
	return false;
}



void input_open_file_helper(service_ptr_t<file> & p_file,const char * p_path,t_input_open_reason p_reason,abort_callback & p_abort)
{
	if (p_file.is_empty()) {
		switch(p_reason) {
		default:
			uBugCheck();
		case input_open_info_read:
		case input_open_decode:
			filesystem::g_open(p_file,p_path,filesystem::open_mode_read,p_abort);
			break;
		case input_open_info_write:
			filesystem::g_open(p_file,p_path,filesystem::open_mode_write_existing,p_abort);
			break;
		}
	} else {
		p_file->reopen(p_abort);
	}
}

uint32_t input_entry::g_flags_for_path( const char * path, uint32_t mask ) {
#if defined(FOOBAR2000_DESKTOP) && FOOBAR2000_TARGET_VERSION >= 80
	return input_manager_v3::get()->flags_for_path(path, mask);
#else
#ifdef FOOBAR2000_DESKTOP
	input_manager_v3::ptr api;
	if ( input_manager_v3::tryGet(api) ) {
		return api->flags_for_path(path, mask);
	}
#endif
	uint32_t ret = 0;
	service_enum_t<input_entry> e; input_entry::ptr p;
	auto ext = pfc::string_extension(path);
	while(e.next(p)) {
		uint32_t f = p->get_flags() & mask;
		if ( f != 0 && p->is_our_path( path, ext ) ) ret |= f;;
	}
	return ret;
#endif
}
uint32_t input_entry::g_flags_for_content_type( const char * ct, uint32_t mask ) {
#if defined(FOOBAR2000_DESKTOP) && FOOBAR2000_TARGET_VERSION >= 80
	return input_manager_v3::get()->flags_for_content_type(ct, mask);
#else
#ifdef FOOBAR2000_DESKTOP
	input_manager_v3::ptr api;
	if ( input_manager_v3::tryGet(api) ) {
		return api->flags_for_content_type( ct, mask );
	}
#endif
    uint32_t ret = 0;
	service_enum_t<input_entry> e; input_entry::ptr p;
	while(e.next(p)) {
		uint32_t f = p->get_flags() & mask;
		if ( f != 0 && p->is_our_content_type(ct) ) ret |= f;
	}
	return ret;
#endif
}

bool input_entry::g_are_parallel_reads_slow(const char * path) {
	return g_flags_for_path(path, flag_parallel_reads_slow) != 0;
}

void input_entry_v3::open_for_decoding(service_ptr_t<input_decoder> & p_instance, service_ptr_t<file> p_filehint, const char * p_path, abort_callback & p_abort) {
	p_instance ^= open_v3( input_decoder::class_guid, p_filehint, p_path, nullptr, p_abort );
}
void input_entry_v3::open_for_info_read(service_ptr_t<input_info_reader> & p_instance, service_ptr_t<file> p_filehint, const char * p_path, abort_callback & p_abort) {
	p_instance ^= open_v3(input_info_reader::class_guid, p_filehint, p_path, nullptr, p_abort);
}
void input_entry_v3::open_for_info_write(service_ptr_t<input_info_writer> & p_instance, service_ptr_t<file> p_filehint, const char * p_path, abort_callback & p_abort) {
	p_instance ^= open_v3(input_info_writer::class_guid, p_filehint, p_path, nullptr, p_abort);
}

void input_info_writer::remove_tags_fallback(abort_callback & abort) {
	uint32_t total = this->get_subsong_count();
	file_info_impl blank;
	for( uint32_t walk = 0; walk < total; ++ walk ) {
		this->set_info( this->get_subsong(walk), blank, abort );
	}
	this->commit( abort );
}

t_filestats input_info_reader_v2::get_file_stats(abort_callback& a) {
	return this->get_stats2(stats2_size | stats2_timestamp, a).to_legacy();
}

t_filestats2 input_info_reader::get_stats2_(const char* fallbackPath, uint32_t f, abort_callback& a) {
	t_filestats2 ret;
	input_info_reader_v2::ptr v2;
	if (v2 &= this) {
		ret = v2->get_stats2(f, a);
	} else if ((f & ~stats2_legacy) == 0) {
		t_filestats subset = this->get_file_stats(a);
		ret.m_size = subset.m_size;
		ret.m_timestamp = subset.m_timestamp;
	} else {
		try {
			auto fs = filesystem::tryGet(fallbackPath);
			if (fs.is_valid()) ret = fs->get_stats2_(fallbackPath, f, a);
		} catch (exception_io) {}
	}
	return ret;
}

GUID input_entry::get_guid_() {
	auto ret = pfc::guid_null;
	input_entry_v2::ptr v2;
	if ( v2 &= this ) ret = v2->get_guid();
	return ret;
}

const char* input_entry::get_name_() {
	const char * ret = "<legacy object>";
	input_entry_v2::ptr v2;
	if ( v2 &= this ) ret = v2->get_name();
	return ret;
}

input_entry::ptr input_entry::g_find_by_guid(const GUID& guid) {
	for (auto ptr : enumerate()) {
		input_entry_v2::ptr v2;
		if (v2 &= ptr) {
			if ( guid == v2->get_guid() ) return v2;
		}
	}
	return nullptr;
}