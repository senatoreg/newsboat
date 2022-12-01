#ifndef NEWSBOAT_PODBOATQUEUEMANAGER_H_
#define NEWSBOAT_PODBOATQUEUEMANAGER_H_

#include <memory>
#include <string>
#include "queuemanager.h"
#include "configparser.h"

namespace newsboat {

class ConfigContainer;
class RssFeed;
class RssItem;

class PodboatQueueManager : public QueueManager {
public:
	/// Construct `PodboatQueueManager` instance out of a config container and a path
	/// to the queue file.
	PodboatQueueManager();
	~PodboatQueueManager();

        void init() {};
        void deinit() {};

        /// Adds the podcast URL to Podboat's queue file
	EnqueueResult enqueue_url(std::shared_ptr<RssItem> item,
				  std::shared_ptr<RssFeed> feed);

private:
	ConfigContainer* cfg = nullptr;
	std::string queue_file;
	std::string _name;

	ConfigParser cfgparser;

	std::string generate_enqueue_filename(std::shared_ptr<RssItem> item,
		std::shared_ptr<RssFeed> feed);
};

}

#endif /* NEWSBOAT_PODBOATQUEUEMANAGER_H_ */

