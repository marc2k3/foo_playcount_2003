#pragma once
#define _ATL_MODULES
#define _WIN32_WINNT _WIN32_WINNT_WIN7
#define WINVER _WIN32_WINNT_WIN7
#define NOMINMAX

#include <array>
#include <filesystem>
#include <fstream>
#include <ranges>
#include <set>
#include <string>
#include <vector>

#define JSON_USE_IMPLICIT_CONVERSIONS 0
#include <nlohmann/json.hpp>
using JSON = nlohmann::json;

#include <helpers/foobar2000+atl.h>
#include <helpers/dropdown_helper.h>
#include <pfc/filetimetools.h>
#include <pfc/string-conv-lite.h>
#include <SDK/coreDarkMode.h>

#include "foo_playcount_2003.hpp"
#include "GUIDS.hpp"
#include "Resource.hpp"

#include "MetadbIndex.hpp"
#include "PlaybackStatistics.hpp"
#include "JSONHelper.hpp"
