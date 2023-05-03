#include "stdafx.hpp"

namespace
{
	class PlaybackStatisticsCollector : public playback_statistics_collector
	{
	public:
		void on_item_played(metadb_handle_ptr handle) final
		{
			PlaybackStatistics::on_item_played(handle);
		}
	};

	FB2K_SERVICE_FACTORY(PlaybackStatisticsCollector);
}
