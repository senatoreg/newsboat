#include "queuemanager.h"

#include <fstream>
#include <iostream>
#include <libxml/uri.h>
#include <dlfcn.h>

#include "fmtstrformatter.h"
#include "logger.h"
#include "rssfeed.h"
#include "utils.h"


namespace newsboat {

	QueueManager::QueueManager(ConfigContainer* cfg_)
	: cfg(cfg_)
{}

QueueManager::~QueueManager()
{}

void QueueManager::init() {
	plugin_init_t plugin_init;

	std::string plugin_file = cfg->get_configvalue("podcast-plugin") + ".so";
	std::string plugin_path = cfg->get_configvalue("plugin-path");
	std::string plugin = plugin_path + "/" + plugin_file;

	LOG(Level::DEBUG,
	    "QueueManager::init: loading plugin `%s'",
	    plugin);

	_plugin_handle = dlopen(plugin.c_str(), RTLD_LAZY);

	if (_plugin_handle == NULL) {
		std::cout << "Cannot load plugin " << plugin << "\n";
		std::cout << dlerror() << "\n";
		return;
	}

        plugin_init = (plugin_init_t) dlsym(_plugin_handle, "plugin_init");

	_queueManager = plugin_init();

	LOG(Level::DEBUG,
	    "QueueManager::init: loaded plugin `%s'",
	    _queueManager->getName());

	_queueManager->init();
}

void QueueManager::deinit() {
	plugin_deinit_t plugin_deinit;

	plugin_deinit = (plugin_deinit_t) dlsym(_plugin_handle, "plugin_deinit");

	_queueManager->deinit();

	plugin_deinit(_queueManager);
}

/// Adds the podcast URL to Podboat's queue file
EnqueueResult QueueManager::enqueue_url(RssItem& item, RssFeed& feed)
{
	return _queueManager->enqueue_url(item, feed);
}

EnqueueResult QueueManager::autoenqueue(RssFeed& feed)
{
	std::lock_guard<std::mutex> lock(feed.item_mutex);
	for (const auto& item : feed.items()) {
		if (item->enqueued() || item->enclosure_url().empty()) {
			continue;
		}

		const auto enclosure_type = item->enclosure_type();
		const auto enclosure_url = item->enclosure_url();

		if (!enclosure_type.empty() && !utils::is_valid_podcast_type(enclosure_type)) {
			LOG(Level::DEBUG, "QueueManager::autoenqueue: Skipping enclosure with url `%s'"
				" because of invalid podcast type `%s'", enclosure_url, enclosure_type);
			continue;
		}

		LOG(Level::DEBUG,
			"QueueManager::autoenqueue: enclosure_url = `%s' enclosure_type = `%s'",
			enclosure_url,
			enclosure_type);
		if (utils::is_http_url(item->enclosure_url())) {
			LOG(Level::INFO,
				"QueueManager::autoenqueue: enqueuing `%s'",
				item->enclosure_url());
			const auto result = enqueue_url(*item, feed);
			switch (result.status) {
			case EnqueueStatus::QUEUED_SUCCESSFULLY:
			case EnqueueStatus::URL_QUEUED_ALREADY:
				// Not an issue, continue processing rest of items
				break;
			case EnqueueStatus::QUEUE_FILE_OPEN_ERROR:
			case EnqueueStatus::OUTPUT_FILENAME_USED_ALREADY:
				// Let caller of `autoenqueue` handle the issue
				return result;
			}
		}
	}

	return {EnqueueStatus::QUEUED_SUCCESSFULLY, ""};
}

} // namespace newsboat
