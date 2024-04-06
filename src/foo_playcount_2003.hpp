#pragma once

namespace Component
{
	static constexpr std::string_view name = "Playcount 2003";
	static constexpr std::string_view path_subsong = "%path%|%subsong%";

	struct MenuItem
	{
		const GUID* guid;
		const std::string_view name;
	};

	extern cfg_bool simple_mode;
	extern cfg_string pin_to;
}
