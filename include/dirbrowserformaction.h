#ifndef NEWSBOAT_DIRBROWSERFORMACTION_H
#define NEWSBOAT_DIRBROWSERFORMACTION_H

#include <sys/stat.h>
#include <grp.h>

#include "configcontainer.h"
#include "file_system.h"
#include "formaction.h"
#include "listwidget.h"
#include "stflrichtext.h"

namespace newsboat {

class DirBrowserFormAction : public FormAction {
public:
	DirBrowserFormAction(View&, std::string formstr, ConfigContainer* cfg);
	~DirBrowserFormAction() override = default;
	void prepare() override;
	void init() override;
	std::vector<KeyMapHintEntry> get_keymap_hint() const override;

	std::string id() const override
	{
		return "dirbrowser";
	}
	std::string title() override;

protected:
	std::string main_widget() const override
	{
		return "files";
	}

private:
	bool process_operation(Operation op,
		const std::vector<std::string>& args,
		BindingType bindingType = BindingType::BindKey) override;
	void update_title(const std::string& working_directory);

	void add_directory(std::vector<file_system::FileSystemEntry>& id_at_position,
		std::string dirname);
	std::vector<file_system::FileSystemEntry> id_at_position;
	std::vector<StflRichText> lines;

	std::string get_formatted_dirname(std::string dirname, mode_t mode);

	LineView file_prompt_line;
	std::string cwd;

	ListWidget files_list;

	View& view;
};

} // namespace newsboat

#endif //NEWSBOAT_DIRBROWSERFORMACTION_H
