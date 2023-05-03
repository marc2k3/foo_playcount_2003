#pragma once

namespace Component
{
	static const pfc::string8 name = "Playcount 2003";
	static const pfc::string8 path_subsong = "%path%|%subsong%";

	struct MenuItem
	{
		const GUID* guid;
		const pfc::string8 name;
	};

	extern cfg_bool simple_mode;
	extern cfg_string pin_to;
}
