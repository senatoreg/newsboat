#ifndef NEWSBOAT_PODBOATQUEUEMANAGER_H_
#define NEWSBOAT_PODBOATQUEUEMANAGER_H_

#include <memory>
#include <string>
#include "queuemanager.h"

namespace newsboat {

class ConfigContainer;
class RssFeed;
class RssItem;

class PodboatQueueManager : public QueueManager {
	ConfigContainer* cfg = nullptr;
	std::string queue_file;
	std::string _name;

public:
	/// Construct `PodboatQueueManager` instance out of a config container and a path
	/// to the queue file.
	PodboatQueueManager(ConfigContainer* cfg, std::string queue_file);

	void init();
	void deinit();

	/// Adds the podcast URL to Podboat's queue file
	EnqueueResult enqueue_url(std::shared_ptr<RssItem> item,
				  std::shared_ptr<RssFeed> feed);

private:
	std::string generate_enqueue_filename(std::shared_ptr<RssItem> item,
		std::shared_ptr<RssFeed> feed);
};

}

#endif /* NEWSBOAT_QUEUEMANAGER_H_ */

