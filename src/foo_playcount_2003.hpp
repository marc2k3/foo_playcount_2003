#pragma once

namespace Component
{
	static const string8 name = "Playcount 2003";
	static const string8 path_subsong = "%path%|%subsong%";

	struct MenuItem
	{
		const GUID* guid;
		const string8 name;
	};

	extern cfg_bool simple_mode;
	extern cfg_string pin_to;
}
