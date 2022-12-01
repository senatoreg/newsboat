#ifndef NEWSBOAT_QUEUEMANAGER_H_
#define NEWSBOAT_QUEUEMANAGER_H_

#include <memory>
#include <string>

namespace newsboat {

class ConfigContainer;
class ConfigPaths;
class RssFeed;
class RssItem;

enum class EnqueueStatus {
	QUEUED_SUCCESSFULLY,
	URL_QUEUED_ALREADY, // `extra_info` should specify the concerning URL
	OUTPUT_FILENAME_USED_ALREADY, // `extra_info` should specify the generated filename
	QUEUE_FILE_OPEN_ERROR, // `extra_info` should specify the location of the queue file
};

struct EnqueueResult {
	EnqueueStatus status;
	std::string extra_info;
};

class QueueManager {
	ConfigContainer* cfg = nullptr;
	std::string _name;
	void* _plugin_handle;
	QueueManager* _queueManager;

public:
	/// Construct `QueueManager` instance out of a config container and a path
	/// to the queue file.
	QueueManager() {};
	QueueManager(ConfigContainer *cfg);
	virtual ~QueueManager();
	virtual std::string &getName() { return _name; };

	virtual void init();
	virtual void deinit();

	/// Adds the podcast URL to Podboat's queue file
	virtual EnqueueResult enqueue_url(std::shared_ptr<RssItem> item,
					  std::shared_ptr<RssFeed> feed);

	/// Add all HTTP and HTTPS enclosures to the queue file
	EnqueueResult autoenqueue(std::shared_ptr<RssFeed> feed);
};

}

typedef newsboat::QueueManager* (*plugin_init_t)();
typedef void (*plugin_deinit_t)(newsboat::QueueManager*);

#endif /* NEWSBOAT_QUEUEMANAGER_H_ */

