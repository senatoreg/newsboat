#include "mpdqueuemanager.h"

#include <fstream>
#include <iostream>
#include <libxml/uri.h>
#include <dlfcn.h>

#include "fmtstrformatter.h"
#include "rssfeed.h"
#include "utils.h"


namespace newsboat {

MPDQueueManager::MPDQueueManager(ConfigContainer* cfg_, std::string queue_file)
	: cfg(cfg_)
	, queue_file(queue_file)
	, _name("mpd")
{}

void MPDQueueManager::deinit()
{
	if (cfg->get_configvalue_as_bool("podcast-to-mpd") && (mpd_handle != NULL)) {
		int ret = dlclose(mpd_handle);
                if (ret > 0) {
			std::cout << "Unloading libmpdclient failed: " << dlerror() << "\n";
                }
	}
}

void MPDQueueManager::init()
{
	std::cout << "Setting up mpd...";

	mpd_handle = dlopen("libmpdclient.so.2", RTLD_LAZY);

	if (mpd_handle == nullptr) {
		std::cout << dlerror() << "\n";
		return;
	}

	mpd_connection_get_error_ = (mpd_connection_get_error_t) dlsym(mpd_handle, "mpd_connection_get_error");
	mpd_search_queue_songs_ = (mpd_search_queue_songs_t) dlsym(mpd_handle, "mpd_search_queue_songs");
	mpd_search_add_uri_constraint_ = (mpd_search_add_uri_constraint_t) dlsym(mpd_handle, "mpd_search_add_uri_constraint");
	mpd_search_commit_ = (mpd_search_commit_t) dlsym(mpd_handle, "mpd_search_commit");
	mpd_recv_song_ = (mpd_recv_song_t) dlsym(mpd_handle, "mpd_recv_song");
	mpd_run_add_ = (mpd_run_add_t) dlsym(mpd_handle, "mpd_run_add");
	mpd_connection_free_ = (mpd_connection_free_t) dlsym(mpd_handle, "mpd_connection_free");
	mpd_connection_new_ = (mpd_connection_new_t) dlsym(mpd_handle, "mpd_connection_new");

	std::cout << "done" << "\n";
	return;
}

EnqueueResult MPDQueueManager::enqueue_url(std::shared_ptr<RssItem> item,
					   std::shared_ptr<RssFeed> feed)
{
	const std::string &url = item->enclosure_url();
	EnqueueResult res;
	std::string mpd_host = cfg->get_configvalue("mpd-host");
	mpd_error err;
	int songs = 0;

	LOG(Level::DEBUG,
	    "MPDQueueManager::enqueue_url: enclosure_url = `%s' enclosure_type = `%s' for `%s'",
	    item->enclosure_url(),
	    item->enclosure_type(),
	    feed->title());
	if (!mpd_host.empty())
		mpd_connection = mpd_connection_new_(mpd_host.c_str(), cfg->get_configvalue_as_int("mpd-port"),
						     cfg->get_configvalue_as_int("mpd-timeout") * 1000);
	else
		mpd_connection = mpd_connection_new_(nullptr, cfg->get_configvalue_as_int("mpd-port"),
						     cfg->get_configvalue_as_int("mpd-timeout") * 1000);

	err = mpd_connection_get_error_(mpd_connection);
	if (err != MPD_ERROR_SUCCESS) {
		std::string err_msg(mpd_connection_get_error_message_(mpd_connection));
		return {EnqueueStatus::QUEUE_FILE_OPEN_ERROR, "Error connecting to MPD (" + err_msg + ")"};
	}

	mpd_search_queue_songs_(mpd_connection, true);
	mpd_search_add_uri_constraint_(mpd_connection,
				       MPD_OPERATOR_DEFAULT,
				       url.c_str());
	mpd_search_commit_(mpd_connection);

	err = mpd_connection_get_error_(mpd_connection);
	if (err != MPD_ERROR_SUCCESS) {
		std::string err_msg(mpd_connection_get_error_message_(mpd_connection));
		mpd_connection_free_(mpd_connection);
		return {EnqueueStatus::QUEUE_FILE_OPEN_ERROR, "MPD search failed (" + err_msg + ")"};
	}
	struct mpd_song* song;
	while ((song = mpd_recv_song_(mpd_connection)) != NULL) {
		songs++;
	}

	if (songs > 0) {
		mpd_connection_free_(mpd_connection);
		return {EnqueueStatus::URL_QUEUED_ALREADY, url};
	}

	mpd_run_add_(mpd_connection, url.c_str());
	err = mpd_connection_get_error_(mpd_connection);
	if (err != MPD_ERROR_SUCCESS) {
		std::string err_msg(mpd_connection_get_error_message_(mpd_connection));
		mpd_connection_free_(mpd_connection);
		return {EnqueueStatus::QUEUE_FILE_OPEN_ERROR, "MPD connection error (" + err_msg + ")"};
	}
	mpd_connection_free_(mpd_connection);
	return {EnqueueStatus::QUEUED_SUCCESSFULLY, ""};
}

} // namespace newsboat

extern "C" newsboat::QueueManager* plugin_init(newsboat::ConfigContainer* cfg, std::string queue_file) {
	return new newsboat::MPDQueueManager(cfg, queue_file);
}

extern "C" void plugin_deinit(newsboat::QueueManager* manager) {
	delete manager;
}
